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
- `rules/identification`
- `tests`

## Current implementation

The current framework slice is in place:

- regex-based message identification with named capture extraction
- downstream parser rulesets loaded from external YAML
- KV parsing using declared token schemas
- shared-library build output for the framework
- example parser executable that emits JSON in a `regex-parser`-style `parsed` array
- loader-side example validation for identification captures and KV token extraction
- external YAML rules loaded from `rules/`

## Build

```bash
make example_parser
ASAN_OPTIONS=detect_leaks=0 ./build/example_parser rules
```

## Test

```bash
make test
```
