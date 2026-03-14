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

JSMN_LIBRARY: subprojects/jsmn
REGEX_PARSER: subprojects/regex-parser
GIT: https://github.com/GCUGreyArea/parser-framework.git
