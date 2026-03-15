import re
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]


class ComposeAssetsTest(unittest.TestCase):
    def test_mongo_compose_assets_are_present_and_referenced(self):
        compose_text = (REPO_ROOT / "docker-compose.yml").read_text(encoding="utf-8")

        self.assertIn("./docker/mongo/run-mongo.sh:/docker/run-mongo.sh:ro", compose_text)
        self.assertIn("./docker/mongo/mongo-keyfile:/docker/mongo/mongo-keyfile:ro", compose_text)

        run_script = REPO_ROOT / "docker" / "mongo" / "run-mongo.sh"
        key_file = REPO_ROOT / "docker" / "mongo" / "mongo-keyfile"

        self.assertTrue(run_script.exists())
        self.assertTrue(key_file.exists())
        self.assertIn("--keyFile /tmp/mongo-keyfile", run_script.read_text(encoding="utf-8"))
        self.assertRegex(key_file.read_text(encoding="utf-8").strip(), r"^[A-Za-z0-9+/=]+$")

    def test_worker_image_ignores_host_build_artifacts(self):
        dockerignore_text = (REPO_ROOT / ".dockerignore").read_text(encoding="utf-8")
        dockerfile_text = (REPO_ROOT / "stack" / "Dockerfile.worker").read_text(encoding="utf-8")

        self.assertIn("build", dockerignore_text)
        self.assertIn("subprojects/jsmn/build", dockerignore_text)
        self.assertIn("subprojects/regex-parser/lib/build", dockerignore_text)
        self.assertIn("nlohmann-json3-dev", dockerfile_text)
        self.assertIn("libasan8", dockerfile_text)
        self.assertIn("rm -rf build subprojects/jsmn/build subprojects/regex-parser/lib/build", dockerfile_text)
        self.assertIn("COPY --from=build /app/subprojects/jsmn/build/release /app/subprojects/jsmn/build/release", dockerfile_text)
        self.assertIn("COPY --from=build /app/subprojects/regex-parser/lib/build/release /app/subprojects/regex-parser/lib/build/release", dockerfile_text)
        self.assertIn("COPY --from=build /app/fragments /app/fragments", dockerfile_text)


if __name__ == "__main__":
    unittest.main()
