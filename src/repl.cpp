#include "include/repl/repl.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

void repl() {
    std::string query;
    // One line queries for the moment.
    while (true) {
        std::cout << "db> ";
        if (!std::getline(std::cin, query)) break;
        if (query == "exit") break;
        std::cout << std::endl;
    }
}
