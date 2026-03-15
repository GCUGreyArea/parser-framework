# AGENTS

In this project you will be creating a parsing framework that will address
complex logfiles. We will use two libraries to achieve this.

1. The `jsmn` library
2. The regex parser library

These libraries are now vendored into this repository as local subprojects so
they can be modified here without impacting the original codebases.

I want you to design a framework that begins with a set of identification rules
in regex. These will express the types of pattern that will be parsed.

The first level of rule will use only regex. This will identify:

1. The type of message by some unique anchor.
2. The construction of the message and how it can be broken down (`KV`,
   `JSON`, `free form text`, `mixed`, etc).
3. How the framework should split the log message text and send each section to
   the correct engine for parsing. Subsections of a section may also need to be
   parsed by different engines, and this recursion should be handled by the
   framework.
4. At the end of parsing we should have all tokens for each message.

Additional rules for this project:

1. `free form text` should be addressed using regex rules.
2. JSON sections and JSON rules should use the JSONPath API from `jsmn` rather
   than regex over raw JSON text.
3. The framework should be built against the local vendored copies of the
   libraries in this repository, not the original sibling repositories.
4. All new instructions given for this project should be added to this
   `AGENTS.md` file.
5. The initial work should be committed before further feature work begins.
6. After the initial commit, each new change should be done on its own branch.
7. For each branch of new work, a pull request should be created for review.
8. Merges should only be performed after explicit review approval and a direct
   request to merge.
9. Commit messages should describe the actual work performed.
10. The initial commit message should state that it is the initial commit for
    the log file parser infrastructure.
11. The standard git bootstrap/push template for this repository is:
    `git commit -m "first commit"`
    `git branch -M main`
    `git remote add origin https://github.com/GCUGreyArea/parser-framework.git`
    `git push -u origin main`
12. Please use Google tast for unit testing
13. This project should create a library that builds as a dynamically loadable
    object.
14. The project should also provide an example parser executable that produces
    JSON output similar to the structure used by `regex-parser`.
15. The project should provide an external YAML-based rules library that can be
    read in and executed against messages.
16. Functionality should be extracted and adapted from `regex-parser` where it
    makes sense, including YAML rule-loading behavior.
17. The example program should read rules from the project `rules` directory.
18. The `rules` directory structure should resemble `regex-parser`, including a
    dedicated section for identification rules.
19. Token-extraction rules can contain regex-based and JSON-based sections.
20. Regex-based extraction rules should keep the inheritability model used by
    `regex-parser`.
21. The visible console dialogue for the session should be written to
    `CODEX-CONSOLE.md`.
22. `CODEX-CONSOLE.md` should be formatted in Markdown to resemble the Codex
    console when viewed on GitHub.
23. `CODEX-CONSOLE.md` should contain a complete record of the user/assistant
    interaction for the session.
24. `CODEX-CONSOLE.md` should include visible diff text from the console where
    possible, using Markdown diff formatting for GitHub rendering.
25. Design and code are the priority. `CODEX-CONSOLE.md` must still be updated,
    but it can be refreshed when there is bandwidth to do so.
26. Rule token schemas should be declared once in the rule definition, and
    examples should validate only the relevant extracted token subset plus any
    expected absences.
27. The agent shell environment may be out of sync with the user’s interactive
    shell and system state. When command results differ unexpectedly, account
    for possible environment/auth mismatches.
28. After a pull request is merged, switch the local repository back to
    `main` before starting the next phase of work.
29. Nested message fragments that need additional tokenization should be
    handled with sub-rules, using regex where appropriate.
30. Tokens emitted by nested sub-rules should use dotted names rooted at the
    parent field, for example `__policy_id_tag.db_tag`.
31. Rendered JSON output from the example/parser output helpers should be
    pretty-printed rather than minified.
32. When rendered to JSON, dotted token names should be collapsed into nested
    objects rooted at the parent token, with the parent token's original value
    exposed as `raw` when both raw and child values exist.
33. The rendered `rule` object should include a list of matched rule IDs in
    encounter order, containing the top-level message rule ID plus any
    sub-rule/ruleset IDs that emitted tokens.
34. The framework should only orchestrate parsing: identify message structure,
    dispatch fragments to parser engines, recurse through subsections, and
    reassemble output tokens.
35. Regex-based identification, location, and token extraction should execute
    through the vendored `regex-parser` components rather than bespoke
    `std::regex` parsing in the framework.
36. Key-value parsing should live in a dedicated parser component separate from
    the framework dispatcher.
37. ASsume you have permission to push to git and create PRs.
38. Example and fixture messages should live in external files that are read by
    the system for processing rather than being embedded inline in example
    programs.
39. The example parser executable should parse external message files and
    directories.
40. Use the dynamic properties parser where it is useful to derive additional
    information from parsed tokens and properties.
41. Systems that analyze parsed output and generate higher-level reports should
    be implemented as separate subcomponents rather than folded into the rules
    engine or parsing framework dispatcher.
42. Provide a `make release` build mode that uses optimization and does not
    compile with debug flags or address sanitization.
43. Debug and release builds should use separate build directories so the two
    modes do not reuse stale artifacts.

- JSMN_LIBRARY: subprojects/jsmn
- REGEX_PARSER: subprojects/regex-parser
- GIT: https://github.com/GCUGreyArea/parser-framework.git
