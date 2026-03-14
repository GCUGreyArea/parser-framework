#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
#include <fstream>
#include <string>
#include <libParser.h>

TEST(testYAML,testYAMLFileCanBeRead) {
    std::ifstream ifs("test/resources/test.yaml");
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
                       (std::istreambuf_iterator<char>()    ) );


    ASSERT_TRUE(content != "");
}

TEST(testYAML,testYAMLFileIsLoaded) {
    constexpr const char * ACCEPT_MSG = "CheckPoint 9929 - [action:\"Accept\"; conn_direction:\"Outgoing\"; \
        contextnum:\"1\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; logid:\"0\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; \
        origin:\"<IP>\"; originsicname:\"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; \
        __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; \
        context_num:\"1\"; dst:\"<IP>\"; hll_key:\"3383197236451420632\"; layer_name:\"<name>\"; layer_name:\"<name>\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; \
        layer_uuid:\"3866c7f4-f88d-402e-abde-252a2426d1d7\"; match_id:\"48\"; match_id:\"16777222\"; parent_rule:\"0\"; parent_rule:\"0\"; rule_action:\"Accept\"; rule_action:\"Accept\"; \
        rule_name:\"Outbound access\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; rule_uid:\"4eb09b29-ccc2-4374-b33a-e66660e3916d\"; nat_addtnl_rulenum:\"0\"; nat_rulenum:\"400\"; \
        product:\"VPN-1 & FireWall-1\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\"; src:\"<IP>\"; xlatedport:\"0\"; xlatedst:\"<IP>\"; xlatesport:\"28650\"; xlatesrc:\"<IP>\"]";
    constexpr const char * REJECT_MSG = "CheckPoint 9929 - [action:\"Reject\"; conn_direction:\"Outgoing\"; \
        contextnum:\"1\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; logid:\"0\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; \
        origin:\"<IP>\"; originsicname:\"CN=Ord1fw1b,O=bos1cpmgmt1.aspect.com.2twi3g\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; \
        __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; \
        context_num:\"1\"; dst:\"<IP>\"; hll_key:\"3383197236451420632\"; layer_name:\"<name>\"; layer_name:\"<name>\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; \
        layer_uuid:\"3866c7f4-f88d-402e-abde-252a2426d1d7\"; match_id:\"48\"; match_id:\"16777222\"; parent_rule:\"0\"; parent_rule:\"0\"; rule_action:\"Accept\"; rule_action:\"Accept\"; \
        rule_name:\"Outbound access\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; rule_uid:\"4eb09b29-ccc2-4374-b33a-e66660e3916d\"; nat_addtnl_rulenum:\"0\"; nat_rulenum:\"400\"; \
        product:\"VPN-1 & FireWall-1\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\"; src:\"<IP>\"; xlatedport:\"0\"; xlatedst:\"<IP>\"; xlatesport:\"28650\"; xlatesrc:\"<IP>\"]";

    YamlFile fl("test/resources/test.yaml");

    fl.readFile();

    std::unique_ptr<Rule> rule = fl.getRule();

    Match m(ACCEPT_MSG);
    bool matched = rule->match(m);
    ASSERT_TRUE(matched);


    Pattern* ptn = m.getMathcedPattern();

    // ptn->print();

    std::vector<std::shared_ptr<Token>>& tokens = ptn->get_tokens();

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

    m.reset(REJECT_MSG);

    matched = rule->match(m);
    ASSERT_TRUE(matched);

    ptn = m.getMathcedPattern();
    tokens = ptn->get_tokens();

    ASSERT_EQ(tokens[0]->as_string(),"Reject");
}