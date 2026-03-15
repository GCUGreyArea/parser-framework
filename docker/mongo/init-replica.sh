#!/usr/bin/env bash
set -euo pipefail

ROOT_URI="mongodb://${MONGO_ROOT_USERNAME}:${MONGO_ROOT_PASSWORD}@mongo:27017/admin"

until mongosh --quiet "${ROOT_URI}" --eval 'db.runCommand({ ping: 1 }).ok' >/dev/null 2>&1; do
  sleep 2
done

mongosh --quiet "${ROOT_URI}" <<'EOF'
try {
  rs.status();
} catch (error) {
  rs.initiate({
    _id: "rs0",
    members: [{ _id: 0, host: "mongo:27017" }]
  });
}
EOF

until mongosh --quiet "${ROOT_URI}" --eval 'db.hello().isWritablePrimary' | grep -q 'true'; do
  sleep 2
done

mongosh --quiet "${ROOT_URI}" <<EOF
db = db.getSiblingDB("ingestion");
try {
  db.createUser({
    user: "${INGEST_MONGO_USERNAME}",
    pwd: "${INGEST_MONGO_PASSWORD}",
    roles: [
      { role: "readWrite", db: "ingestion" }
    ]
  });
} catch (error) {
  if (error.codeName !== "DuplicateKey") {
    throw error;
  }
}

db = db.getSiblingDB("analysis");
try {
  db.createUser({
    user: "${RESULTS_MONGO_USERNAME}",
    pwd: "${RESULTS_MONGO_PASSWORD}",
    roles: [
      { role: "read", db: "analysis" }
    ]
  });
} catch (error) {
  if (error.codeName !== "DuplicateKey") {
    throw error;
  }
}

db = db.getSiblingDB("admin");
try {
  db.createUser({
    user: "${PARSER_MONGO_USERNAME}",
    pwd: "${PARSER_MONGO_PASSWORD}",
    roles: [
      { role: "readWrite", db: "ingestion" },
      { role: "readWrite", db: "analysis" }
    ]
  });
} catch (error) {
  if (error.codeName !== "DuplicateKey") {
    throw error;
  }
}

db = db.getSiblingDB("ingestion");
db.createCollection("bundles");
db.bundles.createIndex({ status: 1, received_at: 1 });

db = db.getSiblingDB("analysis");
db.createCollection("parsed_results");
db.createCollection("report_results");
db.createCollection("rule_catalog");
db.parsed_results.createIndex({ bundle_id: 1 }, { unique: true });
db.report_results.createIndex({ bundle_id: 1 }, { unique: true });
db.rule_catalog.createIndex({ rule_type: 1, status: 1, path: 1 });
db.rule_catalog.createIndex({ content_hash: 1 });
EOF
