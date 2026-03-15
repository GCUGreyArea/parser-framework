import json
import socket
import subprocess
import tempfile
import time
from pathlib import Path
from typing import Any, Dict, Optional

from pymongo import MongoClient, ReturnDocument
from pymongo.errors import PyMongoError

from stack.common.documents import split_processing_output, utc_now_iso
from stack.common.settings import env, required_env


MONGO_URI = required_env("PARSER_MONGO_URI")
INGEST_DB = env("INGEST_DB", "ingestion")
INGEST_COLLECTION = env("INGEST_COLLECTION", "bundles")
ANALYSIS_DB = env("ANALYSIS_DB", "analysis")
PARSED_COLLECTION = env("PARSED_COLLECTION", "parsed_results")
REPORT_COLLECTION = env("REPORT_COLLECTION", "report_results")
PARSER_BINARY = env("PARSER_BINARY", "/app/build/release/example_ingestion_bundle")
RULES_PATH = env("RULES_PATH", "/app/rules")
REPORT_RULES_PATH = env("REPORT_RULES_PATH", "/app/report_rules")
WORKER_ID = env("WORKER_ID", socket.gethostname())
CHANGE_STREAM_MAX_AWAIT_MS = int(env("CHANGE_STREAM_MAX_AWAIT_MS", "1000"))
POLL_INTERVAL_SECONDS = float(env("POLL_INTERVAL_SECONDS", "2"))


def mongo_client() -> MongoClient:
    return MongoClient(MONGO_URI)


def ingestion_collection(client: MongoClient):
    return client[INGEST_DB][INGEST_COLLECTION]


def claim_bundle(client: MongoClient, bundle_id: str) -> Optional[Dict[str, Any]]:
    collection = ingestion_collection(client)
    now = utc_now_iso()
    return collection.find_one_and_update(
        {"_id": bundle_id, "status": "pending"},
        {
            "$set": {
                "status": "processing",
                "processor.claimed_by": WORKER_ID,
                "processor.claimed_at": now,
                "processor.error": "",
            },
            "$inc": {"processor.attempts": 1},
        },
        return_document=ReturnDocument.AFTER,
    )


def run_parser(bundle: Dict[str, Any]) -> Dict[str, Any]:
    with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False, encoding="utf-8") as handle:
        json.dump(bundle, handle)
        bundle_path = handle.name

    try:
        command = [
            PARSER_BINARY,
            "--bundle",
            bundle_path,
            "--rules",
            RULES_PATH,
            "--report-rules",
            REPORT_RULES_PATH,
        ]

        completed = subprocess.run(command, check=False, capture_output=True, text=True)
        if completed.returncode != 0:
            raise RuntimeError(completed.stderr.strip() or "parser execution failed")
        return json.loads(completed.stdout)
    finally:
        Path(bundle_path).unlink(missing_ok=True)


def store_results(client: MongoClient, processed: Dict[str, Any]) -> None:
    parsed_document, report_document = split_processing_output(processed)
    analysis_db = client[ANALYSIS_DB]
    analysis_db[PARSED_COLLECTION].replace_one({"_id": parsed_document["_id"]}, parsed_document, upsert=True)
    analysis_db[REPORT_COLLECTION].replace_one({"_id": report_document["_id"]}, report_document, upsert=True)


def mark_completed(client: MongoClient, bundle_id: str) -> None:
    ingestion_collection(client).update_one(
        {"_id": bundle_id},
        {
            "$set": {
                "status": "completed",
                "processor.completed_at": utc_now_iso(),
                "processor.error": "",
            }
        },
    )


def mark_failed(client: MongoClient, bundle_id: str, error: str) -> None:
    ingestion_collection(client).update_one(
        {"_id": bundle_id},
        {
            "$set": {
                "status": "failed",
                "processor.completed_at": utc_now_iso(),
                "processor.error": error,
            }
        },
    )


def process_claimed_document(client: MongoClient, document: Dict[str, Any]) -> None:
    bundle_id = document["bundle_id"]
    try:
        processed = run_parser(document["bundle"])
        store_results(client, processed)
        mark_completed(client, bundle_id)
    except Exception as error:  # pragma: no cover - exercised in container runtime
        mark_failed(client, bundle_id, str(error))


def process_pending_bundle(client: MongoClient, bundle_id: str) -> None:
    claimed = claim_bundle(client, bundle_id)
    if claimed is not None:
        process_claimed_document(client, claimed)


def drain_pending_documents(client: MongoClient) -> None:
    collection = ingestion_collection(client)
    pending = collection.find({"status": "pending"}, {"bundle_id": 1})
    for document in pending:
        process_pending_bundle(client, document["bundle_id"])


def watch_loop() -> None:
    client = mongo_client()
    collection = ingestion_collection(client)

    while True:
        try:
            drain_pending_documents(client)
            with collection.watch(full_document="updateLookup", max_await_time_ms=CHANGE_STREAM_MAX_AWAIT_MS) as stream:
                for change in stream:
                    if change.get("operationType") != "insert":
                        continue
                    full_document = change.get("fullDocument", {})
                    if full_document.get("status") != "pending":
                        continue
                    process_pending_bundle(client, full_document["bundle_id"])
        except PyMongoError:
            time.sleep(POLL_INTERVAL_SECONDS)


if __name__ == "__main__":
    watch_loop()
