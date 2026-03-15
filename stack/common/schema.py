import json
from pathlib import Path
from typing import Any, Dict


def load_bundle_schema(schema_path: str) -> Dict[str, Any]:
    with open(schema_path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]
