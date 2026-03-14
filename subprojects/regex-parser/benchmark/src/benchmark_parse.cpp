#include <benchmark/benchmark.h>

#include <iostream>
#include <regex>
#include <string>

#include <array>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <signal.h>
#include <unistd.h>

#include <nlohmann/json.hpp>
#include <libParser.h>

std::string exec(const char *cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

static void parse_message(std::vector<std::unique_ptr<Rule>> &rules,
                          std::string message, JSONOut &out) {
  Match m(message);

  for (auto &rule : rules) {
    if (rule->match(m)) {
      out.add_rule_value(*rule);
      break;
    }
  }
}

/**
 * @brief read and process all rules
 *
 * @param dir
 * @param rules
 * @return true
 * @return false
 */
static bool read_rules(std::string dir,
                       std::vector<std::unique_ptr<Rule>> &rules) {
  // Find all the YAML config files
  std::string cmd("find ");
  cmd.append(dir).append(" -name '*.yaml'");
  std::string result = exec(cmd.c_str());

  // Read in the YAML config files ad build the rules
  std::istringstream iss(result);

  for (std::string file; std::getline(iss, file);) {
    YamlFile fl(file);

    try {
      fl.readFile();
    } catch (std::exception e) {
      std::string str = e.what();
      std::cerr << "Exception trying to read [file " << file << " / exception "
                << str << "]" << std::endl;
      return false;
    }

    std::unique_ptr<Rule> rule = fl.getRule();
    rules.push_back(std::move(rule));
  }

  return true;
}

/**
 * @brief Read all messages from directory for log files into message vector
 *
 * @param dir directory containing log files
 * @return true
 * @return false
 */
static bool read_logfiles(std::string dir, std::vector<std::string> &messages) {
  // Read in the log files and play each message then output the return values
  std::string cmd("find ");
  cmd.append(dir).append(" -name '*.log'");
  std::string result = exec(cmd.c_str());

  std::istringstream iss(result);
  for (std::string file; std::getline(iss, file);) {
    std::ifstream infile(file);
    if (!infile.is_open()) {
      std::cerr << "Failed to open [file " << file << "]" << std::endl;
      return false;
    }

    for (std::string message; std::getline(infile, message);) {
      messages.push_back(message);
    }
  }

  return messages.size() > 0;
}

static int get_rules_and_messages(std::vector<std::unique_ptr<Rule>> &rules,
                                  std::vector<std::string> &messages) {

  std::string dir =
      "rules";
  std::string msg = "";
  std::string logs =
      "logs";

  if (!read_rules(dir, rules)) {
    std::cerr << "Failed to read rules from [dir " << dir << "]" << std::endl;
    return -1;
  }

  if (!read_logfiles(logs, messages)) {
    std::cerr << "Failed to read messages from [dir " << logs << "]"
              << std::endl;
    return -1;
  }

  return 0;
}

static void BM_RunParser(benchmark::State &state) {
  std::vector<std::unique_ptr<Rule>> rules;
  std::vector<std::string> messages;
  get_rules_and_messages(rules, messages);

  for (auto _ : state) {
    for (auto m : messages) {
      JSONOut out;
      parse_message(rules, m, out);
    }
  }
}


static void BM_Expression(benchmark::State &state) {
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

        for (auto _ : state) {
            e2.evaluate();
        }
    }
}


// Register the function as a benchmark
BENCHMARK(BM_Expression);

// Run the benchmark
// BENCHMARK_MAIN();