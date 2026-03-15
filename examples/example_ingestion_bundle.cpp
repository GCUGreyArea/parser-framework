#include <iostream>
#include <string>

#include "parser_framework/IngestionBundle.hpp"
#include "parser_framework/IngestionPipeline.hpp"
#include "parser_framework/ParserFramework.hpp"
#include "parser_framework/ReportAnalyzer.hpp"
#include "parser_framework/RuleLoader.hpp"
#include "parser_framework/utils/Args.hpp"

using namespace parser_framework;

extern "C" const char* __asan_default_options() {
    return "detect_leaks=0";
}

extern "C" const char* __lsan_default_options() {
    return "detect_leaks=0";
}

namespace {

void print_usage() {
    std::cout << "Usage: example_ingestion_bundle "
                 "[-b|--bundle <path>] [-r|--rules <path>] [-p|--report-rules <path>]\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        utils::Args args(argc, argv);
        args.add_string_value("-b", "--bundle", "bundles/multi-tenant-ingestion.json");
        args.add_string_value("-r", "--rules", "rules");
        args.add_string_value("-p", "--report-rules", "report_rules");
        args.add_key("-h", "--help");

        if (args.is_key_present("-h")) {
            print_usage();
            return 0;
        }

        const IngestionBundle bundle = IngestionLoader::load_bundle(args.get_string_value("-b"));
        ParserFramework framework(RuleLoader::load_rules(args.get_string_value("-r")));
        ReportAnalyzer analyzer(ReportRuleLoader::load_rules(args.get_string_value("-p")));
        IngestionPipeline pipeline(std::move(framework), std::move(analyzer));

        const BundleProcessingResult result = pipeline.process(bundle);
        std::cout << render_bundle_processing_result_as_json(result) << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }
}
