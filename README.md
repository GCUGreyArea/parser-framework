# Parser Framework

This repository vendors the parser dependencies as local subprojects so they
can be modified here without affecting the original sibling repositories.

## Layout

- `subprojects/jsmn`
- `subprojects/regex-parser`
- `docker`
- `stack`
- `include/parser_framework`
- `src`
- `examples`
- `bundles`
- `messages`
- `messages/cloudflare`
- `report_rules`
- `rules`
- `rules/KV`
- `rules/JSON`
- `rules/identification`
- `schemas`
- `tests`

## Current implementation

The current framework slice is in place:

- regex-based message identification with named capture extraction
- downstream parser rulesets loaded from external YAML
- external message fixtures loaded from files and directories
- the framework acting as a dispatcher over separate regex, KV, and JSON engines
- KV and JSON parsing using dedicated parser components and `jsmn` JSONPath extraction
- dynamic-property evaluation to derive additional parser-side metadata
- a separate report-analysis subcomponent that consumes parsed output and emits breach-oriented JSON reports
- ATT&CK-style enrichment for threat family, tactic, and technique identifiers
- multi-system correlation for repeated attack families seen across distinct systems
- ingestion-envelope schema support for attributed multi-network log bundles
- shared-library build output for the framework
- example parser executable that emits JSON in a `regex-parser`-style `parsed` array
- example parser defaults to the external message fixtures under `messages/`
- example breach-report executable that reads parser output and emits a separate `reports` JSON structure
- example ingestion-bundle executable that reads tagged JSON bundles and runs parsing plus reporting over attributed collections
- loader-side example validation for identification captures plus KV/JSON token extraction
- external YAML rules loaded from `rules/`

## What the system does

This project parses heterogeneous security log messages into structured JSON and
then optionally derives higher-level breach-oriented reports from the parsed
results.

At a high level it does four things:

1. Identify the message type with regex-based identification rules.
2. Dispatch the relevant message fragments to dedicated parser engines for KV,
   JSON, or regex parsing.
3. Reassemble the extracted tokens, including nested token structures produced
   by sub-rules.
4. Evaluate dynamic-property expressions to derive additional information such
   as threat metadata and report fields.

The framework itself is only responsible for orchestration. Regex matching runs
through the vendored `regex-parser` components, KV parsing runs through a
separate KV parser component, JSON extraction uses `jsmn` JSONPath, and report
generation is handled by a separate analysis subcomponent.

## Ingestion Envelope

The repository now includes a literal JSON ingestion schema at
`schemas/log-ingestion-bundle.schema.json`. It is intended for remote log
forwarders or collection services that need to send attributed log bundles into
the parser locally.

Each bundle carries:

- producer metadata so remote components can be identified when they submit a
  bundle
- storage hints so the same document shape can be stored locally in a database
  such as MongoDB without reshaping it
- organization, site, network, and system inventories with stable IDs
- explicit attribution fields for owner, operator, tenant, and provider
  relationships
- log collections that reference those inventories and contain `records` as a
  JSON array of raw log strings

This separation matters for multi-tenant or managed environments. A system can
therefore be modeled as customer-owned, provider-hosted, and third-party
operated at the same time. The worked example bundle in
`bundles/multi-tenant-ingestion.json` shows:

- Cloudflare edge telemetry for a tenant system operated by BAE for SIS
- AWS-hosted WAF infrastructure leased to and operated for SIS
- a customer-owned secure network operated by BAE on behalf of SIS

The ingestion pipeline resolves these references, parses each collection with
the normal parser rules, and then runs the report analyzer across the bundle so
correlations can span multiple systems and networks.

### Bundle Shape

At a minimum, an ingestion bundle looks like this:

```json
{
  "schema_version": "1.0.0",
  "bundle_id": "bundle-2026-03-15-demo-001",
  "produced_at": "2026-03-15T09:30:00Z",
  "producer": {
    "component_id": "forwarder-eu-01",
    "component_type": "remote-log-forwarder",
    "organisation_id": "org-bae",
    "region": "eu-west"
  },
  "storage": {
    "backend": "mongodb",
    "database": "parser_framework",
    "bundle_collection": "ingestion_bundles",
    "result_collection": "ingestion_results"
  },
  "organizations": [],
  "sites": [],
  "networks": [],
  "systems": [],
  "collections": [
    {
      "id": "col-cloudflare-log4shell",
      "system_id": "sys-cloudflare-waf",
      "network_id": "net-public-edge",
      "site_id": "site-global-edge",
      "log_type": "waf",
      "format": "json",
      "classification": "official-sensitive",
      "observed_start": "2021-12-13T11:59:40Z",
      "observed_end": "2021-12-13T11:59:40Z",
      "attribution": {
        "owner_org_ids": [],
        "operator_org_ids": ["org-bae"],
        "tenant_org_ids": ["org-sis"],
        "provider_org_ids": ["org-cloudflare"]
      },
      "records": [
        "{\"Action\":\"block\",...}"
      ]
    }
  ]
}
```

The important design choice is that `collections.records` contains raw log
strings, while the surrounding document contains the metadata needed to
attribute, store, and query those logs later.

### Processing Flow

The ingestion path is now:

1. A remote or local component emits a bundle that conforms to
   `schemas/log-ingestion-bundle.schema.json`.
2. `IngestionLoader` validates and resolves the bundle into typed inventories
   and collections.
3. `IngestionPipeline` feeds each collection's `records` into the existing
   parser framework.
4. The optional report analyzer runs across all parsed collection output so
   correlations can cross system or network boundaries.
5. The resulting document can be stored locally with both raw attribution data
   and derived parser/report output preserved.

This model is aimed at eventual local persistence in a database such as
MongoDB, where the bundle itself and the derived analysis result can be stored
as related documents without losing the organisational context of the source
logs.

## Container Stack

The repository now includes a Docker Compose deployment that separates
ingestion, parsing, and results retrieval into distinct services:

- `mongo`: single MongoDB instance running as a single-node replica set so
  change streams are available
- `mongo-init`: one-shot bootstrap container that initializes the replica set,
  creates scoped users, and creates ingestion and analysis collections
- `ingest-api`: external REST API for writing ingestion bundles
- `parser-worker`: change-stream worker that claims pending bundles, runs the
  parser, and writes derived results
- `results-api`: external REST API for reading parsed output and reports

This keeps the write path and read path separated at both the API and MongoDB
access-control levels.

### Mongo Separation

The stack uses one MongoDB instance but separates the data logically:

- `ingestion.bundles`: raw submitted bundles plus processing status
- `analysis.parsed_results`: parser output grouped by bundle
- `analysis.report_results`: derived report output grouped by bundle

The default Compose bootstrap creates three users:

- `ingest_user`: read/write access to `ingestion`
- `parser_user`: read/write access to both `ingestion` and `analysis`
- `results_user`: read-only access to `analysis`

That means new data submission and results retrieval are intentionally handled
through different Mongo credentials and different REST APIs even when the same
MongoDB server is used underneath.

### Event Flow

The containerized runtime flow is:

1. A client submits a bundle to `ingest-api`.
2. `ingest-api` validates it against
   `schemas/log-ingestion-bundle.schema.json` and inserts it into
   `ingestion.bundles` with status `pending`.
3. MongoDB emits a change-stream event for the insert.
4. `parser-worker` claims that bundle, runs
   `example_ingestion_bundle`, and captures the combined parser/report output.
5. The worker writes parsed output into `analysis.parsed_results` and report
   output into `analysis.report_results`.
6. The worker marks the original bundle `completed` or `failed`.
7. Clients query `results-api` for parsed results or reports.

Because work claiming is status-driven, the worker model can be expanded later
to multiple replicas without rewriting the document layout.

## Threat Analysis Techniques

The demo rules now use a few practical threat-analysis techniques that are
useful when moving from raw parser output toward incident-oriented reporting:

- ATT&CK-style enrichment: parser rules derive `threat.family`,
  `threat.technique_id`, and `threat.tactic_id` so downstream reporting can
  normalize exploit and post-compromise activity across products.
- Multi-signal correlation: the report analyzer correlates events from distinct
  systems when they share a source IP and attack family, producing a
  higher-level multi-system breach-attempt report.
- Defense-in-depth evidence: the demo includes edge, network, and IDS-style
  telemetry so the same activity can be observed at different points in the
  attack path.
- Stage-aware reporting: event rules still emit per-system reports such as
  `initial_access`, `exploitation`, and `command_and_control`, while correlated
  reports summarize repeated hostile activity across those stages.

The current fixtures include a Cloudflare firewall event modeled on the
official firewall-events dataset and Cloudflare's published Log4Shell managed
rule IDs, plus FortiGate IPS and Suricata examples so the report layer can show
both single-system detections and correlated multi-system activity.

## How the rules engine works

Rules are split into two libraries:

- `rules/` contains parser rules.
- `report_rules/` contains post-parse report-generation rules.

Parser execution starts with identification rules under
`rules/identification/`. An identification rule decides whether a message is of
the expected type and defines which parser sections should run next.

For example, the Check Point identification rule:

```yaml
messages:
  - id: 46432110-a2a1-4bc9-8e78-fe30591375e1
    name: CheckPoint
    message_filter:
      - pattern: '^CheckPoint [0-9]+ - \[(?<fields>.*)\]$'
    sections:
      - id: checkpoint_fields
        extract: fields
        parser: mixed
        sections:
          - id: checkpoint_fields_kv
            parser: kv
            ruleset: CheckPointKV
          - id: checkpoint_policy_id_tag
            parser: regex
            ruleset: CheckPointPolicyTagRegex
            locator:
              pattern: '__policy_id_tag:"(product=[^"]+)"'
              group: 1
```

This does three things:

- identifies a Check Point log line
- captures the main field block as `fields`
- dispatches that captured fragment to a mixed section that runs multiple
  parser engines over different parts of the same message

The dispatcher then resolves the referenced rulesets. A ruleset declares the
token schema once, the parser type, optional dynamic properties, and examples.

Example KV ruleset:

```yaml
rules:
  - id: 9b1c8e5c-7a0b-4f1e-9c3a-2f8e5d6c7b8a
    name: CheckPointKV
    parser: kv
    bindings:
      - 46432110-a2a1-4bc9-8e78-fe30591375e1
    tokens:
      action: string
      contextnum: int
      __policy_id_tag: string
      rule_uid: uuid
      service_id: string
```

Example JSON ruleset with dynamic properties:

```yaml
rules:
  - id: 4f8db6bf-6934-4d04-85bf-d4c431ebc0f6
    name: AWSWAFJson
    parser: json
    bindings:
      - 1c507cb9-91d4-4024-871d-49f38ee08f36
    tokens:
      terminating_rule_id: string
      action: string
      client_ip: ip
      uri: string
    dynamic_properties:
      - 'if action == "BLOCK" and terminating_rule_id ?= "SQLi" then threat.event = "web_attack" and threat.source_ip = client_ip and threat.rule_name = terminating_rule_id and threat.severity = "high" and threat.outcome = action'
    json_matches:
      - path: "$.formatVersion"
        equals: "1"
    json_tokens:
      terminating_rule_id: "$.terminatingRuleId"
      action: "$.action"
      client_ip: "$.httpRequest.clientIp"
      uri: "$.httpRequest.uri"
```

## Sub-rules and nested parsing

Sub-rules are used when one extracted field contains its own internal
structure. The Check Point `__policy_id_tag` field is a good example: the KV
parser extracts it as a raw string, then a regex sub-rule tokenizes the inner
content.

Example sub-rule:

```yaml
rules:
  - id: 7c55684a-8746-43d8-a34a-130ebd3ec0d4
    name: CheckPointPolicyTagRegex
    parser: regex
    bindings:
      - 46432110-a2a1-4bc9-8e78-fe30591375e1
    tokens:
      __policy_id_tag.product: string
      __policy_id_tag.db_tag: string
      __policy_id_tag.mgmt: string
      __policy_id_tag.date: int
      __policy_id_tag.policy_name: string
    regex_tokens:
      - name: __policy_id_tag.product
        pattern: '^product=([^\[]+)'
      - name: __policy_id_tag.db_tag
        pattern: 'db_tag=\{([^}]+)\}'
      - name: __policy_id_tag.mgmt
        pattern: ';mgmt=([^;]+)'
```

The parent identification rule uses a locator to isolate only the
`__policy_id_tag` fragment before handing it to this sub-rule. The emitted token
names are dotted so the renderer can collapse them into a nested object rooted
at `__policy_id_tag`.

That means the parser can turn:

```text
__policy_id_tag:"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\]"
```

into:

```json
"__policy_id_tag": {
  "raw": "product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]",
  "product": "VPN-1 & FireWall-1",
  "db_tag": "599255D7-8B94-704F-9D9A-CFA3719EA5CE",
  "mgmt": "bos1cpmgmt01",
  "date": "1630333752",
  "policy_name": "<policy>"
}
```

This same model can be used for any nested fragment: identify the outer
message, capture or locate the relevant substring, and then run a dedicated
ruleset over that inner fragment.

## Dynamic properties

Dynamic properties are derived fields produced after token extraction. They are
useful for expressing semantic meaning that is not directly present as a single
field in the raw message.

Examples:

- mark an AWS WAF SQL injection block as `threat.event = "web_attack"`
- derive `threat.source_ip` from `client_ip`
- mark a FortiGate IPS block as `threat.event = "ips_detection"`
- carry attack names, severity, outcome, and target information into a common
  threat namespace

These properties are rendered separately from raw tokens under the `property`
object in parser output.

## Build

```bash
make release
./build/release/example_parser --rules rules --messages messages
./build/release/example_breach_report --rules rules --report-rules report_rules --messages messages
./build/release/example_ingestion_bundle --bundle bundles/multi-tenant-ingestion.json --rules rules --report-rules report_rules
make test_stack
```

With no message-path arguments, the example parser defaults to the external
fixtures under `messages/`. You can also point it at one file or a directory
tree:

```bash
./build/release/example_parser --messages messages/aws-waf/sqli-block.json
./build/release/example_parser --messages messages
./build/release/example_parser -m messages/aws-waf/sqli-block.json messages/cloudflare/log4shell-header-block.json
```

The breach-report example reads the same external message files, parses them
through the framework, then runs the separate report-analysis component over the
results:

```bash
./build/release/example_breach_report --rules rules --report-rules report_rules --messages messages
```

The ingestion-bundle example reads a JSON envelope that already contains
attributed log collections and raw records:

```bash
./build/release/example_ingestion_bundle --bundle bundles/multi-tenant-ingestion.json
./build/release/example_ingestion_bundle -b bundles/multi-tenant-ingestion.json -r rules -p report_rules
```

Both example binaries also support `-h` / `--help` to print their CLI syntax.
The ingestion example supports `-h` / `--help` as well.

## Docker Compose

The deployment files are:

- `docker-compose.yml`
- `docker/mongo/init-replica.sh`
- `stack/Dockerfile.api`
- `stack/Dockerfile.worker`

To start the stack:

```bash
docker compose up --build
```

That exposes:

- ingestion API on `http://localhost:8080`
- results API on `http://localhost:8081`
- MongoDB on `mongodb://localhost:27017`

### Ingestion API

Default route set:

- `POST /api/v1/bundles`
- `GET /api/v1/bundles/<bundle_id>/status`

Example:

```bash
curl -X POST http://localhost:8080/api/v1/bundles \
  -H 'Content-Type: application/json' \
  -H 'X-API-Key: ingest-api-key' \
  --data @bundles/multi-tenant-ingestion.json
```

### Results API

Default route set:

- `GET /api/v1/results/<bundle_id>`
- `GET /api/v1/reports/<bundle_id>`

Examples:

```bash
curl -H 'X-API-Key: results-api-key' \
  http://localhost:8081/api/v1/results/bundle-2026-03-15-demo-001

curl -H 'X-API-Key: results-api-key' \
  http://localhost:8081/api/v1/reports/bundle-2026-03-15-demo-001
```

These APIs intentionally do not share credentials or route namespaces.

`make release` performs a clean optimized build without debug flags or address
sanitization so the example binaries can be run directly from `build/release/`.
Sanitizer-enabled development builds live separately under `build/debug/`, so
switching modes no longer reuses stale artifacts. For development builds, use
the default targets such as `make example_parser` or `make test`; the `test`
target applies the necessary `ASAN_OPTIONS` internally.

Example output:

```json
{
  "reports": [
    {
      "rule": {
        "id": "02bb72ce-70e6-49a0-8d7f-b88de9c542ff",
        "name": "AWSWAFBreachAttemptReport"
      },
      "source": {
        "message_rule_id": "1c507cb9-91d4-4024-871d-49f38ee08f36",
        "message_rule_name": "AWS WAF",
        "event_name": "AWS WAF"
      },
      "report": {
        "attack": {
          "name": "STMTest_SQLi_XSS"
        },
        "event": "breach_attempt",
        "outcome": "BLOCK",
        "phase": "initial_access",
        "source": {
          "ip": "1.1.1.1"
        },
        "system": "aws_waf",
        "target": {
          "uri": "/"
        }
      }
    },
    {
      "rule": {
        "id": "147f53e2-4fd8-4e12-b3b8-8d1cb2b51470",
        "name": "FortiGateIPSBreachAttemptReport"
      },
      "source": {
        "message_rule_id": "5d6ce0b2-55fd-4bb8-b376-786b63d79795",
        "message_rule_name": "FortiGate IPS",
        "event_name": "FortiGate IPS"
      },
      "report": {
        "attack": {
          "name": "Apache.Log4j.Error.Log.Remote.Code.Execution"
        },
        "event": "breach_attempt",
        "outcome": "blocked",
        "phase": "exploitation",
        "severity": "critical",
        "source": {
          "ip": "198.51.100.25"
        },
        "system": "fortigate_ips",
        "target": {
          "ip": "10.0.0.25"
        }
      }
    },
    {
      "rule": {
        "id": "e64ced89-4c39-4543-9e6c-f602e0f73d21",
        "name": "SuricataPostCompromiseReport"
      },
      "source": {
        "message_rule_id": "d6874d8d-1c8a-4ff8-a686-7608e35f3371",
        "message_rule_name": "Suricata EVE Alert",
        "event_name": "Suricata EVE Alert"
      },
      "report": {
        "attack": {
          "name": "ET MALWARE Win32/CryptFile2 / Revenge Ransomware Checkin M3"
        },
        "event": "post_compromise_activity",
        "phase": "command_and_control",
        "severity": "1",
        "source": {
          "ip": "142.11.240.191"
        },
        "system": "suricata",
        "target": {
          "ip": "192.0.2.55"
        }
      }
    }
  ]
}
```

## How reports are generated

Reports are generated by a separate analysis subcomponent after parsing
completes. The report analyzer consumes `ParseResult` objects, including tokens,
message metadata, and derived properties, and then evaluates report rules from
`report_rules/`.

A report rule binds to one or more parser message rule IDs and uses dynamic
property expressions to produce normalized report fields.

Example report rule:

```yaml
reports:
  - id: 02bb72ce-70e6-49a0-8d7f-b88de9c542ff
    name: AWSWAFBreachAttemptReport
    bindings:
      - 1c507cb9-91d4-4024-871d-49f38ee08f36
    dynamic_properties:
      - 'if threat.event == "web_attack" then report.event = "breach_attempt" and report.phase = "initial_access" and report.system = "aws_waf" and report.source.ip = threat.source_ip and report.attack.name = threat.rule_name and report.outcome = threat.outcome and report.target.uri = uri'
```

Each generated report contains:

- the report rule metadata under `rule`
- the parser rule metadata that produced the source event under `source`
- the derived report content under `report`

In practice the report content is intended to capture normalized breach
analysis, such as:

- event type, for example `breach_attempt` or `post_compromise_activity`
- attack phase, for example `initial_access`, `exploitation`, or
  `command_and_control`
- source IP and target IP or URI
- system type, such as `aws_waf`, `fortigate_ips`, or `suricata`
- attack or signature name
- severity and outcome when available

This separation keeps the parser engine focused on token extraction and keeps
security analysis logic in a distinct post-processing component.

## Test

```bash
make test
```
