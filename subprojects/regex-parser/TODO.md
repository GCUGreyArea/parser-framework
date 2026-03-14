# TODO

## Now

1. Add aggregated example-validation reporting so a full ruleset load reports all invalid examples instead of stopping at the first failure.
2. Replace remaining shorthand `example:` entries in test fixtures with explicit `examples:` blocks where exact token and property checks matter.
3. Add unit tests for ambiguous cross-rule matches where two different rules can both parse the same message.
4. Add unit tests for dynamic-property `else` branches, including `absent_properties` expectations.
5. Add unit tests for precedence across inherited patterns with asserts, not only synthetic fixtures.

## Next

1. Decide whether `even.name` in the AWS Toy rule is intentional; if it is a typo, rename it to `event.name` and update rules, tests, and documentation.
2. Add a dedicated validation mode to the CLI, for example `parser -r rules --validate`.
3. Preserve YAML source location metadata so rule and example validation errors can identify the file and line number directly.
4. Improve the validation API so callers can consume structured diagnostics instead of only exception strings.
5. Add library-level tests for loading the real `rules/` directory without relying on the current traced ASan runner path.

## Later

1. Improve dynamic-property compile-time type checking so invalid arithmetic and comparison combinations fail before runtime.
2. Expose a higher-level library interface for clients that want to load and validate rulesets without using the CLI entry point.
3. Add makefile dependency tracking so header changes do not require a clean rebuild to avoid stale-object issues.
4. Revisit benchmark coverage and fix the current benchmark gaps.
5. Add containerized build and test support.

## Existing roadmap items still worth revisiting

1. Add normalized token mapping between rule-local values and global system tokens.
2. Add translation and mapping facilities from rule token values to normalized values and system tokens.
3. Review whether conditional logic should evolve beyond the current proof-of-concept implementation strategy.
