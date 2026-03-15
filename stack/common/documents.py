import copy
from datetime import datetime, timezone
from typing import Any, Dict, Optional, Tuple


def utc_now_iso() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def build_ingestion_document(bundle: Dict[str, Any], received_at: Optional[str] = None) -> Dict[str, Any]:
    bundle_id = bundle["bundle_id"]
    timestamp = received_at or utc_now_iso()
    return {
        "_id": bundle_id,
        "bundle_id": bundle_id,
        "schema_version": bundle.get("schema_version", ""),
        "produced_at": bundle.get("produced_at", ""),
        "received_at": timestamp,
        "status": "pending",
        "bundle": copy.deepcopy(bundle),
        "processor": {
            "attempts": 0,
            "claimed_by": "",
            "claimed_at": "",
            "completed_at": "",
            "error": "",
        },
    }


def build_status_projection(document: Dict[str, Any]) -> Dict[str, Any]:
    processor = document.get("processor", {})
    return {
        "bundle_id": document.get("bundle_id", ""),
        "status": document.get("status", "unknown"),
        "schema_version": document.get("schema_version", ""),
        "produced_at": document.get("produced_at", ""),
        "received_at": document.get("received_at", ""),
        "processor": {
            "attempts": processor.get("attempts", 0),
            "claimed_by": processor.get("claimed_by", ""),
            "claimed_at": processor.get("claimed_at", ""),
            "completed_at": processor.get("completed_at", ""),
            "error": processor.get("error", ""),
        },
    }


def split_processing_output(processed: Dict[str, Any], processed_at: Optional[str] = None) -> Tuple[Dict[str, Any], Dict[str, Any]]:
    timestamp = processed_at or utc_now_iso()
    bundle = copy.deepcopy(processed.get("bundle", {}))
    bundle_id = bundle.get("bundle_id", "")

    parsed_document = {
        "_id": bundle_id,
        "bundle_id": bundle_id,
        "processed_at": timestamp,
        "bundle": bundle,
        "collections": copy.deepcopy(processed.get("collections", [])),
    }

    report_document = {
        "_id": bundle_id,
        "bundle_id": bundle_id,
        "processed_at": timestamp,
        "bundle": {
            "bundle_id": bundle_id,
            "schema_version": bundle.get("schema_version", ""),
            "produced_at": bundle.get("produced_at", ""),
        },
        "report_output": copy.deepcopy(processed.get("report_output", {})),
    }

    return parsed_document, report_document
