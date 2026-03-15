#ifndef PARSER_FRAMEWORK_INGESTION_PIPELINE_HPP
#define PARSER_FRAMEWORK_INGESTION_PIPELINE_HPP

#include <optional>
#include <string>
#include <vector>

#include "parser_framework/IngestionBundle.hpp"
#include "parser_framework/ParserFramework.hpp"
#include "parser_framework/ReportAnalyzer.hpp"

namespace parser_framework {

struct CollectionProcessingResult {
    LogCollection collection;
    std::optional<SystemMetadata> system;
    std::optional<NetworkMetadata> network;
    std::optional<SiteMetadata> site;
    std::vector<ParseResult> parsed;
};

struct BundleProcessingResult {
    IngestionBundle bundle;
    std::vector<CollectionProcessingResult> collections;
    std::vector<ReportFinding> reports;
};

class IngestionPipeline {
public:
    explicit IngestionPipeline(ParserFramework framework);
    IngestionPipeline(ParserFramework framework, ReportAnalyzer analyzer);

    BundleProcessingResult process(const IngestionBundle& bundle) const;

private:
    ParserFramework m_framework;
    std::optional<ReportAnalyzer> m_analyzer;
};

std::string render_bundle_processing_result_as_json(const BundleProcessingResult& result);

} // namespace parser_framework

#endif
