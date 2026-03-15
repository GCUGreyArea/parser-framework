#ifndef PARSER_FRAMEWORK_INGESTION_BUNDLE_HPP
#define PARSER_FRAMEWORK_INGESTION_BUNDLE_HPP

#include <string>
#include <vector>

namespace parser_framework {

struct Attribution {
    std::vector<std::string> owner_org_ids;
    std::vector<std::string> operator_org_ids;
    std::vector<std::string> tenant_org_ids;
    std::vector<std::string> provider_org_ids;
};

struct ProducerMetadata {
    std::string component_id;
    std::string component_type;
    std::string organisation_id;
    std::string region;
};

struct StorageHint {
    std::string backend;
    std::string database;
    std::string bundle_collection;
    std::string result_collection;
};

struct OrganizationMetadata {
    std::string id;
    std::string name;
    std::string kind;
    std::vector<std::string> tags;
};

struct SiteMetadata {
    std::string id;
    std::string name;
    std::string country_code;
    std::string region;
    std::string city;
    std::string timezone;
};

struct NetworkMetadata {
    std::string id;
    std::string name;
    std::string kind;
    std::string environment;
    std::string site_id;
    Attribution attribution;
    std::vector<std::string> tags;
};

struct SystemMetadata {
    std::string id;
    std::string name;
    std::string kind;
    std::string vendor;
    std::string product;
    std::string deployment_model;
    std::string network_id;
    std::string site_id;
    Attribution attribution;
    std::vector<std::string> tags;
};

struct LogCollection {
    std::string id;
    std::string name;
    std::string system_id;
    std::string network_id;
    std::string site_id;
    std::string log_type;
    std::string format;
    std::string classification;
    std::string observed_start;
    std::string observed_end;
    Attribution attribution;
    std::vector<std::string> tags;
    std::vector<std::string> records;
};

struct IngestionBundle {
    std::string schema_version;
    std::string bundle_id;
    std::string produced_at;
    ProducerMetadata producer;
    StorageHint storage;
    std::vector<OrganizationMetadata> organizations;
    std::vector<SiteMetadata> sites;
    std::vector<NetworkMetadata> networks;
    std::vector<SystemMetadata> systems;
    std::vector<LogCollection> collections;
};

namespace IngestionLoader {

IngestionBundle load_bundle(const std::string& path);

} // namespace IngestionLoader

std::string render_bundle_as_json(const IngestionBundle& bundle);

} // namespace parser_framework

#endif
