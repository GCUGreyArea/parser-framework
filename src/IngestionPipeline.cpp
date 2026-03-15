#include "parser_framework/IngestionPipeline.hpp"

#include <map>
#include <optional>
#include <sstream>
#include <string>

namespace parser_framework {

namespace {

std::string escape_json_string(const std::string& value) {
    std::ostringstream stream;
    for (const char ch : value) {
        switch (ch) {
        case '\\':
            stream << "\\\\";
            break;
        case '"':
            stream << "\\\"";
            break;
        case '\b':
            stream << "\\b";
            break;
        case '\f':
            stream << "\\f";
            break;
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        default:
            stream << ch;
            break;
        }
    }

    return stream.str();
}

void append_indent(std::ostringstream& stream, std::size_t depth) {
    for (std::size_t idx = 0; idx < depth; ++idx) {
        stream << "  ";
    }
}

std::string indent_embedded_json(const std::string& json, std::size_t depth) {
    std::ostringstream stream;
    bool first_line = true;
    std::size_t start = 0;

    while (start <= json.size()) {
        const std::size_t end = json.find('\n', start);
        const std::string line = json.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (!first_line) {
            append_indent(stream, depth);
        }
        stream << line;
        if (end == std::string::npos) {
            break;
        }
        stream << "\n";
        first_line = false;
        start = end + 1;
    }

    return stream.str();
}

void render_string_array(std::ostringstream& stream, const std::vector<std::string>& values) {
    stream << "[";
    for (std::size_t idx = 0; idx < values.size(); ++idx) {
        if (idx != 0) {
            stream << ", ";
        }
        stream << "\"" << escape_json_string(values[idx]) << "\"";
    }
    stream << "]";
}

void render_attribution(std::ostringstream& stream, const Attribution& attribution, std::size_t depth) {
    append_indent(stream, depth);
    stream << "{\n";
    append_indent(stream, depth + 1);
    stream << "\"owner_org_ids\": ";
    render_string_array(stream, attribution.owner_org_ids);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"operator_org_ids\": ";
    render_string_array(stream, attribution.operator_org_ids);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"tenant_org_ids\": ";
    render_string_array(stream, attribution.tenant_org_ids);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"provider_org_ids\": ";
    render_string_array(stream, attribution.provider_org_ids);
    stream << "\n";
    append_indent(stream, depth);
    stream << "}";
}

template <typename Entry>
const Entry* find_by_id(const std::vector<Entry>& entries, const std::string& id) {
    for (const auto& entry : entries) {
        if (entry.id == id) {
            return &entry;
        }
    }

    return nullptr;
}

void render_collection_context(std::ostringstream& stream,
    const CollectionProcessingResult& collection_result,
    std::size_t depth) {
    append_indent(stream, depth);
    stream << "\"collection\": {\n";
    append_indent(stream, depth + 1);
    stream << "\"id\": \"" << escape_json_string(collection_result.collection.id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"name\": \"" << escape_json_string(collection_result.collection.name) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"system_id\": \"" << escape_json_string(collection_result.collection.system_id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"network_id\": \"" << escape_json_string(collection_result.collection.network_id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"site_id\": \"" << escape_json_string(collection_result.collection.site_id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"log_type\": \"" << escape_json_string(collection_result.collection.log_type) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"format\": \"" << escape_json_string(collection_result.collection.format) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"classification\": \"" << escape_json_string(collection_result.collection.classification) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"observed_start\": \"" << escape_json_string(collection_result.collection.observed_start) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"observed_end\": \"" << escape_json_string(collection_result.collection.observed_end) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"record_count\": \"" << collection_result.collection.records.size() << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"attribution\": ";
    render_attribution(stream, collection_result.collection.attribution, depth + 1);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"tags\": ";
    render_string_array(stream, collection_result.collection.tags);
    stream << "\n";
    append_indent(stream, depth);
    stream << "}";
}

void render_entry_header(std::ostringstream& stream,
    const std::string& name,
    std::size_t depth) {
    append_indent(stream, depth);
    stream << "\"" << name << "\": {\n";
}

void render_system(std::ostringstream& stream, const SystemMetadata& system, std::size_t depth) {
    render_entry_header(stream, "system", depth);
    append_indent(stream, depth + 1);
    stream << "\"id\": \"" << escape_json_string(system.id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"name\": \"" << escape_json_string(system.name) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"kind\": \"" << escape_json_string(system.kind) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"vendor\": \"" << escape_json_string(system.vendor) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"product\": \"" << escape_json_string(system.product) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"deployment_model\": \"" << escape_json_string(system.deployment_model) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"network_id\": \"" << escape_json_string(system.network_id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"site_id\": \"" << escape_json_string(system.site_id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"attribution\": ";
    render_attribution(stream, system.attribution, depth + 1);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"tags\": ";
    render_string_array(stream, system.tags);
    stream << "\n";
    append_indent(stream, depth);
    stream << "}";
}

void render_network(std::ostringstream& stream, const NetworkMetadata& network, std::size_t depth) {
    render_entry_header(stream, "network", depth);
    append_indent(stream, depth + 1);
    stream << "\"id\": \"" << escape_json_string(network.id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"name\": \"" << escape_json_string(network.name) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"kind\": \"" << escape_json_string(network.kind) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"environment\": \"" << escape_json_string(network.environment) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"site_id\": \"" << escape_json_string(network.site_id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"attribution\": ";
    render_attribution(stream, network.attribution, depth + 1);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"tags\": ";
    render_string_array(stream, network.tags);
    stream << "\n";
    append_indent(stream, depth);
    stream << "}";
}

void render_site(std::ostringstream& stream, const SiteMetadata& site, std::size_t depth) {
    render_entry_header(stream, "site", depth);
    append_indent(stream, depth + 1);
    stream << "\"id\": \"" << escape_json_string(site.id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"name\": \"" << escape_json_string(site.name) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"country_code\": \"" << escape_json_string(site.country_code) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"region\": \"" << escape_json_string(site.region) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"city\": \"" << escape_json_string(site.city) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"timezone\": \"" << escape_json_string(site.timezone) << "\"\n";
    append_indent(stream, depth);
    stream << "}";
}

} // namespace

IngestionPipeline::IngestionPipeline(ParserFramework framework)
    : m_framework(std::move(framework)) {}

IngestionPipeline::IngestionPipeline(ParserFramework framework, ReportAnalyzer analyzer)
    : m_framework(std::move(framework)),
      m_analyzer(std::move(analyzer)) {}

BundleProcessingResult IngestionPipeline::process(const IngestionBundle& bundle) const {
    BundleProcessingResult result;
    result.bundle = bundle;

    std::vector<ParseResult> all_parsed;

    for (const auto& collection : bundle.collections) {
        CollectionProcessingResult collection_result;
        collection_result.collection = collection;
        if (const SystemMetadata* system = find_by_id(bundle.systems, collection.system_id)) {
            collection_result.system = *system;
        }
        if (const NetworkMetadata* network = find_by_id(bundle.networks, collection.network_id)) {
            collection_result.network = *network;
        }
        if (const SiteMetadata* site = find_by_id(bundle.sites, collection.site_id)) {
            collection_result.site = *site;
        }

        collection_result.parsed = m_framework.parse_messages(collection.records);
        all_parsed.insert(all_parsed.end(), collection_result.parsed.begin(), collection_result.parsed.end());
        result.collections.push_back(std::move(collection_result));
    }

    if (m_analyzer.has_value()) {
        result.reports = m_analyzer->analyze(all_parsed);
    }

    return result;
}

std::string render_bundle_processing_result_as_json(const BundleProcessingResult& result) {
    std::ostringstream stream;
    stream << "{\n";
    append_indent(stream, 1);
    stream << "\"bundle\": ";
    stream << indent_embedded_json(render_bundle_as_json(result.bundle), 1);
    stream << ",\n";

    append_indent(stream, 1);
    stream << "\"collections\": [\n";
    for (std::size_t idx = 0; idx < result.collections.size(); ++idx) {
        const CollectionProcessingResult& collection_result = result.collections[idx];
        append_indent(stream, 2);
        stream << "{\n";
        render_collection_context(stream, collection_result, 3);
        if (collection_result.system.has_value()) {
            stream << ",\n";
            render_system(stream, *collection_result.system, 3);
        }
        if (collection_result.network.has_value()) {
            stream << ",\n";
            render_network(stream, *collection_result.network, 3);
        }
        if (collection_result.site.has_value()) {
            stream << ",\n";
            render_site(stream, *collection_result.site, 3);
        }
        stream << ",\n";
        append_indent(stream, 3);
        stream << "\"parser_output\": ";
        stream << indent_embedded_json(render_results_as_json(collection_result.parsed), 3) << "\n";
        append_indent(stream, 2);
        stream << "}";
        if (idx + 1 != result.collections.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    append_indent(stream, 1);
    stream << "],\n";

    append_indent(stream, 1);
    stream << "\"report_output\": ";
    stream << indent_embedded_json(render_reports_as_json(result.reports), 1) << "\n";
    stream << "}";
    return stream.str();
}

} // namespace parser_framework
