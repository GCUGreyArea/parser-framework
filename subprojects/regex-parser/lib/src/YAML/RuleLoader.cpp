#include "RuleLoader.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "Framework/Rule.h"
#include "Framework/RuleValidator.h"
#include "Exception/Exceptions.h"
#include "YamlFile.h"

namespace RuleLoader {

std::vector<std::unique_ptr<Rule>> load_rules(const std::string& dir)
{
    namespace fs = std::filesystem;

    fs::path root(dir);
    if (!fs::exists(root))
        throw Exception::General("Rules directory does not exist [" + dir + "]");

    if (!fs::is_directory(root))
        throw Exception::General("Rules path is not a directory [" + dir + "]");

    std::vector<std::string> files;
    for (const auto& entry : fs::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() == ".yaml")
            files.push_back(entry.path().string());
    }

    if (files.empty())
        throw Exception::General("No rule files found in directory [" + dir + "]");

    std::sort(files.begin(), files.end());

    std::vector<std::unique_ptr<Rule>> rules;
    for (const auto& file : files)
    {
        YamlFile fl(file);
        try {
            fl.readFile();
        }
        catch (Exception::Generic& e) {
            throw Exception::General("Exception trying to read [file " + file + " / exception " + e.what() + "]");
        }

        rules.push_back(fl.getRule());
    }

    try {
        RuleValidation::validate_examples(rules);
    }
    catch (Exception::Generic& e) {
        throw Exception::General("Exception trying to validate rule examples [exception " + std::string(e.what()) + "]");
    }

    return rules;
}

}
