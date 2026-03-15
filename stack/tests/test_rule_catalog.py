import os
import tempfile
import unittest
from pathlib import Path

from stack.common.rule_catalog import discover_rule_catalog_documents, sync_rule_catalog


class FakeRuleCatalogCollection:
    def __init__(self, documents=None):
        self.documents = {}
        self.indexes = []
        for document in documents or []:
            self.documents[document["_id"]] = dict(document)

    def create_index(self, spec, **kwargs):
        self.indexes.append((tuple(spec), kwargs))

    def find(self, *_args, **_kwargs):
        return [dict(document) for document in self.documents.values()]

    def replace_one(self, _filter, replacement, upsert=False):
        self.documents[replacement["_id"]] = dict(replacement)

    def update_one(self, selector, update):
        document = self.documents[selector["_id"]]
        for key, value in update.get("$set", {}).items():
            document[key] = value


class RuleCatalogTest(unittest.TestCase):
    def test_discover_rule_catalog_documents_reads_parser_and_report_rules(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            parser_root = root / "rules"
            report_root = root / "report_rules"
            parser_root.mkdir()
            report_root.mkdir()

            (parser_root / "alpha.yaml").write_text("rules:\n  - id: alpha\n", encoding="utf-8")
            nested = parser_root / "nested"
            nested.mkdir()
            (nested / "beta.yml").write_text("rules:\n  - id: beta\n", encoding="utf-8")
            (report_root / "gamma.yaml").write_text("reports:\n  - id: gamma\n", encoding="utf-8")
            (report_root / "ignore.txt").write_text("ignored\n", encoding="utf-8")

            documents = discover_rule_catalog_documents(
                {
                    "parser": str(parser_root),
                    "report": str(report_root),
                }
            )

            self.assertEqual(sorted(documents), ["parser:alpha.yaml", "parser:nested/beta.yml", "report:gamma.yaml"])
            self.assertEqual(documents["parser:nested/beta.yml"]["path"], "nested/beta.yml")
            self.assertEqual(documents["report:gamma.yaml"]["rule_type"], "report")
            self.assertTrue(documents["parser:alpha.yaml"]["content_hash"])

    def test_sync_rule_catalog_tracks_created_updated_refreshed_and_removed_rules(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            parser_root = root / "rules"
            report_root = root / "report_rules"
            parser_root.mkdir()
            report_root.mkdir()

            changed_rule = parser_root / "changed.yaml"
            touched_rule = parser_root / "touched.yaml"
            removed_rule = report_root / "removed.yaml"

            changed_rule.write_text("rules:\n  - id: first\n", encoding="utf-8")
            touched_rule.write_text("rules:\n  - id: touched\n", encoding="utf-8")
            removed_rule.write_text("reports:\n  - id: removed\n", encoding="utf-8")

            collection = FakeRuleCatalogCollection()
            initial = sync_rule_catalog(
                collection,
                {
                    "parser": str(parser_root),
                    "report": str(report_root),
                },
                synced_at="2026-03-15T11:00:00Z",
            )

            self.assertEqual(initial["created"], 3)
            self.assertEqual(initial["removed"], 0)
            self.assertEqual(collection.documents["parser:changed.yaml"]["status"], "active")

            changed_rule.write_text("rules:\n  - id: second\n", encoding="utf-8")
            current_ns = touched_rule.stat().st_mtime_ns
            os.utime(touched_rule, ns=(current_ns + 5_000_000_000, current_ns + 5_000_000_000))
            removed_rule.unlink()

            second = sync_rule_catalog(
                collection,
                {
                    "parser": str(parser_root),
                    "report": str(report_root),
                },
                synced_at="2026-03-15T12:00:00Z",
            )

            self.assertEqual(second["created"], 0)
            self.assertEqual(second["updated"], 1)
            self.assertEqual(second["refreshed"], 1)
            self.assertEqual(second["removed"], 1)
            self.assertEqual(second["unchanged"], 0)

            self.assertIn("second", collection.documents["parser:changed.yaml"]["content"])
            self.assertEqual(collection.documents["parser:touched.yaml"]["updated_at"], "2026-03-15T11:00:00Z")
            self.assertEqual(collection.documents["parser:touched.yaml"]["last_synced_at"], "2026-03-15T12:00:00Z")
            self.assertEqual(collection.documents["report:removed.yaml"]["status"], "removed")
            self.assertEqual(collection.documents["report:removed.yaml"]["removed_at"], "2026-03-15T12:00:00Z")
            self.assertEqual(len(collection.indexes), 4)


if __name__ == "__main__":
    unittest.main()
