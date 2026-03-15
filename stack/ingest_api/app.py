import uuid

from flask import Flask, jsonify, request
from jsonschema import ValidationError, validate
from pymongo import MongoClient

from stack.common.auth import require_api_key
from stack.common.documents import build_ingestion_document, build_status_projection
from stack.common.schema import load_bundle_schema, repo_root
from stack.common.settings import env, required_env


app = Flask(__name__)

SCHEMA = load_bundle_schema(str(repo_root() / "schemas" / "log-ingestion-bundle.schema.json"))
API_KEY = required_env("INGEST_API_KEY")
MONGO_URI = required_env("INGEST_MONGO_URI")
INGEST_DB = env("INGEST_DB", "ingestion")
INGEST_COLLECTION = env("INGEST_COLLECTION", "bundles")


def bundles_collection():
    client = MongoClient(MONGO_URI)
    return client[INGEST_DB][INGEST_COLLECTION]


@app.post("/api/v1/bundles")
@require_api_key(API_KEY)
def create_bundle():
    payload = request.get_json(silent=True)
    if payload is None:
        return jsonify({"error": "invalid_json"}), 400

    if not payload.get("bundle_id"):
        payload["bundle_id"] = f"bundle-{uuid.uuid4()}"

    try:
        validate(instance=payload, schema=SCHEMA)
    except ValidationError as error:
        return jsonify({"error": "schema_validation_failed", "detail": error.message}), 400

    collection = bundles_collection()
    if collection.find_one({"_id": payload["bundle_id"]}) is not None:
        return jsonify({"error": "bundle_exists", "bundle_id": payload["bundle_id"]}), 409

    document = build_ingestion_document(payload)
    collection.insert_one(document)
    return jsonify(build_status_projection(document)), 202


@app.get("/api/v1/bundles/<bundle_id>/status")
@require_api_key(API_KEY)
def get_bundle_status(bundle_id: str):
    collection = bundles_collection()
    document = collection.find_one({"_id": bundle_id})
    if document is None:
        return jsonify({"error": "not_found", "bundle_id": bundle_id}), 404
    return jsonify(build_status_projection(document))


@app.get("/healthz")
def healthcheck():
    return jsonify({"status": "ok"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8080)
