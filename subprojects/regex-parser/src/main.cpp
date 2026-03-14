/**
 * @file main.cpp
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief main executable file for project
 * @version 0.1
 * @date 26/09/2021
 *
 * @copyright Copyright Barry Robinson(c) 2022
 * @defgroup main
 * @{
 */

#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <signal.h>
#include <unistd.h>

#include <nlohmann/json.hpp>
#include <hs/hs.h>

#include <libParser.h>
#include "Args.h"

/**
 * @brief Executer an OS level program like find and return
 * teh results as a string
 *
 * @param cmd
 * @return std::string
 */

static std::string exec(std::string command) {
    std::array<char, 128> buffer;
    std::string result;

	// std::cout << "Executing : " << command << std::endl;

    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe)
		throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }

	// std::cout << "Returning : " << result << std::endl;

    return result;
}
/**
 * @brief signal handler
 *
 */

bool gExit = false;

static void ctrl_handler(int sig)
{
	static int sigCount = 0;
	signal(sig, SIG_IGN);

	switch (sig)
	{
	case SIGINT:
		if(sigCount > 1) {
			std::cout << "New SIGINT, exiting abruptly" << std::endl;
			exit(0);
		}
		std::cout << "SIGINT caught -- exiting" << std::endl;
		gExit = true;
		sigCount++;
		break;

	case SIGSEGV:
	case SIGKILL:
	case SIGSTOP:
	case SIGABRT:
	case SIGBUS:
		exit(0);
	}
}

static void parse_message(std::vector<std::unique_ptr<Rule>>& rules,std::string message,JSONOut& out) {
    Match m(message);

	for(auto& rule : rules) {
		if(rule->match(m)) {
			out.add_rule_value(*rule);
			break;
		}
	}
}

/**
 * @brief Read all messages from directory for log files into message vector
 *
 * @param dir directory containing log files
 * @return true
 * @return false
 */
static bool read_logfiles(std::string dir,std::vector<std::string>& messages) {
	// Read in the log files and play each message then output the return values
	std::string cmd("find ");
	cmd.append(dir).append(" -name '*.log'");
	std::string result = exec(cmd.c_str());

	std::istringstream iss(result);
	for (std::string file; std::getline(iss, file); ) {
		std::ifstream infile(file);
		if(!infile.is_open()) {
			std::cerr << "Failed to open [file " << file << "]" << std::endl;
			return false;
		}

		for (std::string message; std::getline(infile, message); ) {
			messages.push_back(message);
		}
	}

	return messages.size() > 0;
}

/**
 * @brief Structure for Hyperscan matches
 *
 */
struct id_handler {
	id_handler()
		: ptn_id()
		, from()
		, to() {}

	std::vector<int> ptn_id;
	std::vector<int> from;
	std::vector<int> to;
};

/**
 * @brief Things that hyperscan needs to do its job
 *
 */
struct hyperscan_internal {
	hs_database_t* db;
	hs_scratch_t* scratch;
	std::vector<const char *> rx_str;
	std::vector<unsigned int> ids;

	hyperscan_internal(std::vector<std::unique_ptr<Rule>>& rules)
		: db(nullptr)
		, scratch(nullptr) {

		if(rules.size() == 0) {
			std::cerr << " No rules files loaded" << std::endl;
			abort();
		}

		size_t idx=0;
		for(auto& r :rules) {
			// All regex in this list are anchors that point to this rule
			std::vector<std::string> anc = r->get_anchor_regex();

			for(auto a : anc) {
				char* str = new char[a.length()+1];
				strcpy(str,a.data());
				rx_str.push_back(str);
				ids.push_back(idx);
			}

			idx++;
		}

		std::vector<unsigned int> flags(rx_str.size(), HS_FLAG_DOTALL | HS_FLAG_SINGLEMATCH);

		hs_error_t hs_err;
		hs_compile_error_t* error{};
		if ((hs_err = hs_compile_multi(rx_str.data(), flags.data(), ids.data(),rx_str.size(), HS_MODE_BLOCK, NULL, &db, &error)) != HS_SUCCESS) {
			std::cerr << "Failed to compile hyperscan table" << std::endl;
			abort();
		}

		if (hs_alloc_scratch(db, &scratch) != HS_SUCCESS)
		{
			fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
			hs_free_database(db);
			abort();
		}
	}

	~hyperscan_internal() {
		 hs_free_scratch(scratch);
	    hs_free_database(db);

		for(auto& s : rx_str)
			delete [] s;


	}
};

static int event_handler(unsigned int id, unsigned long long from,
                        unsigned long long to, unsigned int flags, void *ctx)
{
	id_handler* hndl = (id_handler*) ctx;

	hndl->ptn_id.push_back(id);
	hndl->from.push_back(from);
	hndl->to.push_back(to);

    return 0;
}


static bool hyperscan_match(hs_database_t* db, hs_scratch_t* scratch, id_handler& hndl, std::string msg) {

    if (hs_scan(db, msg.c_str(), msg.length(), 0, scratch, event_handler,(void *)&hndl) != HS_SUCCESS)
    {
        fprintf(stderr, "ERROR: Unable to scan input buffer. Exiting.\n");
        hs_free_scratch(scratch);
        hs_free_database(db);
        abort();
    }

	return true;
}



/**
 * @brief parse message with compiled hyperscan table to identify which pattern it should
 * be sent to for filed extraction (if one exists)
 *
 * @param messages
 * @param rules
 * @param db
 * @param scratch
 * @param display
 */

void parse_hyperscan_match(std::vector<std::string>& messages, std::vector<std::unique_ptr<Rule>>& rules,  hyperscan_internal& hs, bool display = true) {
	JSONOut out {};
	for(auto m : messages) {
		id_handler hndl {};
		hyperscan_match(hs.db,hs.scratch,hndl,m);
		size_t size = hndl.ptn_id.size();
		if(size > 0) {
			for(size_t idx=0;idx<size;idx++) {
				auto ridx = hndl.ptn_id.at(idx);
				auto& rule = rules.at(ridx);
				Match msg(m);
				if(rule->match(msg,false))
					out.add_rule_value(*rule);
			}
		}
		// TODO: Add failed message table
	}
	if(display)
		std::cout << out << std::endl;
}

int main(int argc, const char ** argv)
{
    // Setup handling for CTRL-C
	signal(SIGINT, ctrl_handler);

    // Setup argument processing
    Args args(argc,argv);

    // And add an argument to test for
    args.add_string_value("-r","--rules","rules");
	args.add_string_value("-m","--message");
	args.add_string_value("-l","--logdir","logs");
	args.add_string_value("-o","--outformat", "json");
	args.add_string_value("-d","--outir","output");

	args.add_key("-t","--test");
	args.add_key("-s","--show");

	args.add_int_value("-e","--exec",10000);

	if(!args.is_string_present("-r")) {
		std::cout << "A value for \"rules\" directory [-r | --rules] must be supplied" << std::endl;
		return -1;
	}

	if(!args.is_string_present("-m") && !args.is_string_present("-l")) {
		std::cout << "A string escaped value for message [-m | --message] must be supplied" << std::endl;
		std::cout << "A directory for messages [-l | -logdir] must be supplied" << std::endl;
		return -1;
	}


	std::string dir  = args.get_string_value("-r");
	std::string msg  = args.get_string_value("-m");
	std::string logs = args.get_string_value("-l");

	// std::cout << "Log files are at " << logs << std::endl;
	// std::cout << "Rules are at " << dir << std::endl;


	std::vector<std::unique_ptr<Rule>> rules;
	try {
		rules = RuleLoader::load_rules(dir);
	}
	catch(Exception::Generic& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "Failed to read rules from [dir " << dir << "]" << std::endl;
		return -1;
	}

	// Create a table of anchor patterns for hyperscan
	hyperscan_internal hs(rules);


	if(args.is_string_present("-m")) {
		JSONOut out;
		parse_message(rules,msg,out);

		std::cout << out << std::endl;
		return 0;

	}



	std::vector<std::string> messages;
	if(args.is_string_present("-l")) {
		if(!read_logfiles(logs,messages)) {
			std::cerr << "Failed to read messages from [dir " << logs << "]" << std::endl;
			return -1;
		}

	}

	if(args.is_key_present("-t")) {
		bool display = args.is_key_present("-s");
		unsigned int runfor = args.get_int_value("-e");
		std::string run;
		std::cout << "Entering test mode" << std::endl;
		std::cout << "Press CTRL + C to exit" << std::endl;
		std::cout << "Press endter to begin" << std::endl;
		std::getline(std::cin, run);

		std::cout << "==> Runing for " << runfor << " iterations <==" << std::endl;
		for(size_t i=0;i<runfor;i++) {
			if(gExit)
				break;

			parse_hyperscan_match(messages, rules, hs, display);

			// std::cout << "Iteration " << i << std::endl;
		}
 		std::cout << "==> Finshed <==" << std::endl;
	}
	else {
		parse_hyperscan_match(messages, rules, hs, true);
 	}



	return 0;
}

/**
 * @}
 */
