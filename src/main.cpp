#include <iostream>
#include <string>
#include <filesystem>
#include <sstream>
#include <vector>

std::vector<std::string> Split(const std::string& str, char delimiter)
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

bool checkPath(const std::string& command, std::string& outDir)
{
#ifdef _WIN32
    const std::string osPathSeparator(";");
#else
    const std::string osPathSeparator(":");
#endif

    const char delimiter = osPathSeparator.c_str()[0];
    const char* path = std::getenv("PATH");

    std::vector<std::string> pathVariables = Split(path, delimiter);

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

int main()
{
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true)
    {
        std::cout << "$ ";

        std::string command;
        std::getline(std::cin, command);

        if (command == "exit 0")
            return 0;

        if (command == "exit 1")
            return 1;

        if (command.starts_with("echo"))
        {
            std::cout << command.substr(5, command.length() - 5) << std::endl;
            continue;
        }

        if (command == "pwd")
        {
            std::cout << std::filesystem::current_path() << "\n";
            continue;
        }

        if (command.starts_with("type"))
        {
            command = command.substr(5, command.length() - 5);
            if (command == "exit" || command == "echo" || command == "type")
                std::cout << command << " is a shell builtin" << std::endl;
            else
            {
                std::string outDir;
                if (checkPath(command, outDir))
                {
                    std::cout << command << " is " << outDir << "\n";
                }
                else
                {
                    std::cout << command << ": not found" << std::endl;
                }
            }

            continue;
        }

        std::string outDir;
        size_t spacePos = command.find(' ');
        std::string commandWithoutVariable = command.substr(0, spacePos);

        if (checkPath(commandWithoutVariable, outDir))
        {
            std::system(command.c_str());
            continue;
        }

        std::cout << command << ": command not found" << std::endl;
    }
}
