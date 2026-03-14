#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
#include <libParser.h>
#include <stdlib.h>
#include <stdio.h>


TEST(testExpressionsAndValues,testThatBaseValueMatchesToken) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=1024 next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        BaseValue v1("active");
        BaseValue v2(1024);
        BaseValue v3("{test:one}");


        auto t1 = tokens[0];
        auto t2 = tokens[1];
        auto t3 = tokens[2];

        BaseValue tv1(t1);
        BaseValue tv2(t2);
        BaseValue tv3(t3);

        ASSERT_EQ(tokens[0]->name(),"tk1");
        ASSERT_EQ(tokens[1]->name(),"tk2");
        ASSERT_EQ(tokens[2]->name(),"tk3");
        bool res = (tv1 == v1);
        ASSERT_TRUE(res);
        res = (tv2 == v2);
        ASSERT_TRUE(res);
        res = (tv3 == v3);
        ASSERT_TRUE(res);
    }
    else {
        ASSERT_FALSE(true);
    }
}


TEST(testExpressionsAndValues, testConditionMatchesForTokens) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=1024 next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        BaseValue v1("active");
        BaseValue v2(1024);
        BaseValue v3("{test:one}");


        BaseValue t1(tokens[0]);
        BaseValue t2(tokens[1]);
        BaseValue t3(tokens[2]);



        Condition c1(t1,Condition::Operator::EQUAL_TO,v1);
        Condition c2(t2,Condition::Operator::EQUAL_TO,v2);
        Condition c3(t3,Condition::Operator::EQUAL_TO,v3);

        ASSERT_TRUE(c1.evaluate());
        ASSERT_TRUE(c2.evaluate());
        ASSERT_TRUE(c3.evaluate());
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testThatExpressionCanMatchByTree) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=1024 next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        BaseValue v1("active");
        BaseValue v2(1024);
        BaseValue v3("{test:one}");


        BaseValue t1(tokens[0]);
        BaseValue t2(tokens[1]);
        BaseValue t3(tokens[2]);

        Condition c1(t1,Condition::Operator::EQUAL_TO,v1);
        Condition c2(t2,Condition::Operator::EQUAL_TO,v2);
        Condition c3(t3,Condition::Operator::EQUAL_TO,v3);

        std::shared_ptr<BoolExpression> e1 = std::make_shared<BoolExpression>(c1,BoolExpression::Conjunction::AND,c2);
        BoolExpression e2(e1,BoolExpression::Conjunction::AND,c3);

        ASSERT_TRUE(e2.evaluate());
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testExpressionEvaluatesWithToken) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=1024 next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        BaseValue v1("active");
        BaseValue v2(1024);
        BaseValue v3("one");


        BaseValue t1(tokens[0]);
        BaseValue t2(tokens[1]);
        BaseValue t3(tokens[2]);

        Condition c1(t1,Condition::Operator::EQUAL_TO,v1);
        Condition c2(t2,Condition::Operator::EQUAL_TO,v2);
        Condition c3(t3,Condition::Operator::STR_CONTAINS,v3);

        std::shared_ptr<BoolExpression> e1 = std::make_shared<BoolExpression>(c1,BoolExpression::Conjunction::AND,c2);
        BoolExpression e2(e1,BoolExpression::Conjunction::AND,c3);

        ASSERT_TRUE(e2.evaluate());
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testThatVariablesCanMatchToken) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=1024 next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        auto t3 = tokens[2];

        Variable v3("var3",t3);
        Variable v4("var4", "{test:one}");

        bool ret = (v3 == v4);
        ASSERT_TRUE(ret);

        ret = (v4 == v3);
        ASSERT_TRUE(ret);
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testThatVariablesCanBeAssigneddValueFromToken) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=1024 next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        auto t3 = tokens[2];

        Variable v3("var3",t3);
        Variable v4("var4", "");

        v4 = v3;

        bool ret = (v3 == v4);
        ASSERT_TRUE(ret);

        ret = (v4 == v3);
        ASSERT_TRUE(ret);

        ASSERT_EQ(v3.as_string(),"{test:one}");
        ASSERT_EQ(v4.as_string(),"{test:one}");
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testVariablesForIntegerTypesConvert) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>[0-9]+)";
    constexpr const char * PTN = "Test=active and=1024 next=1024";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::INT,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        auto t3 = tokens[2];

        Variable v3("var3",t3);
        Variable v4("var4", 0);

        v4 = v3;

        bool ret = (v3 == v4);
        ASSERT_TRUE(ret);

        ret = (v4 == v3);
        ASSERT_TRUE(ret);

        std::string nstr = v3.as_string();
        ASSERT_EQ(v3.as_string(),"1024");

        nstr = v4.as_string();
        ASSERT_EQ(nstr,"1024");

        ASSERT_EQ(1024,v4.as_int());
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testConditionalAssignmentOperators) {
    Variable v1("v1","test");

    Variable v2("v2","");

    v2 = v1;

    ASSERT_EQ(v2.as_string(), v1.as_string());
}

TEST(testExpressionsAndValues, testAssignmentExpressions) {
    std::shared_ptr<Variable> value = std::make_shared<Variable>("value","nothing", false);

    bool const_value = true;
    std::vector<std::shared_ptr<Variable>> vars {
        value,
        {std::make_shared<Variable>("const1", "one",   const_value)},
        {std::make_shared<Variable>("const2", ".",     const_value)},
        {std::make_shared<Variable>("const3", "two",   const_value)},
        {std::make_shared<Variable>("const4", ".",     const_value)},
        {std::make_shared<Variable>("const4", "three", const_value)},
        {std::make_shared<Variable>("const5", ".",     const_value)},
        {std::make_shared<Variable>("const6", "four",  const_value)}
    };
    std::vector<Assignment::Operator> ops{
        Assignment::EQUALS,
        Assignment::PLUS,
        Assignment::PLUS,
        Assignment::PLUS,
        Assignment::PLUS,
        Assignment::PLUS,
        Assignment::PLUS
    };

    auto a1 = Assignment::build(vars,ops);
    ASSERT_TRUE(a1 != nullptr);

    auto a2 = a1->evaluate();

    std::string str = a2->as_string();

    // std::cout << "Variable [" << a2.as_string() << "]" << std::endl;
    ASSERT_EQ(str,"one.two.three.four");
}

TEST(testExpressionsAndValues, testAssignmentExpressionsWithSimpleTokens) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=1024 next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        // std::cout << "Got tokens [size " << tokens.size() << "]" << std::endl;

        std::vector<std::shared_ptr<Variable>> vars {
            {std::make_shared<Variable>("sum","",false)},
            {std::make_shared<Variable>("token2",tokens[1])},
            {std::make_shared<Variable>("var2", ".")},
            {std::make_shared<Variable>("token3",tokens[2])}
        };

        std::vector<Assignment::Operator> ops{
            Assignment::EQUALS,
            Assignment::PLUS,
            Assignment::PLUS
        };

        auto a1 = Assignment::build(vars,ops);
        auto a2 = a1->evaluate();


        ASSERT_EQ(a2->as_string(),"1024.{test:one}");
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testAssignmentExpressionsWithIntTokens) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>[0-9]+)";
    constexpr const char * PTN = "Test=active and=1024 next=1024";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::INT,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        // std::cout << "Got tokens [size " << tokens.size() << "]" << std::endl;

        std::vector<std::shared_ptr<Variable>> vars {
            {std::make_shared<Variable>("sum","",false)},
            {std::make_shared<Variable>("token1",tokens[0])},
            {std::make_shared<Variable>("var1", ".")},
            {std::make_shared<Variable>("token2",tokens[1])},
            {std::make_shared<Variable>("token3",tokens[2])}
        };

        std::vector<Assignment::Operator> ops{
            Assignment::EQUALS,
            Assignment::PLUS,
            Assignment::PLUS,
            Assignment::PLUS
        };

        auto a1 = Assignment::build(vars,ops);
        auto a2 = a1->evaluate();


        ASSERT_EQ(a2->as_string(),"active.2048");
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues, testConditionalAssignmentAssignsValues) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[0-9]+)";
    constexpr const char * TK3 = "next=(?P<two>[0-9]+)";
    constexpr const char * PTN = "Test=active and=1024 next=1024";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::INT,1);
    ptn.add_token("tk3", TK3, ValueType::INT,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        EXPECT_EQ(tokens[0]->as_string(),"active");
        EXPECT_EQ(tokens[1]->as_int(),1024);
        EXPECT_EQ(tokens[2]->as_int(),1024);

        std::vector<std::shared_ptr<Variable>> vars {
            {std::make_shared<Variable>("sum","",false)},
            {std::make_shared<Variable>("token1",tokens[0])},
            {std::make_shared<Variable>("var1", ".")},
            {std::make_shared<Variable>("token2",tokens[1])},
            {std::make_shared<Variable>("token3",tokens[2])}
        };

        std::vector<Assignment::Operator> ops{
            Assignment::EQUALS,
            Assignment::PLUS,
            Assignment::PLUS,
            Assignment::PLUS
        };

        auto a1 = Assignment::build(vars,ops);

        BaseValue v1("active");
        BaseValue v2(1024);
        BaseValue v3(4096);


        BaseValue t1(tokens[0]);
        BaseValue t2(tokens[1]);
        BaseValue t3(tokens[2]);


        bool res = t3 < v3;
        ASSERT_TRUE(res);

        Condition c1(t1,Condition::Operator::EQUAL_TO,v1);
        Condition c2(t2,Condition::Operator::EQUAL_TO,v2);
        Condition c3(t3,Condition::Operator::LESS_THAN,v3);

        std::shared_ptr<BoolExpression> e1 = std::make_shared<BoolExpression>(c1,BoolExpression::Conjunction::AND,c2);
        std::shared_ptr<BoolExpression> e2 = std::make_shared<BoolExpression>(e1,BoolExpression::Conjunction::AND,c3);

        res = e2->evaluate();
        ASSERT_TRUE(res);

        // std::shared_ptr<BoolExpression> ifEx {e2};
        std::vector<std::shared_ptr<Assignment>> thenEx{a1};


        ConditionalAssignment ca(e2,thenEx);

        ca.evaluate();

        std::vector<ResolvedProperty> values = ca.get_results();


        ASSERT_EQ(values[0].name,"sum");
        ASSERT_EQ(values[0].value,"active.2048");
    }
    else {
        ASSERT_FALSE(true);
    }
}

TEST(testExpressionsAndValues,testNumbersAreResolvedCorrectly) {
    std::string str = "0x10";

    unsigned int base = Numbers::is_number(str);

    ASSERT_EQ(base,16);

    size_t num = Numbers::to_int(str,base);
    ASSERT_EQ(num,16);

    str = "10";
    base = Numbers::is_number(str);
    ASSERT_EQ(base,10);
    num = Numbers::to_int(str,base);
    ASSERT_EQ(num,10);

    str = "010";
    base = Numbers::is_number(str);
    ASSERT_EQ(base,8);
    num = Numbers::to_int(str,base);
    ASSERT_EQ(num,8);


    str = "0b10";
    base = Numbers::is_number(str);
    ASSERT_EQ(base,2);
    num = Numbers::to_int(str,base);
    ASSERT_EQ(num,2);
}

TEST(Exception, testExceptionIsThrown)
{
    EXPECT_THROW(Numbers::to_int("0b111",5),Exception::NotAValidBase);

    bool thrown=false;
    try {
        Numbers::to_int("0b111",5);
    }
    catch( Exception::NotAValidBase& e) {
        std::string str = e.what();
        EXPECT_EQ(str,"Not a valid number base, did you call is_number first? [base 5]");
        thrown=true;
    }

    ASSERT_TRUE(thrown);
}
