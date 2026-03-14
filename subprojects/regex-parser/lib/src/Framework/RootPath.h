#ifndef ROOT_PATH_HEADER_H
#define ROOT_PATH_HEADER_H

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <iostream>


class RootPath {
public:
    RootPath(std::vector<std::vector<size_t>>& patterns);
    RootPath(RootPath& r);
    void print_paths();

    std::vector<size_t> path_has_pattern(std::vector<size_t> path);
    std::vector<size_t> path_has_tokens(std::vector<size_t> path);
    std::vector<size_t> get_unique_tokens() {return mUnique;}

protected:
    std::vector<size_t> get_unique_tokens(std::vector<std::vector<size_t>>& patterns);
    // void build_from_root_path(size_t token, std::vector<size_t>& unique, std::vector<std::vector<size_t>>& patterns);
    std::string genereate_path_string(std::vector<size_t>& path);
    void build_unique_token_list(std::vector<size_t>& path,std::vector<std::vector<size_t>>& patterns, std::vector<size_t>& tokens);
    bool is_sub_path(std::vector<size_t>& sub, std::vector<size_t>& path);
    std::vector<std::vector<size_t>> get_all_subsets(std::vector<size_t>& set);
    std::vector<std::vector<size_t>> generate_all_paths(std::vector<std::vector<size_t>>& patterns);

private:
    static void print_vector(std::string name, std::vector<size_t> vect) {
            std::cout << name << " [ ";
            for(auto& i : vect)
                std::cout << i << " ";
            std::cout << "]" << std::endl;
    }

    static void has_token(size_t token, std::vector<std::vector<size_t>>& patterns, std::vector<size_t>& ptns);

    static void path_in_patterns(std::vector<size_t>& path, std::vector<std::vector<size_t>>& patterns,std::vector<size_t>& ptns);

    std::unordered_map<std::string,std::vector<size_t>> mTokenMap;
    std::unordered_map<std::string,std::vector<size_t>> mPatternMap;
    std::vector<std::vector<size_t>> mPaths;
    std::vector<size_t> mUnique;
};

#endif//ROOT_PATH_HEADER_H