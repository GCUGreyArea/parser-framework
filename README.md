# Parser Framework

This repository vendors the parser dependencies as local subprojects so they
can be modified here without affecting the original sibling repositories.

## Layout

- `subprojects/jsmn`
- `subprojects/regex-parser`
- `include/parser_framework`
- `src`
- `examples`
- `tests`

## Current implementation

The first framework slice is in place:

- regex anchor-based message identification
- recursive section dispatch
- regex parsing for free-form text
- generic KV parsing
- JSON section parsing through `jsmn` with JSONPath rules

## Build

```bash
make demo
./build/demo
```

## Test

```bash
make test
```
