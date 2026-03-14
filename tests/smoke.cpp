#include <cassert>
#include <string>

#include "parser_framework/ParserFramework.hpp"

using namespace parser_framework;

static const Token* find_token(const ParseResult& result, const std::string& name) {
    for (const auto& token : result.tokens) {
        if (token.name == name) {
            return &token;
        }
    }

    return nullptr;
}

int main() {
    MessageRule rule;
    rule.id = "app_event";
    rule.name = "Application Event";
    rule.anchor_patterns = {"^APP\\b"};

    SectionRule header;
    header.id = "header";
    header.kind = SectionKind::KV;
    header.locator_pattern = "((?:severity|msg|context)[^\\{]+)(?=payload=)";

    SectionRule payload;
    payload.id = "payload";
    payload.kind = SectionKind::JSON;
    payload.locator_pattern = "payload=(\\{.*\\})$";
    payload.json_matches = {
        {"$.event.type", std::string("login")}
    };
    payload.json_tokens = {
        {"event.type", "$.event.type"},
        {"user.id", "$.user.id"}
    };

    rule.sections = {header, payload};

    ParserFramework framework({rule});
    ParseResult result = framework.parse_message(
        "APP severity=info msg=\"user login\" context=5 "
        "payload={\"event\":{\"type\":\"login\"},\"user\":{\"id\":\"alice\"}}");

    assert(result.matched);
    assert(result.errors.empty());

    const Token* severity = find_token(result, "severity");
    assert(severity != nullptr);
    assert(severity->value == "info");

    const Token* message = find_token(result, "msg");
    assert(message != nullptr);
    assert(message->value == "user login");

    const Token* event_type = find_token(result, "event.type");
    assert(event_type != nullptr);
    assert(event_type->value == "login");

    const Token* user_id = find_token(result, "user.id");
    assert(user_id != nullptr);
    assert(user_id->value == "alice");

    return 0;
}
