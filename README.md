# Parser Framework

This repository vendors the parser dependencies as local subprojects so they
can be modified here without affecting the original sibling repositories.

## Layout

- `subprojects/jsmn`
- `subprojects/regex-parser`
- `include/parser_framework`
- `src`
- `examples`
- `rules`
- `rules/KV`
- `rules/JSON`
- `rules/identification`
- `tests`

## Current implementation

The current framework slice is in place:

- regex-based message identification with named capture extraction
- downstream parser rulesets loaded from external YAML
- KV and JSON parsing using declared token schemas and `jsmn` JSONPath extraction
- shared-library build output for the framework
- example parser executable that emits JSON in a `regex-parser`-style `parsed` array
- example parser defaults to the identification examples declared in the rules library
- loader-side example validation for identification captures plus KV/JSON token extraction
- external YAML rules loaded from `rules/`

## Build

```bash
make example_parser
ASAN_OPTIONS=detect_leaks=0 ./build/example_parser rules
```

With no message-file arguments, the example executable parses the example
messages declared under `rules/identification`.

## Test

```bash
make test
```
