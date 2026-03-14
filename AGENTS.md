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

JSMN_LIBRARY: subprojects/jsmn
REGEX_PARSER: subprojects/regex-parser
GIT: https://github.com/GCUGreyArea/parser-framework.git
