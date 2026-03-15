import unittest

from stack.common.documents import build_ingestion_document, build_status_projection, split_processing_output


class DocumentsTest(unittest.TestCase):
    def test_build_ingestion_document_sets_pending_status(self):
        bundle = {
            "schema_version": "1.0.0",
            "bundle_id": "bundle-123",
            "produced_at": "2026-03-15T10:00:00Z",
        }
        document = build_ingestion_document(bundle, received_at="2026-03-15T10:01:00Z")

        self.assertEqual(document["_id"], "bundle-123")
        self.assertEqual(document["status"], "pending")
        self.assertEqual(document["received_at"], "2026-03-15T10:01:00Z")
        self.assertEqual(document["processor"]["attempts"], 0)

    def test_build_status_projection_filters_bundle_payload(self):
        document = build_ingestion_document(
            {
                "schema_version": "1.0.0",
                "bundle_id": "bundle-456",
                "produced_at": "2026-03-15T10:00:00Z",
            },
            received_at="2026-03-15T10:02:00Z",
        )
        projection = build_status_projection(document)

        self.assertEqual(projection["bundle_id"], "bundle-456")
        self.assertNotIn("bundle", projection)
        self.assertEqual(projection["processor"]["attempts"], 0)

    def test_split_processing_output_separates_parsed_and_report_documents(self):
        processed = {
            "bundle": {"bundle_id": "bundle-789", "schema_version": "1.0.0", "produced_at": "2026-03-15T10:00:00Z"},
            "collections": [{"collection": {"id": "col-1"}}],
            "report_output": {"reports": [{"id": "rep-1"}]},
        }

        parsed_document, report_document = split_processing_output(processed, processed_at="2026-03-15T10:03:00Z")

        self.assertEqual(parsed_document["_id"], "bundle-789")
        self.assertEqual(parsed_document["processed_at"], "2026-03-15T10:03:00Z")
        self.assertEqual(parsed_document["collections"][0]["collection"]["id"], "col-1")

        self.assertEqual(report_document["_id"], "bundle-789")
        self.assertEqual(report_document["report_output"]["reports"][0]["id"], "rep-1")
        self.assertEqual(report_document["bundle"]["schema_version"], "1.0.0")


if __name__ == "__main__":
    unittest.main()
