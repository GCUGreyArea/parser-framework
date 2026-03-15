# Parser Framework

This repository vendors the parser dependencies as local subprojects so they
can be modified here without affecting the original sibling repositories.

## Layout

- `subprojects/jsmn`
- `subprojects/regex-parser`
- `include/parser_framework`
- `src`
- `examples`
- `messages`
- `messages/cloudflare`
- `report_rules`
- `rules`
- `rules/KV`
- `rules/JSON`
- `rules/identification`
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
- shared-library build output for the framework
- example parser executable that emits JSON in a `regex-parser`-style `parsed` array
- example parser defaults to the external message fixtures under `messages/`
- example breach-report executable that reads parser output and emits a separate `reports` JSON structure
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

Both example binaries also support `-h` / `--help` to print their CLI syntax.

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
