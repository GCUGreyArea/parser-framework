#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
#include <vector>
#include <string>
#include <random>

#include <hash.h>

/**
 * @brief Generate a vector of random strings
 * 
 * @param size 
 * @param length 
 * @return std::vector<std::string> 
 */
static std::vector<std::string> build_random_strings(unsigned int size, unsigned int length) {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<char> distr(0x20, 0x7F); // define the range

    std::vector<std::string> strs;

    for(unsigned int i=0; i < size; i++) {
        std::string str = "";
        for(unsigned int j=0; j < length; j++) {
            char st[2] =  {'\0'};
            st[0] = (char) distr(gen);
            str += st; 
        }

        strs.push_back(str);
    }   

    return strs;
}

/**
 * @brief Test that a 4096 paths of 2048 length will not 
 * cause a hash colission. It is unlikely that a real world 
 * message will contain this number of long paths, but I 
 * have seen very long logs from AWS. 
 * 
 */
TEST(hashFunction, testHashFunctionDistribution) {
    // Generate a large number of random strings and 
    // test the amount distribution of the hashing function

    std::map<unsigned int,std::vector<std::string>> hash_map;

    std::vector<std::string> strings = build_random_strings(4096, 2048);
    // print random string to see what we have

    for(auto s : strings) {
        unsigned val = hash(s.c_str(),s.length());
        auto it = hash_map.find(val);
        if(it != hash_map.end()) {
            it->second.push_back(s);
        }
        else {
            std::vector<std::string> st_vect;
            st_vect.push_back(s);
            hash_map.emplace(val,st_vect);
        }
    }

    for (auto const& [key, val] : hash_map) {
        ASSERT_TRUE(val.size() == 1);
    }
}

/**
 * @brief Test that the concatination of long
 * paths will not cause a hash conflict. 
 * 
 * @note We need to do this with multi level 
 * hash concatination.
 * 
 */
TEST(hashFunction, testHashMergeFunctionDistributionForTwoMerges) {
    struct pair {
        std::string str1;
        std::string str2;
    };

    std::map<unsigned int,std::vector<pair>> hash_map;

    std::vector<std::string> strings = build_random_strings(4096*2, 2048);
    // print random string to see what we have

    int count = 0;
    unsigned next;
    std::string last;
    for(auto s : strings) {
        count++;
        
        pair p;
        unsigned val = hash(s.c_str(),s.length());
        if(count == 1) {
            next = val;
            last = s;
        }
        else {
            val = merge_hash(next,val);
            p = {last,s};
        }

        if(count == 2) {
            auto it = hash_map.find(val);
            if(it != hash_map.end()) {
                it->second.push_back(p);
            }
            else {
                std::vector<pair> st_vect;
                st_vect.push_back(p);
                hash_map.emplace(val,st_vect);
            }
        }
    }

    for (auto const& [key, val] : hash_map) {
        ASSERT_TRUE(val.size() == 1);
    }
}