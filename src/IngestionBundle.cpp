#include "parser_framework/IngestionBundle.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <yaml-cpp/yaml.h>

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

YAML::Node require_node(const YAML::Node& node, const std::string& field) {
    const YAML::Node child = node[field];
    if (!child) {
        throw std::runtime_error("Missing required ingestion field [" + field + "]");
    }

    return child;
}

std::string require_string(const YAML::Node& node, const std::string& field) {
    return require_node(node, field).as<std::string>();
}

std::string optional_string(const YAML::Node& node, const std::string& field) {
    const YAML::Node child = node[field];
    if (!child) {
        return "";
    }

    return child.as<std::string>();
}

std::vector<std::string> parse_string_list(const YAML::Node& node, const std::string& field) {
    std::vector<std::string> values;
    const YAML::Node child = node[field];
    if (!child) {
        return values;
    }
    if (!child.IsSequence()) {
        throw std::runtime_error("Expected ingestion field [" + field + "] to be a sequence");
    }

    for (const auto& entry : child) {
        values.push_back(entry.as<std::string>());
    }

    return values;
}

Attribution parse_attribution(const YAML::Node& node) {
    const YAML::Node attribution = node["attribution"];
    if (!attribution) {
        return {};
    }

    Attribution parsed;
    parsed.owner_org_ids = parse_string_list(attribution, "owner_org_ids");
    parsed.operator_org_ids = parse_string_list(attribution, "operator_org_ids");
    parsed.tenant_org_ids = parse_string_list(attribution, "tenant_org_ids");
    parsed.provider_org_ids = parse_string_list(attribution, "provider_org_ids");
    return parsed;
}

template <typename Entry>
void ensure_unique_ids(const std::vector<Entry>& entries, const std::string& label) {
    std::set<std::string> seen;
    for (const auto& entry : entries) {
        if (!seen.insert(entry.id).second) {
            throw std::runtime_error("Duplicate " + label + " id [" + entry.id + "]");
        }
    }
}

template <typename Entry>
std::set<std::string> collect_ids(const std::vector<Entry>& entries) {
    std::set<std::string> ids;
    for (const auto& entry : entries) {
        ids.insert(entry.id);
    }

    return ids;
}

void validate_reference(const std::set<std::string>& ids,
    const std::string& value,
    const std::string& field) {
    if (value.empty()) {
        return;
    }
    if (ids.find(value) == ids.end()) {
        throw std::runtime_error("Unknown reference [" + value + "] in field [" + field + "]");
    }
}

void validate_attribution(const Attribution& attribution,
    const std::set<std::string>& organisation_ids,
    const std::string& context) {
    const std::vector<std::pair<std::string, const std::vector<std::string>*>> fields = {
        {"owner_org_ids", &attribution.owner_org_ids},
        {"operator_org_ids", &attribution.operator_org_ids},
        {"tenant_org_ids", &attribution.tenant_org_ids},
        {"provider_org_ids", &attribution.provider_org_ids}
    };

    for (const auto& field : fields) {
        for (const auto& value : *field.second) {
            if (organisation_ids.find(value) == organisation_ids.end()) {
                throw std::runtime_error(
                    "Unknown organisation reference [" + value + "] in " + context + "." + field.first);
            }
        }
    }
}

void render_string_array(std::ostringstream& stream,
    const std::vector<std::string>& values,
    std::size_t depth) {
    stream << "[";
    for (std::size_t idx = 0; idx < values.size(); ++idx) {
        if (idx != 0) {
            stream << ", ";
        }
        stream << "\"" << escape_json_string(values[idx]) << "\"";
    }
    stream << "]";
    if (depth == 0) {
        stream << "\n";
    }
}

void render_attribution(std::ostringstream& stream, const Attribution& attribution, std::size_t depth) {
    append_indent(stream, depth);
    stream << "{\n";
    append_indent(stream, depth + 1);
    stream << "\"owner_org_ids\": ";
    render_string_array(stream, attribution.owner_org_ids, depth + 1);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"operator_org_ids\": ";
    render_string_array(stream, attribution.operator_org_ids, depth + 1);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"tenant_org_ids\": ";
    render_string_array(stream, attribution.tenant_org_ids, depth + 1);
    stream << ",\n";
    append_indent(stream, depth + 1);
    stream << "\"provider_org_ids\": ";
    render_string_array(stream, attribution.provider_org_ids, depth + 1);
    stream << "\n";
    append_indent(stream, depth);
    stream << "}";
}

void render_tags(std::ostringstream& stream, const std::vector<std::string>& tags, std::size_t depth) {
    append_indent(stream, depth);
    stream << "\"tags\": ";
    render_string_array(stream, tags, depth);
}

} // namespace

namespace IngestionLoader {

IngestionBundle load_bundle(const std::string& path) {
    const YAML::Node root = YAML::LoadFile(path);

    IngestionBundle bundle;
    bundle.schema_version = require_string(root, "schema_version");
    bundle.bundle_id = require_string(root, "bundle_id");
    bundle.produced_at = require_string(root, "produced_at");

    const YAML::Node producer = require_node(root, "producer");
    bundle.producer.component_id = require_string(producer, "component_id");
    bundle.producer.component_type = require_string(producer, "component_type");
    bundle.producer.organisation_id = optional_string(producer, "organisation_id");
    bundle.producer.region = optional_string(producer, "region");

    const YAML::Node storage = require_node(root, "storage");
    bundle.storage.backend = require_string(storage, "backend");
    bundle.storage.database = require_string(storage, "database");
    bundle.storage.bundle_collection = require_string(storage, "bundle_collection");
    bundle.storage.result_collection = require_string(storage, "result_collection");

    const YAML::Node organisations = require_node(root, "organizations");
    if (!organisations.IsSequence()) {
        throw std::runtime_error("Expected [organizations] to be a sequence");
    }
    for (const auto& organization : organisations) {
        OrganizationMetadata parsed;
        parsed.id = require_string(organization, "id");
        parsed.name = require_string(organization, "name");
        parsed.kind = require_string(organization, "kind");
        parsed.tags = parse_string_list(organization, "tags");
        bundle.organizations.push_back(std::move(parsed));
    }

    const YAML::Node sites = require_node(root, "sites");
    if (!sites.IsSequence()) {
        throw std::runtime_error("Expected [sites] to be a sequence");
    }
    for (const auto& site : sites) {
        SiteMetadata parsed;
        parsed.id = require_string(site, "id");
        parsed.name = require_string(site, "name");
        parsed.country_code = require_string(site, "country_code");
        parsed.region = require_string(site, "region");
        parsed.city = optional_string(site, "city");
        parsed.timezone = optional_string(site, "timezone");
        bundle.sites.push_back(std::move(parsed));
    }

    const YAML::Node networks = require_node(root, "networks");
    if (!networks.IsSequence()) {
        throw std::runtime_error("Expected [networks] to be a sequence");
    }
    for (const auto& network : networks) {
        NetworkMetadata parsed;
        parsed.id = require_string(network, "id");
        parsed.name = require_string(network, "name");
        parsed.kind = require_string(network, "kind");
        parsed.environment = require_string(network, "environment");
        parsed.site_id = require_string(network, "site_id");
        parsed.attribution = parse_attribution(network);
        parsed.tags = parse_string_list(network, "tags");
        bundle.networks.push_back(std::move(parsed));
    }

    const YAML::Node systems = require_node(root, "systems");
    if (!systems.IsSequence()) {
        throw std::runtime_error("Expected [systems] to be a sequence");
    }
    for (const auto& system : systems) {
        SystemMetadata parsed;
        parsed.id = require_string(system, "id");
        parsed.name = require_string(system, "name");
        parsed.kind = require_string(system, "kind");
        parsed.vendor = require_string(system, "vendor");
        parsed.product = require_string(system, "product");
        parsed.deployment_model = require_string(system, "deployment_model");
        parsed.network_id = require_string(system, "network_id");
        parsed.site_id = require_string(system, "site_id");
        parsed.attribution = parse_attribution(system);
        parsed.tags = parse_string_list(system, "tags");
        bundle.systems.push_back(std::move(parsed));
    }

    const YAML::Node collections = require_node(root, "collections");
    if (!collections.IsSequence()) {
        throw std::runtime_error("Expected [collections] to be a sequence");
    }
    for (const auto& collection : collections) {
        LogCollection parsed;
        parsed.id = require_string(collection, "id");
        parsed.name = require_string(collection, "name");
        parsed.system_id = require_string(collection, "system_id");
        parsed.network_id = require_string(collection, "network_id");
        parsed.site_id = require_string(collection, "site_id");
        parsed.log_type = require_string(collection, "log_type");
        parsed.format = require_string(collection, "format");
        parsed.classification = require_string(collection, "classification");
        parsed.observed_start = require_string(collection, "observed_start");
        parsed.observed_end = require_string(collection, "observed_end");
        parsed.attribution = parse_attribution(collection);
        parsed.tags = parse_string_list(collection, "tags");
        parsed.records = parse_string_list(collection, "records");
        if (parsed.records.empty()) {
            throw std::runtime_error("Log collection [" + parsed.id + "] must include at least one record");
        }
        bundle.collections.push_back(std::move(parsed));
    }

    ensure_unique_ids(bundle.organizations, "organization");
    ensure_unique_ids(bundle.sites, "site");
    ensure_unique_ids(bundle.networks, "network");
    ensure_unique_ids(bundle.systems, "system");
    ensure_unique_ids(bundle.collections, "collection");

    const std::set<std::string> organisation_ids = collect_ids(bundle.organizations);
    const std::set<std::string> site_ids = collect_ids(bundle.sites);
    const std::set<std::string> network_ids = collect_ids(bundle.networks);
    const std::set<std::string> system_ids = collect_ids(bundle.systems);

    if (!bundle.producer.organisation_id.empty() &&
        organisation_ids.find(bundle.producer.organisation_id) == organisation_ids.end()) {
        throw std::runtime_error(
            "Unknown organisation reference [" + bundle.producer.organisation_id + "] in producer.organisation_id");
    }

    for (const auto& network : bundle.networks) {
        validate_reference(site_ids, network.site_id, "networks.site_id");
        validate_attribution(network.attribution, organisation_ids, "networks[" + network.id + "]");
    }

    for (const auto& system : bundle.systems) {
        validate_reference(network_ids, system.network_id, "systems.network_id");
        validate_reference(site_ids, system.site_id, "systems.site_id");
        validate_attribution(system.attribution, organisation_ids, "systems[" + system.id + "]");
    }

    for (const auto& collection : bundle.collections) {
        validate_reference(system_ids, collection.system_id, "collections.system_id");
        validate_reference(network_ids, collection.network_id, "collections.network_id");
        validate_reference(site_ids, collection.site_id, "collections.site_id");
        validate_attribution(collection.attribution, organisation_ids, "collections[" + collection.id + "]");

        const auto system_it = std::find_if(
            bundle.systems.begin(),
            bundle.systems.end(),
            [&](const SystemMetadata& system) { return system.id == collection.system_id; });
        if (system_it != bundle.systems.end()) {
            if (system_it->network_id != collection.network_id) {
                throw std::runtime_error(
                    "Collection [" + collection.id + "] network_id does not match parent system network_id");
            }
            if (system_it->site_id != collection.site_id) {
                throw std::runtime_error(
                    "Collection [" + collection.id + "] site_id does not match parent system site_id");
            }
        }
    }

    return bundle;
}

} // namespace IngestionLoader

std::string render_bundle_as_json(const IngestionBundle& bundle) {
    std::ostringstream stream;
    stream << "{\n";
    append_indent(stream, 1);
    stream << "\"schema_version\": \"" << escape_json_string(bundle.schema_version) << "\",\n";
    append_indent(stream, 1);
    stream << "\"bundle_id\": \"" << escape_json_string(bundle.bundle_id) << "\",\n";
    append_indent(stream, 1);
    stream << "\"produced_at\": \"" << escape_json_string(bundle.produced_at) << "\",\n";

    append_indent(stream, 1);
    stream << "\"producer\": {\n";
    append_indent(stream, 2);
    stream << "\"component_id\": \"" << escape_json_string(bundle.producer.component_id) << "\",\n";
    append_indent(stream, 2);
    stream << "\"component_type\": \"" << escape_json_string(bundle.producer.component_type) << "\",\n";
    append_indent(stream, 2);
    stream << "\"organisation_id\": \"" << escape_json_string(bundle.producer.organisation_id) << "\",\n";
    append_indent(stream, 2);
    stream << "\"region\": \"" << escape_json_string(bundle.producer.region) << "\"\n";
    append_indent(stream, 1);
    stream << "},\n";

    append_indent(stream, 1);
    stream << "\"storage\": {\n";
    append_indent(stream, 2);
    stream << "\"backend\": \"" << escape_json_string(bundle.storage.backend) << "\",\n";
    append_indent(stream, 2);
    stream << "\"database\": \"" << escape_json_string(bundle.storage.database) << "\",\n";
    append_indent(stream, 2);
    stream << "\"bundle_collection\": \"" << escape_json_string(bundle.storage.bundle_collection) << "\",\n";
    append_indent(stream, 2);
    stream << "\"result_collection\": \"" << escape_json_string(bundle.storage.result_collection) << "\"\n";
    append_indent(stream, 1);
    stream << "},\n";

    append_indent(stream, 1);
    stream << "\"organizations\": [\n";
    for (std::size_t idx = 0; idx < bundle.organizations.size(); ++idx) {
        const OrganizationMetadata& organization = bundle.organizations[idx];
        append_indent(stream, 2);
        stream << "{\n";
        append_indent(stream, 3);
        stream << "\"id\": \"" << escape_json_string(organization.id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"name\": \"" << escape_json_string(organization.name) << "\",\n";
        append_indent(stream, 3);
        stream << "\"kind\": \"" << escape_json_string(organization.kind) << "\",\n";
        render_tags(stream, organization.tags, 3);
        stream << "\n";
        append_indent(stream, 2);
        stream << "}";
        if (idx + 1 != bundle.organizations.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    append_indent(stream, 1);
    stream << "],\n";

    append_indent(stream, 1);
    stream << "\"sites\": [\n";
    for (std::size_t idx = 0; idx < bundle.sites.size(); ++idx) {
        const SiteMetadata& site = bundle.sites[idx];
        append_indent(stream, 2);
        stream << "{\n";
        append_indent(stream, 3);
        stream << "\"id\": \"" << escape_json_string(site.id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"name\": \"" << escape_json_string(site.name) << "\",\n";
        append_indent(stream, 3);
        stream << "\"country_code\": \"" << escape_json_string(site.country_code) << "\",\n";
        append_indent(stream, 3);
        stream << "\"region\": \"" << escape_json_string(site.region) << "\",\n";
        append_indent(stream, 3);
        stream << "\"city\": \"" << escape_json_string(site.city) << "\",\n";
        append_indent(stream, 3);
        stream << "\"timezone\": \"" << escape_json_string(site.timezone) << "\"\n";
        append_indent(stream, 2);
        stream << "}";
        if (idx + 1 != bundle.sites.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    append_indent(stream, 1);
    stream << "],\n";

    append_indent(stream, 1);
    stream << "\"networks\": [\n";
    for (std::size_t idx = 0; idx < bundle.networks.size(); ++idx) {
        const NetworkMetadata& network = bundle.networks[idx];
        append_indent(stream, 2);
        stream << "{\n";
        append_indent(stream, 3);
        stream << "\"id\": \"" << escape_json_string(network.id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"name\": \"" << escape_json_string(network.name) << "\",\n";
        append_indent(stream, 3);
        stream << "\"kind\": \"" << escape_json_string(network.kind) << "\",\n";
        append_indent(stream, 3);
        stream << "\"environment\": \"" << escape_json_string(network.environment) << "\",\n";
        append_indent(stream, 3);
        stream << "\"site_id\": \"" << escape_json_string(network.site_id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"attribution\": ";
        render_attribution(stream, network.attribution, 3);
        stream << ",\n";
        render_tags(stream, network.tags, 3);
        stream << "\n";
        append_indent(stream, 2);
        stream << "}";
        if (idx + 1 != bundle.networks.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    append_indent(stream, 1);
    stream << "],\n";

    append_indent(stream, 1);
    stream << "\"systems\": [\n";
    for (std::size_t idx = 0; idx < bundle.systems.size(); ++idx) {
        const SystemMetadata& system = bundle.systems[idx];
        append_indent(stream, 2);
        stream << "{\n";
        append_indent(stream, 3);
        stream << "\"id\": \"" << escape_json_string(system.id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"name\": \"" << escape_json_string(system.name) << "\",\n";
        append_indent(stream, 3);
        stream << "\"kind\": \"" << escape_json_string(system.kind) << "\",\n";
        append_indent(stream, 3);
        stream << "\"vendor\": \"" << escape_json_string(system.vendor) << "\",\n";
        append_indent(stream, 3);
        stream << "\"product\": \"" << escape_json_string(system.product) << "\",\n";
        append_indent(stream, 3);
        stream << "\"deployment_model\": \"" << escape_json_string(system.deployment_model) << "\",\n";
        append_indent(stream, 3);
        stream << "\"network_id\": \"" << escape_json_string(system.network_id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"site_id\": \"" << escape_json_string(system.site_id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"attribution\": ";
        render_attribution(stream, system.attribution, 3);
        stream << ",\n";
        render_tags(stream, system.tags, 3);
        stream << "\n";
        append_indent(stream, 2);
        stream << "}";
        if (idx + 1 != bundle.systems.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    append_indent(stream, 1);
    stream << "],\n";

    append_indent(stream, 1);
    stream << "\"collections\": [\n";
    for (std::size_t idx = 0; idx < bundle.collections.size(); ++idx) {
        const LogCollection& collection = bundle.collections[idx];
        append_indent(stream, 2);
        stream << "{\n";
        append_indent(stream, 3);
        stream << "\"id\": \"" << escape_json_string(collection.id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"name\": \"" << escape_json_string(collection.name) << "\",\n";
        append_indent(stream, 3);
        stream << "\"system_id\": \"" << escape_json_string(collection.system_id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"network_id\": \"" << escape_json_string(collection.network_id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"site_id\": \"" << escape_json_string(collection.site_id) << "\",\n";
        append_indent(stream, 3);
        stream << "\"log_type\": \"" << escape_json_string(collection.log_type) << "\",\n";
        append_indent(stream, 3);
        stream << "\"format\": \"" << escape_json_string(collection.format) << "\",\n";
        append_indent(stream, 3);
        stream << "\"classification\": \"" << escape_json_string(collection.classification) << "\",\n";
        append_indent(stream, 3);
        stream << "\"observed_start\": \"" << escape_json_string(collection.observed_start) << "\",\n";
        append_indent(stream, 3);
        stream << "\"observed_end\": \"" << escape_json_string(collection.observed_end) << "\",\n";
        append_indent(stream, 3);
        stream << "\"attribution\": ";
        render_attribution(stream, collection.attribution, 3);
        stream << ",\n";
        render_tags(stream, collection.tags, 3);
        stream << ",\n";
        append_indent(stream, 3);
        stream << "\"records\": ";
        render_string_array(stream, collection.records, 3);
        stream << "\n";
        append_indent(stream, 2);
        stream << "}";
        if (idx + 1 != bundle.collections.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    append_indent(stream, 1);
    stream << "]\n";
    stream << "}";
    return stream.str();
}

} // namespace parser_framework
