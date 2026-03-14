%require "3.5"
%language "c++"

%define api.namespace {DynamicProperty}
%define api.parser.class {Parser}
%define api.value.type variant
%define parse.error verbose
%define api.token.constructor

%code requires {
    #include <memory>
    #include <string>
    #include <vector>
    #include "Ast.h"

    typedef void* yyscan_t;

    namespace DynamicProperty {
    class Driver;
    }
}

%parse-param { DynamicProperty::Driver& driver }
%parse-param { yyscan_t scanner }
%lex-param { yyscan_t scanner }
%lex-param { DynamicProperty::Driver& driver }

%code {
    #include "Compiler.h"

    DynamicProperty::Parser::symbol_type dynprop_lex(yyscan_t scanner, DynamicProperty::Driver& driver);
    #define yylex dynprop_lex
}

%token IF THEN ELSE AND OR
%token EQ NE LT LE GT GE CONTAINS
%token ASSIGN PLUS MINUS TIMES DIVIDE
%token LPAREN RPAREN
%token <std::string> IDENT STRING
%token <int> NUMBER

%type <DynamicProperty::ConditionalAstPtr> conditional
%type <std::vector<DynamicProperty::AssignmentStatement>> assignment_list else_clause
%type <DynamicProperty::AssignmentStatement> assignment
%type <DynamicProperty::BooleanExpressionPtr> bool_expr or_expr and_expr predicate
%type <DynamicProperty::ValueExpressionPtr> value_expr term primary

%left OR
%left AND
%nonassoc EQ NE LT LE GT GE CONTAINS
%left PLUS MINUS
%left TIMES DIVIDE

%start conditional

%%

conditional:
    IF bool_expr THEN assignment_list else_clause {
        $$ = std::make_shared<DynamicProperty::ConditionalAst>($2, std::move($4), std::move($5));
        driver.set_result($$);
    }
;

else_clause:
    %empty {
        $$ = {};
    }
    | ELSE assignment_list {
        $$ = std::move($2);
    }
;

assignment_list:
    assignment {
        $$.push_back(std::move($1));
    }
    | assignment_list AND assignment {
        $$ = std::move($1);
        $$.push_back(std::move($3));
    }
;

assignment:
    IDENT ASSIGN value_expr {
        $$ = DynamicProperty::AssignmentStatement(std::move($1), $3);
    }
;

bool_expr:
    or_expr {
        $$ = $1;
    }
;

or_expr:
    and_expr {
        $$ = $1;
    }
    | or_expr OR and_expr {
        $$ = std::make_shared<DynamicProperty::LogicalExpression>(DynamicProperty::LogicalOperator::OR, $1, $3);
    }
;

and_expr:
    predicate {
        $$ = $1;
    }
    | and_expr AND predicate {
        $$ = std::make_shared<DynamicProperty::LogicalExpression>(DynamicProperty::LogicalOperator::AND, $1, $3);
    }
;

predicate:
    LPAREN bool_expr RPAREN {
        $$ = $2;
    }
    | value_expr EQ value_expr {
        $$ = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::EQUAL, $1, $3);
    }
    | value_expr NE value_expr {
        $$ = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::NOT_EQUAL, $1, $3);
    }
    | value_expr LT value_expr {
        $$ = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::LESS_THAN, $1, $3);
    }
    | value_expr LE value_expr {
        $$ = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::LESS_OR_EQUAL, $1, $3);
    }
    | value_expr GT value_expr {
        $$ = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::GREATER_THAN, $1, $3);
    }
    | value_expr GE value_expr {
        $$ = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::GREATER_OR_EQUAL, $1, $3);
    }
    | value_expr CONTAINS value_expr {
        $$ = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::CONTAINS, $1, $3);
    }
;

value_expr:
    term {
        $$ = $1;
    }
    | value_expr PLUS term {
        $$ = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::ADD, $1, $3);
    }
    | value_expr MINUS term {
        $$ = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::SUBTRACT, $1, $3);
    }
;

term:
    primary {
        $$ = $1;
    }
    | term TIMES primary {
        $$ = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::MULTIPLY, $1, $3);
    }
    | term DIVIDE primary {
        $$ = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::DIVIDE, $1, $3);
    }
;

primary:
    IDENT {
        $$ = std::make_shared<DynamicProperty::IdentifierExpression>(std::move($1));
    }
    | STRING {
        $$ = std::make_shared<DynamicProperty::StringLiteralExpression>(std::move($1));
    }
    | NUMBER {
        $$ = std::make_shared<DynamicProperty::NumberLiteralExpression>($1);
    }
    | LPAREN value_expr RPAREN {
        $$ = $2;
    }
;

%%

void DynamicProperty::Parser::error(const std::string& msg) {
    driver.set_error(msg);
}
