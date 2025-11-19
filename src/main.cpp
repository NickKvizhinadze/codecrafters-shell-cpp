#include <iostream>
#include <string>
#include <filesystem>
#include <sstream>
#include <vector>
#include "BuiltinsRegistry.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif


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

std::string PathJoin(const std::vector<std::string>& stringParts, char delimiter)
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

std::vector<std::string> GenerateArguments(const std::string& args)
{
    std::vector<std::string> argsVector;
    if (!args.starts_with('\''))
    {
        argsVector = Split(args, ' ');
    }
    else
    {
        bool isQuotesStarted = false;
        std::string currentArg = "";
        for (char ch : args)
        {
            if (ch == '\'')
            {
                isQuotesStarted = !isQuotesStarted;
                if (!isQuotesStarted && currentArg != "")
                {
                    argsVector.push_back(currentArg);
                    currentArg.clear();
                }
                continue;
            }

            if (isQuotesStarted)
            {
                currentArg += ch;
            }
        }
    }

    return argsVector;
}

int main()
{
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    BuiltinsRegistry registry;

    registry.RegisterCommand("echo", [](BuiltinsRegistry& reg, const std::string& args)
    {
        std::vector<std::string> argsVector = GenerateArguments(args);
        if (argsVector.size() > 0)
        {
            for (std::string arg : argsVector)
            {
                std::cout << arg << std::endl;
            }
        }
    });

    registry.RegisterCommand("pwd", [](BuiltinsRegistry& reg, const std::string& args)
    {
        std::cout << std::filesystem::current_path().string() << "\n";
    });

    registry.RegisterCommand("cd", [](BuiltinsRegistry& reg, const std::string& args)
    {
        std::string newPath = args;

        char pathDelimiter;
#ifdef _WIN32
        pathDelimiter = '\\';
#else
        pathDelimiter = '/';
#endif

        if (newPath == "~")
        {
#ifdef _WIN32
            newPath = getenv("USERPROFILE");
#else
            const char* home = getenv("HOME");
            if (home != nullptr)
            {
                newPath = home;
            }
            else
            {
                newPath = getpwuid(getuid())->pw_dir;
            }
#endif
        }
        else if (newPath == "/")
        {
            newPath = std::filesystem::current_path().root_directory().string();
        }
        else if (newPath.starts_with("./") || newPath.starts_with("../"))
        {
            while (newPath.starts_with("./"))
            {
                newPath = newPath.substr(2);
            }

            std::string currentPath = std::filesystem::current_path().string();
            std::vector<std::string> pathParts = Split(currentPath, pathDelimiter);
            while (newPath.starts_with("../"))
            {
                if (pathParts.size() == 0)
                {
                    std::cout << "cd: " << args << ": No such file or directory" << "\n";
                    return;
                }

                pathParts.erase(--pathParts.end());
                newPath = newPath.substr(3);
            }

            newPath = PathJoin(pathParts, pathDelimiter) + pathDelimiter + newPath;
        }

        if (!std::filesystem::exists(newPath))
        {
            std::cout << "cd: " << args << ": No such file or directory" << "\n";
            return;
        }

#ifdef _WIN32
        SetCurrentDirectoryA(newPath.c_str());
#else
        chdir(newPath.c_str());
#endif
    });

    registry.RegisterCommand("type", [](BuiltinsRegistry& reg, const std::string& args)
    {
        if (args == "exit" || reg.HasCommand(args))
            std::cout << args << " is a shell builtin" << std::endl;
        else
        {
            std::string outDir;
            if (checkPath(args, outDir))
            {
                std::cout << args << " is " << outDir << "\n";
            }
            else
            {
                std::cout << args << ": not found" << std::endl;
            }
        }
    });

    while (true)
    {
        std::cout << "$ ";

        std::string command;
        std::getline(std::cin, command);

        if (command == "exit 0")
            return 0;

        if (command == "exit 1")
            return 1;

        size_t spacePos = command.find(' ');
        std::string commandWithoutArgs = command.substr(0, spacePos);
        std::string args = command.substr(spacePos + 1);

        if (registry.HasCommand(commandWithoutArgs))
        {
            registry.Execute(commandWithoutArgs, args);
            continue;
        }

        std::string outDir;

        if (checkPath(commandWithoutArgs, outDir))
        {
            std::system(command.c_str());
            continue;
        }

        std::cout << command << ": command not found" << std::endl;
    }
}
