#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include "RootPath.h"


/**
 * @brief Construct a new RootPathobject so that all paths are represented as
 * 0 -> [all token indicies for all patterns that contain 0]
 * 1 -> [all token indicies for patterns that contain 1]
 * etc...
 * @param patterns
 */
RootPath::RootPath(std::vector<std::vector<size_t>>& patterns) {
    for(auto& p : patterns) {
        std::sort( p.begin( ), p.end( ), [](size_t& lhs, size_t& rhs) {
            if(lhs > rhs)
                return false;
            return true;
        });
    }

    // Get all unique tokens from all viable paths
    mUnique = get_unique_tokens(patterns);

    // Generate all pathes to a token match
    mPaths = generate_all_paths(patterns);

    // Create keys for tokens and patterns for each path
    for(auto path : mPaths) {
        if(path.size() > 0) {
            std::vector<size_t> ptns;
            path_in_patterns(path,patterns,ptns);

            if(ptns.size() > 0) {

                // Get all tokens
                std::vector<size_t> temp;
                for(auto p : ptns) {
                    auto pattern = patterns[p];
                    for(auto pt : pattern) {
                        if(std::find(path.begin(),path.end(),pt) == path.end()) {
                            temp.push_back(pt);
                        }
                    }
                }

                // Remove duplicates
                 std::sort( temp.begin(), temp.end(), [](size_t& lhs, size_t& rhs) {
                    if(lhs > rhs)
                        return false;
                    return true;
                });

                std::vector<size_t> tokens;
                size_t last = -1;
                for(auto t : temp) {
                    if(t == last) {
                        continue;
                    }

                    tokens.push_back(t);
                }

                std::string key = genereate_path_string(path);
                mTokenMap.emplace(key,tokens);
                mPatternMap.emplace(key,ptns);
            }
        }
    }

    // print_paths();
}

RootPath::RootPath(RootPath& r)
    : mTokenMap(r.mTokenMap)
    , mPatternMap(r.mPatternMap)
    , mPaths(r.mPaths)
    , mUnique(r.mUnique) {}

/**
 * @brief Get a list of unique tokens from all active patterns
 *
 * @param patterns
 * @param tokens
 * @return size_t
 */
std::vector<size_t> RootPath::get_unique_tokens(std::vector<std::vector<size_t>>& patterns) {
    size_t highest = 0;
    std::vector<size_t> tokens;
    std::unordered_map<size_t,bool> map;

    for(auto& ptn : patterns)
        for(auto& i : ptn)
            if(map.find(i) == map.end())
                map.emplace(i,true);


    for(auto i : map) {
        if(i.first > highest)
            highest = i.first;
        tokens.push_back(i.first);
    }

    return tokens;
}

std::string RootPath::genereate_path_string(std::vector<size_t>& path) {
    std::string p;

    for(auto& i : path) {
        p += "." + std::to_string(i);
    }

    return p;
}

void RootPath::path_in_patterns(std::vector<size_t>& path, std::vector<std::vector<size_t>>& patterns,std::vector<size_t>& ptns) {
    // print_vector("testing path", path);
    size_t n_patterns = patterns.size();
    size_t size = path.size();
    for(size_t i=0;i<n_patterns;i++) {
        auto& ptn = patterns[i];

        // print_vector("testing pattern", ptn);
        size_t used = 0;
        for(auto& p : ptn) {
            for(auto& t : path) {
                if(p == t)
                    used++;

                if(used == size)
                    break;

            }

            if(used == size) {
                ptns.push_back(i);
                used = 0;
            }
        }
    }

}

void RootPath::has_token(size_t token, std::vector<std::vector<size_t>>& patterns, std::vector<size_t>& ptns) {
    std::vector path {token};
    path_in_patterns(path,patterns,ptns);
}


/**
 * @brief Build a vector of tokens for this path that only exists in the path
 *
 * @param path
 * @param patterns
 */
void RootPath::build_unique_token_list(std::vector<size_t>& path,std::vector<std::vector<size_t>>& patterns, std::vector<size_t>& tokens) {
    std::vector<size_t> validPtn;

    path_in_patterns(path,patterns,validPtn);

    std::unordered_map<size_t,size_t> map;
    for(auto& ptn : validPtn) {
        auto& pattern = patterns.at(ptn);
        for(auto& i : pattern) {
            if(map.find(i) == map.end()) {
                map.emplace(i,i);
                if(std::find(path.begin(), path.end(), i) == path.end())
                    tokens.push_back(i);
            }
        }
    }

    std::sort( tokens.begin(), tokens.end(), [](size_t& lhs, size_t& rhs) {
        if(lhs > rhs)
            return false;
        return true;
    });

    // RootPath::print_vector("\ttokens",tokens);
}

/**
 * @brief sub is a subpath of path
 *
 * @param sub
 * @param path
 * @return true
 * @return false
 */
bool RootPath::is_sub_path(std::vector<size_t>& sub, std::vector<size_t>& path) {
    if(sub.size() >= path.size())
        return false;

    size_t sub_size = sub.size();

    for(size_t i=0;i<sub_size;i++)
        if(sub.at(i) != path.at(i))
            return false;

    return true;
}


void RootPath::print_paths() {
    for(auto& p : mTokenMap) {
        std::cout << "Tokens in path [path " << p.first << " / " << "patterns [ ";
        for(auto& p2 : p.second) {
            std::cout << p2 << " ";
        }
        std::cout << "]]" << std::endl;
    }

    for(auto& p : mPatternMap) {
        std::cout << "Patterns in path [path " << p.first << " / " << "patterns [ ";
        for(auto& p2 : p.second) {
            std::cout << p2 << " ";
        }
        std::cout << "]]" << std::endl;
    }
}

std::vector<std::vector<size_t>> RootPath::get_all_subsets(std::vector<size_t>& set)
{
    std::vector<std::vector<size_t>> subset;
    std::vector<size_t> empty;
    subset.push_back( empty );

    for (std::size_t i = 0; i < set.size(); i++)
    {
        std::vector<std::vector<size_t>> subsetTemp = subset;  //making a copy of given 2-d vector.

        for (std::size_t j = 0L; j < subsetTemp.size(); j++)
            subsetTemp[j].push_back( set[i] );   // adding set[i] element to each subset of subsetTemp. like adding {2}(in 2nd iteration  to {{},{1}} which gives {{2},{1,2}}.

        for (std::size_t j = 0L; j < subsetTemp.size(); j++)
            subset.push_back( subsetTemp[j] );  //now adding modified subsetTemp to original subset (before{{},{1}} , after{{},{1},{2},{1,2}})
    }
    return subset;
}

std::vector<std::vector<size_t>> RootPath::generate_all_paths(std::vector<std::vector<size_t>>& patterns) {
    std::vector<std::vector<size_t>> paths;

    for(auto ptn : patterns) {
        std::vector<size_t> path;
        for(auto t : ptn) {
            path.push_back(t);
            paths.push_back(path);
        }
    }

    return paths;
}

std::vector<size_t> RootPath::path_has_pattern(std::vector<size_t> path) {
    std::string key = genereate_path_string(path);
    auto it = mPatternMap.find(key);
    if(it != mPatternMap.end()) {
        return it->second;
    }

    return std::vector<size_t>{};
}

std::vector<size_t> RootPath::path_has_tokens(std::vector<size_t> path) {
    std::string key = genereate_path_string(path);
    auto it = mTokenMap.find(key);
    if(it != mTokenMap.end()) {
        return it->second;
    }

    return std::vector<size_t>{};
}
