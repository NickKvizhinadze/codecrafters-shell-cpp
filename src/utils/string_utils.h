//
// Created by Nick on 21.11.2025.
//

#ifndef SHELL_STARTER_CPP_STRING_UTILS_H
#define SHELL_STARTER_CPP_STRING_UTILS_H
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

namespace shell::string_utils
{
    std::vector<std::string> split(const std::string& str, char delimiter);
    bool check_path(const std::string& command, std::string& outDir);
    std::string path_join(const std::vector<std::string>& stringParts, char delimiter);
    std::string string_join(const std::vector<std::string>& stringParts, char delimiter);
    std::string remove_characters(const std::string& str, char characterToRemove);

} // shell::string_utils

#endif //SHELL_STARTER_CPP_STRING_UTILS_H
