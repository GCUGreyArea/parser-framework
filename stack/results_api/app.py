from flask import Flask, jsonify
from pymongo import MongoClient

from stack.common.auth import require_api_key
from stack.common.settings import env, required_env


app = Flask(__name__)

API_KEY = required_env("RESULTS_API_KEY")
MONGO_URI = required_env("RESULTS_MONGO_URI")
ANALYSIS_DB = env("ANALYSIS_DB", "analysis")
PARSED_COLLECTION = env("PARSED_COLLECTION", "parsed_results")
REPORT_COLLECTION = env("REPORT_COLLECTION", "report_results")


def analysis_database():
    client = MongoClient(MONGO_URI)
    return client[ANALYSIS_DB]


@app.get("/api/v1/results/<bundle_id>")
@require_api_key(API_KEY)
def get_parsed_results(bundle_id: str):
    document = analysis_database()[PARSED_COLLECTION].find_one({"_id": bundle_id}, {"_id": 0})
    if document is None:
        return jsonify({"error": "not_found", "bundle_id": bundle_id}), 404
    return jsonify(document)


@app.get("/api/v1/reports/<bundle_id>")
@require_api_key(API_KEY)
def get_report_results(bundle_id: str):
    document = analysis_database()[REPORT_COLLECTION].find_one({"_id": bundle_id}, {"_id": 0})
    if document is None:
        return jsonify({"error": "not_found", "bundle_id": bundle_id}), 404
    return jsonify(document)


@app.get("/healthz")
def healthcheck():
    return jsonify({"status": "ok"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8081)
