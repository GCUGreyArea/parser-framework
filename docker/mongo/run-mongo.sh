#!/usr/bin/env bash
set -euo pipefail

install -o 999 -g 999 -m 400 /docker/mongo/mongo-keyfile /tmp/mongo-keyfile

exec /usr/local/bin/docker-entrypoint.sh mongod \
  --bind_ip_all \
  --replSet rs0 \
  --auth \
  --keyFile /tmp/mongo-keyfile
