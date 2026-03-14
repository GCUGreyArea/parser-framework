#include <gtest/gtest.h>
#include <glog/logging.h>

#include <re2/re2.h>
#include <iostream>
#include <libParser.h>

#include "testUtils.h"

TEST(testRegexp,testBasicCaptureGroup) {
    constexpr const char * REGEX_STR = "Test=(?P<test>[a-z]+)";
    constexpr const char * PTN = "Test=active and new";

    Match m(PTN);
    Regex rx(REGEX_STR);

    m.set_match_type(Match::Type::SEQUENTIAL);


    if(rx.match(m)) {
        ASSERT_EQ(m.offset(),11);
        ASSERT_EQ(rx.as_string(),"active");

        std::string str1(m.data());
        std::string str2(" and new");
        ASSERT_TRUE(str1==str2);

    }
    else {
        ASSERT_FALSE(true);
    }

}

TEST(testRegexp,testToken) {
    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * PTN = "Test=active and=new next={test:one}";


    Token tk("Test", TK1, ValueType::STRING,0);
    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);


    if(tk.match(m)) {
        ASSERT_EQ(m.offset(),11);
        ASSERT_EQ(tk.as_string(),"active");

        // std::cout << "Match : [" << m.data() << "]" << std::endl;
        std::string str1(m.data());
        std::string str2(" and=new next={test:one}");
        ASSERT_TRUE(str1==str2);

        tk.reset();
        ASSERT_FALSE(tk.match(m));

    }
    else {
        ASSERT_FALSE(true);
    }
}


TEST(testRegexp,testPattern) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[a-z]+)";
    constexpr const char * TK3 = "next=(?P<two>\\{[a-z:]+\\})";
    constexpr const char * PTN = "Test=active and=new next={test:one}";

    AnchorPattern anchor("anchor",ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern", PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::STRING,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    ptn.validate();

    Match m(PTN);

    m.set_match_type(Match::Type::SEQUENTIAL);

    if(ptn.match(m)) {
        std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

        ASSERT_EQ(tokens[0]->name(),"tk1");
        ASSERT_EQ(tokens[1]->name(),"tk2");
        ASSERT_EQ(tokens[2]->name(),"tk3");
        ASSERT_EQ(tokens[0]->as_string(),"active");
        ASSERT_EQ(tokens[1]->as_string(),"new");
        ASSERT_EQ(tokens[2]->as_string(),"{test:one}");
    }
    else {
        ASSERT_FALSE(true);
    }

}


TEST(testRegexp,testPatternDoesntMatch) {
    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";

    constexpr const char * ANCHOR = "^Test=(?P<pattern>[ a-z\\=\\{\\}:]+)";

    constexpr const char * TK1 = "Test=(?P<test>[a-z]+)";
    constexpr const char * TK2 = "and=(?P<one>[a-z]+)";
    constexpr const char * TK3 = "next=(?P<one>\\{[a-z:]+\\})";

    constexpr const char * PTN1 = "Test=active and=new five=2 next={test:one}";
    constexpr const char * PTN2 = "TestTwo=active and=new next={test:one}";

    AnchorPattern anchor("anchor", ANCHOR_ID, ANCHOR);

    Pattern ptn("pattern",PATTERN_ID, anchor,1);

    ptn.add_token("tk1", TK1, ValueType::STRING,0);
    ptn.add_token("tk2", TK2, ValueType::STRING,1);
    ptn.add_token("tk3", TK3, ValueType::STRING,2);

    Match m1(PTN1);
    m1.set_match_type(Match::Type::SEQUENTIAL);

    bool res = ptn.match(m1);
    ASSERT_TRUE(res);

    Match m2(PTN2);
    m2.set_match_type(Match::Type::SEQUENTIAL);

    ptn.reset();
    res = ptn.match(m2);
    ASSERT_FALSE(res);

    ptn.validate();
}

TEST(testRegexp,testPatternWithExample) {
    constexpr const char * ANCHOR = "^(CheckPoint)";

    constexpr const char * TK1  = "\\[action:\"([Aa-z]+)\";";
    constexpr const char * TK2  = "contextnum:\"([0-9]+)\"";
    constexpr const char * TK3  = "flags:\"([0-9]+)\"";
    constexpr const char * TK4  = "ifdir:\"([a-z]+)\"";
    constexpr const char * TK5  = "ifname:\"([a-z0-9]+)\"";
    constexpr const char * TK6  = "loguid:\"([\\{\\}a-z0-9,]+)\"";
    constexpr const char * TK7  = "sequencenum:\"([0-9]+)\";";
    constexpr const char * TK8  = "time:\"([0-9]+)\";";
    constexpr const char * TK9  = "version:\"([0-9]+)\";";
    constexpr const char * TK10 = "db_tag=\\{([0-9A-F\\-]+)\\};";
    constexpr const char * TK11 = "layer_uuid:\"([0-9\\-a-f]+)\"";
    constexpr const char * TK13 = "match_id:\"([0-9]+)\";";
    constexpr const char * TK15 = "rule_uid:\"([0-9a-f\\-]+)\";";
    constexpr const char * TK16 = "proto:\"([0-9]+)\";";
    constexpr const char * TK17 = "s_port:\"([0-9]+)\";";
    constexpr const char * TK18 = "service:\"([0-9]+)\";";
    constexpr const char * TK19 = "service_id:\"(https|http|scp)\";";

    constexpr const char * PTN1 = "CheckPoint 9929 - [action:\"Accept\"; conn_direction:\"Outgoing\"; \
        contextnum:\"1\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; logid:\"0\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; \
        origin:\"<IP>\"; originsicname:\"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; \
        __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; \
        context_num:\"1\"; dst:\"<IP>\"; hll_key:\"3383197236451420632\"; layer_name:\"<name>\"; layer_name:\"<name>\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; \
        layer_uuid:\"3866c7f4-f88d-402e-abde-252a2426d1d7\"; match_id:\"48\"; match_id:\"16777222\"; parent_rule:\"0\"; parent_rule:\"0\"; rule_action:\"Accept\"; rule_action:\"Accept\"; \
        rule_name:\"Outbound access\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; rule_uid:\"4eb09b29-ccc2-4374-b33a-e66660e3916d\"; nat_addtnl_rulenum:\"0\"; nat_rulenum:\"400\"; \
        product:\"VPN-1 & FireWall-1\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\"; src:\"<IP>\"; xlatedport:\"0\"; xlatedst:\"<IP>\"; xlatesport:\"28650\"; xlatesrc:\"<IP>\"]";

    constexpr const char * PTN2 = "3aws: 9929 - [action:\"Accept\"; conn_direction:\"Outgoing\"; \
        contextnum:\"1\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; logid:\"0\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; \
        origin:\"<IP>\"; originsicname:\"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; \
        __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; \
        context_num:\"1\"; dst:\"<IP>\"; hll_key:\"3383197236451420632\"; layer_name:\"<name>\"; layer_name:\"<name>\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; \
        layer_uuid:\"3866c7f4-f88d-402e-abde-252a2426d1d7\"; match_id:\"48\"; match_id:\"16777222\"; parent_rule:\"0\"; parent_rule:\"0\"; rule_action:\"Accept\"; rule_action:\"Accept\"; \
        rule_name:\"Outbound access\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; rule_uid:\"4eb09b29-ccc2-4374-b33a-e66660e3916d\"; nat_addtnl_rulenum:\"0\"; nat_rulenum:\"400\"; \
        product:\"VPN-1 & FireWall-1\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\"; src:\"<IP>\"; xlatedport:\"0\"; xlatedst:\"<IP>\"; xlatesport:\"28650\"; xlatesrc:\"<IP>\"]";

    constexpr const char * ANCHOR_ID = "E126BCEC-EDD6-E3C2-466B-3493AB81AA83";
    constexpr const char * PATTERN_ID = "0538061F-7EF4-C09C-5476-81A41A63DC02";


    AnchorPattern anchor("anchor", ANCHOR_ID, ANCHOR);

    Pattern ptn("test", PATTERN_ID, anchor,1);

    ptn.add_token("action",       TK1,  ValueType::STRING,0);
    ptn.add_token("contextnum",   TK2,  ValueType::STRING,1);
    ptn.add_token("flags",        TK3,  ValueType::STRING,2);
    ptn.add_token("ifdir",        TK4,  ValueType::STRING,3);
    ptn.add_token("ifname",       TK5,  ValueType::STRING,4);
    ptn.add_token("loguid",       TK6,  ValueType::STRING,5);
    ptn.add_token("sequencenum",  TK7,  ValueType::STRING,6);
    ptn.add_token("time",         TK8,  ValueType::STRING,7);
    ptn.add_token("version",      TK9,  ValueType::STRING,8);
    ptn.add_token("db_tag",       TK10, ValueType::STRING,9);
    ptn.add_token("layer_uuid",   TK11, ValueType::STRING,10);
    ptn.add_token("match_id",     TK13, ValueType::STRING,11);
    ptn.add_token("rule_uid",     TK15, ValueType::STRING,12);
    ptn.add_token("proto",        TK16, ValueType::STRING,13);
    ptn.add_token("s_port",       TK17, ValueType::STRING,14);
    ptn.add_token("service",      TK18, ValueType::STRING,15);
    ptn.add_token("service_id",   TK19, ValueType::STRING,16);

    ptn.validate();

    Match m(PTN1);
    bool res = ptn.match(m);
    ASSERT_TRUE(res);


    std::vector<std::shared_ptr<Token>>& tokens = ptn.get_tokens();

    ASSERT_EQ(tokens[0]->as_string(),"Accept");
    ASSERT_EQ(tokens[1]->as_string(),"1");
    ASSERT_EQ(tokens[2]->as_string(),"7263232");
    ASSERT_EQ(tokens[3]->as_string(),"outbound");
    ASSERT_EQ(tokens[4]->as_string(),"eth2");
    ASSERT_EQ(tokens[5]->as_string(),"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}");
    ASSERT_EQ(tokens[6]->as_string(),"110");
    ASSERT_EQ(tokens[7]->as_string(),"1630417161");
    ASSERT_EQ(tokens[8]->as_string(),"5");
    ASSERT_EQ(tokens[9]->as_string(),"599255D7-8B94-704F-9D9A-CFA3719EA5CE");
    ASSERT_EQ(tokens[10]->as_string(),"9d7748a0-845f-491d-9eef-1fb41680bc35");
    ASSERT_EQ(tokens[11]->as_string(),"48");
    ASSERT_EQ(tokens[12]->as_string(),"8bf81033-b6c7-44fe-a88a-2068c155f50e");
    ASSERT_EQ(tokens[13]->as_string(),"6");
    ASSERT_EQ(tokens[14]->as_string(),"44853");
    ASSERT_EQ(tokens[15]->as_string(),"443");
    ASSERT_EQ(tokens[16]->as_string(),"https");

    // ptn.print();

    m.reset(PTN2);
    ptn.reset();
    res = ptn.match(m);
    ASSERT_FALSE(res);
}

TEST(testRegexp,testRegexSubstitution) {
    constexpr const char * RGX  = "rule_uid:\"([0-9a-f\\-]+)\";";
    constexpr const char * SUBR = "name";

    std::string str = RGX;

    std::string sub = "(";
    sub.append(SUBR);
    sub.append(")");

    RE2 re("\\(.*\\)");

    RE2::GlobalReplace(&str,re,sub);

    RE2 rx(str);
    std::string res;

    str = "rule_uid:\"name\";";

    ASSERT_EQ(RE2::PartialMatch(str, rx, &res),true);
}