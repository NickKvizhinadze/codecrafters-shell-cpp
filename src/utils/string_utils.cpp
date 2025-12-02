//
// Created by Nick on 21.11.2025.
//

#include "string_utils.h"

namespace shell::string_utils
{
    std::vector<std::string> split(const std::string& str, char delimiter)
    {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter))
        {
            if (!token.empty())
            {
                result.push_back(token);
            }
        }

        return result;
    }

    bool check_path(const std::string& command, std::string& outDir)
    {
#ifdef _WIN32
        const std::string osPathSeparator(";");
#else
        const std::string osPathSeparator(":");
#endif

        const char delimiter = osPathSeparator.c_str()[0];
        const char* path = std::getenv("PATH");

        std::vector<std::string> pathVariables = shell::string_utils::split(path, delimiter);

        for (std::string token : pathVariables)
        {
            if (std::filesystem::exists(token) && std::filesystem::is_directory(token))
            {
                for (auto& entry : std::filesystem::directory_iterator(token))
                {
                    std::error_code ec;
                    if (!std::filesystem::is_regular_file(entry, ec))
                        continue;

                    auto perms = std::filesystem::status(entry).permissions();
                    if ((perms & std::filesystem::perms::owner_exec) == std::filesystem::perms::none &&
                        (perms & std::filesystem::perms::group_exec) == std::filesystem::perms::none &&
                        (perms & std::filesystem::perms::others_exec) == std::filesystem::perms::none)
                        continue;

                    if (command == entry.path().stem())
                    {
                        outDir = entry.path().string();
                        return true;
                    }
                }
            }
        }

        return false;
    }


    std::string path_join(const std::vector<std::string>& stringParts, char delimiter)
    {
        std::string result;
        for (int i = 0; i < stringParts.size(); i++)
        {
#ifdef _WIN32
            if (i == 0)
                result = stringParts[i];
            else
                result += (delimiter + stringParts[i]);
#else
            result += (delimiter + stringParts[i]);
#endif
        }

        return result;
    }

    std::string string_join(const std::vector<std::string>& stringParts, char delimiter)
    {
        std::string result;
        for (int i = 0; i < stringParts.size(); i++)
        {
            if (i == 0)
                result = stringParts[i];
            else
                result += (delimiter + stringParts[i]);
        }

        return result;
    }

    std::string remove_characters(const std::string& str, char characterToRemove)
    {
        std::string result = str;
        result.erase(std::remove(result.begin(), result.end(), '\''), result.end());
        return result;
    }
} // shell::string_utils