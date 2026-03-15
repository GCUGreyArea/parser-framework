import hashlib
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, Mapping

from stack.common.documents import utc_now_iso
from stack.common.schema import repo_root


YAML_SUFFIXES = {".yaml", ".yml"}


def iso_from_timestamp(timestamp: float) -> str:
    return datetime.fromtimestamp(timestamp, tz=timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def project_relative_path(path: Path) -> str:
    try:
        return path.resolve().relative_to(repo_root().resolve()).as_posix()
    except ValueError:
        return path.resolve().as_posix()


def iter_rule_files(root: Path) -> Iterable[Path]:
    if root.is_file():
        if root.suffix.lower() in YAML_SUFFIXES:
            yield root
        return

    for path in sorted(candidate for candidate in root.rglob("*") if candidate.is_file() and candidate.suffix.lower() in YAML_SUFFIXES):
        yield path


def build_rule_catalog_document(rule_type: str, root: Path, path: Path) -> Dict[str, Any]:
    stat = path.stat()
    content = path.read_text(encoding="utf-8")

    relative_path = path.name if root.is_file() else path.relative_to(root).as_posix()
    content_hash = hashlib.sha256(content.encode("utf-8")).hexdigest()

    return {
        "_id": f"{rule_type}:{relative_path}",
        "rule_type": rule_type,
        "rule_root": project_relative_path(root),
        "path": relative_path,
        "file_name": path.name,
        "status": "active",
        "content": content,
        "content_hash": content_hash,
        "source_modified_at": iso_from_timestamp(stat.st_mtime),
        "source_mtime_ns": stat.st_mtime_ns,
    }


def discover_rule_catalog_documents(rule_roots: Mapping[str, str]) -> Dict[str, Dict[str, Any]]:
    documents: Dict[str, Dict[str, Any]] = {}

    for rule_type, root_value in sorted(rule_roots.items()):
        root = Path(root_value)
        if not root.exists():
            raise RuntimeError(f"Rules path does not exist [{root}]")

        for path in iter_rule_files(root):
            document = build_rule_catalog_document(rule_type, root, path)
            documents[document["_id"]] = document

    return documents


def merge_rule_catalog_document(
    existing: Dict[str, Any],
    discovered: Dict[str, Any],
    synced_at: str,
) -> Dict[str, Any]:
    content_changed = existing.get("content_hash") != discovered["content_hash"]
    source_changed = existing.get("source_mtime_ns") != discovered["source_mtime_ns"]
    reactivated = existing.get("status") != "active"

    if not (content_changed or source_changed or reactivated):
        return {}

    merged = dict(existing)
    merged.update(discovered)
    merged["status"] = "active"
    merged["created_at"] = existing.get("created_at", synced_at)
    merged["updated_at"] = synced_at if content_changed or reactivated else existing.get("updated_at", synced_at)
    merged["last_synced_at"] = synced_at
    merged["removed_at"] = ""
    return merged


def ensure_rule_catalog_indexes(collection: Any) -> None:
    collection.create_index([("rule_type", 1), ("status", 1), ("path", 1)])
    collection.create_index([("content_hash", 1)])


def sync_rule_catalog(collection: Any, rule_roots: Mapping[str, str], synced_at: str = "") -> Dict[str, int]:
    sync_time = synced_at or utc_now_iso()
    ensure_rule_catalog_indexes(collection)

    discovered = discover_rule_catalog_documents(rule_roots)
    existing = {document["_id"]: document for document in collection.find({})}

    summary = {
        "created": 0,
        "updated": 0,
        "refreshed": 0,
        "removed": 0,
        "unchanged": 0,
        "active": len(discovered),
    }

    for document_id, document in discovered.items():
        current = existing.get(document_id)
        if current is None:
            created = dict(document)
            created["created_at"] = sync_time
            created["updated_at"] = sync_time
            created["last_synced_at"] = sync_time
            created["removed_at"] = ""
            collection.replace_one({"_id": document_id}, created, upsert=True)
            summary["created"] += 1
            continue

        merged = merge_rule_catalog_document(current, document, sync_time)
        if not merged:
            summary["unchanged"] += 1
            continue

        collection.replace_one({"_id": document_id}, merged, upsert=True)
        if current.get("content_hash") != document["content_hash"] or current.get("status") != "active":
            summary["updated"] += 1
        else:
            summary["refreshed"] += 1

    for document_id, current in existing.items():
        if document_id in discovered or current.get("status") == "removed":
            continue

        collection.update_one(
            {"_id": document_id},
            {
                "$set": {
                    "status": "removed",
                    "updated_at": sync_time,
                    "last_synced_at": sync_time,
                    "removed_at": sync_time,
                }
            },
        )
        summary["removed"] += 1

    return summary
