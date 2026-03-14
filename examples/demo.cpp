#include <iostream>
#include <string>

#include "parser_framework/ParserFramework.hpp"

using namespace parser_framework;

static ParserFramework build_demo_framework() {
    MessageRule rule;
    rule.id = "app_event";
    rule.name = "Application Event";
    rule.anchor_patterns = {"^APP\\b"};

    SectionRule prefix;
    prefix.id = "prefix";
    prefix.kind = SectionKind::FREE_TEXT;
    prefix.locator_pattern = "^(APP)";
    prefix.regex_tokens = {
        {"vendor", "^(APP)", 1}
    };

    SectionRule header;
    header.id = "header_kv";
    header.kind = SectionKind::KV;
    header.locator_pattern = "((?:severity|msg|context)[^\\{]+)(?=payload=)";

    SectionRule payload;
    payload.id = "payload_json";
    payload.kind = SectionKind::JSON;
    payload.locator_pattern = "payload=(\\{.*\\})$";
    payload.json_matches = {
        {"$.event.type", std::string("login")},
        {"$.event.outcome", std::string("success")}
    };
    payload.json_tokens = {
        {"event.type", "$.event.type"},
        {"event.outcome", "$.event.outcome"},
        {"user.id", "$.user.id"},
        {"src.ip", "$.network.src.ip"}
    };

    rule.sections = {prefix, header, payload};
    return ParserFramework({rule});
}

int main() {
    const std::string message =
        "APP severity=info msg=\"user login\" context=5 "
        "payload={\"event\":{\"type\":\"login\",\"outcome\":\"success\"},"
        "\"user\":{\"id\":\"alice\"},\"network\":{\"src\":{\"ip\":\"10.0.0.8\"}}}";

    ParserFramework framework = build_demo_framework();
    ParseResult result = framework.parse_message(message);

    std::cout << "matched=" << (result.matched ? "true" : "false") << "\n";
    std::cout << "rule=" << result.message_rule_name << "\n";

    for (const auto& token : result.tokens) {
        std::cout
            << token.section
            << " [" << token.source_engine << "] "
            << token.name
            << "=" << token.value
            << "\n";
    }

    if (!result.errors.empty()) {
        std::cout << "errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  " << error << "\n";
        }
    }

    return result.matched ? 0 : 1;
}
