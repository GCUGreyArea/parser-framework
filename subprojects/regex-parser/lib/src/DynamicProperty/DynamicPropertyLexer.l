%option noyywrap
%option nounput
%option noinput
%option batch
%option never-interactive
%option reentrant

%{
#include <stdexcept>
#include <string>

#include "Compiler.h"
#include "DynamicPropertyParser.hpp"

typedef void* yyscan_t;

static std::string unescape_string(const char * text, int length) {
    std::string input(text, static_cast<size_t>(length));
    std::string result;
    result.reserve(input.size());

    for(size_t idx = 1; idx + 1 < input.size(); ++idx) {
        char ch = input[idx];
        if(ch == '\\' && idx + 1 < input.size() - 1) {
            ++idx;
            switch(input[idx]) {
                case '\\':
                    result.push_back('\\');
                    break;
                case '"':
                    result.push_back('"');
                    break;
                case 'n':
                    result.push_back('\n');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                default:
                    result.push_back(input[idx]);
                    break;
            }
        }
        else {
            result.push_back(ch);
        }
    }

    return result;
}

#define YY_DECL DynamicProperty::Parser::symbol_type dynprop_lex(yyscan_t yyscanner, DynamicProperty::Driver& driver)
%}

%%

[ \t\r\n]+              ;
"if"                    { return DynamicProperty::Parser::make_IF(); }
"then"                  { return DynamicProperty::Parser::make_THEN(); }
"else"                  { return DynamicProperty::Parser::make_ELSE(); }
"and"                   { return DynamicProperty::Parser::make_AND(); }
"or"                    { return DynamicProperty::Parser::make_OR(); }
"=="                    { return DynamicProperty::Parser::make_EQ(); }
"!="                    { return DynamicProperty::Parser::make_NE(); }
"<="                    { return DynamicProperty::Parser::make_LE(); }
">="                    { return DynamicProperty::Parser::make_GE(); }
"\?="                   { return DynamicProperty::Parser::make_CONTAINS(); }
"<"                     { return DynamicProperty::Parser::make_LT(); }
">"                     { return DynamicProperty::Parser::make_GT(); }
"="                     { return DynamicProperty::Parser::make_ASSIGN(); }
"+"                     { return DynamicProperty::Parser::make_PLUS(); }
"-"                     { return DynamicProperty::Parser::make_MINUS(); }
"*"                     { return DynamicProperty::Parser::make_TIMES(); }
"/"                     { return DynamicProperty::Parser::make_DIVIDE(); }
"("                     { return DynamicProperty::Parser::make_LPAREN(); }
")"                     { return DynamicProperty::Parser::make_RPAREN(); }
\"([^\\\"]|\\.)*\"      { return DynamicProperty::Parser::make_STRING(unescape_string(yytext, yyleng)); }
[0-9]+                  { return DynamicProperty::Parser::make_NUMBER(std::stoi(yytext)); }
[A-Za-z_][A-Za-z0-9_.$]* { return DynamicProperty::Parser::make_IDENT(std::string(yytext, static_cast<size_t>(yyleng))); }
.                       {
                            driver.set_error(std::string("Unexpected character in dynamic property expression [") + yytext[0] + "]");
                            throw DynamicProperty::Parser::syntax_error(driver.error());
                        }

%%
