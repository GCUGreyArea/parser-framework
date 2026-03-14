#include <jsmn.hpp>
#include <stdio.h>
#include <string.h>

#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
    if (argc == 1)
        return -1;

    jsmn_parser p(argv[1]);

    int count = p.parse();
    if (count < 0) {
        std::cerr << "Failed to parse: " << argv[1] << std::endl;
        return -1;
    }

    std::cout << "Parsed tokens: " << count << std::endl;
    for (unsigned int i = 0; i < count; i++) {
        std::cout << "Token: " << i << " ";
        p.print_token(i);
    }

    p.serialise("test.bin");
    p.init();
    p.deserialise("test.bin");

    count = p.parsed_tokens();
    if (count < 0) {
        std::cerr << "Failed to aquire parsed tokens from: " << argv[1]
                  << std::endl;
        return -1;
    }

    std::cout << std::endl << "Deserialised tokens: " << count << std::endl;
    for (unsigned int i = 0; i < count; i++) {
        std::cout << "Token: " << i << " ";
        p.print_token(i);
    }

    if (argc == 3) {
        const char *search = argv[2];
        if (search != nullptr) {
            struct jqpath *path = path_parse_string(search);
            if (path != nullptr) {
                JQ *val = p.get_path(path);
                if (val != nullptr) {
                    std::string str = p.extract_string(val->get_token());
                    std::cout << std::endl
                              << "Path value for " << argv[2] << "  : " << str
                              << std::endl;
                } else {
                    std::cerr << "Failed to find path: " << argv[2]
                              << std::endl;
                    exit - 1;
                }
            } else {
                std::cerr << "Failed to parse path: " << argv[2] << std::endl;
                return -1;
            }
        }
    }

    std::filesystem::remove("test.bin");

    return 0;
}
