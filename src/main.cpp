#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include "BuiltinsRegistry.h"
#include "utils/string_utils.h"

namespace string_utils = shell::string_utils;

#ifdef _WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif


std::vector<std::string> GenerateArguments(const std::string& args, bool collapseSpaces)
{
    std::vector<std::string> argsVector;
    if (!args.starts_with('\''))
    {
        std::string newArgs = string_utils::remove_characters(args, '\'');
        if (collapseSpaces)
        {
            argsVector.push_back(string_utils::string_join(string_utils::split(newArgs, ' '), ' '));
        }
        else
        {
            argsVector = string_utils::split(newArgs, ' ');
        }
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
                if (currentArg != "")
                {
                    argsVector.push_back(currentArg);
                    currentArg.clear();
                }
                continue;
            }

            currentArg += ch;
        }

        if (currentArg != "")
            argsVector.push_back(currentArg);
    }

    return argsVector;
}


std::string read_file(const std::string& filePath)
{
    std::ifstream f(filePath);

    // Check if the file is
    // successfully opened
    if (!f.is_open())
    {
        std::cerr << "Error opening the file!";
        return "";
    }

    std::string s;

    // Read each line of the file, store
    // it in string s and print it to the
    // standard output stream
    while (getline(f, s))
    {}

    // Close the file
    f.close();
    return s;
}


int main()
{
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    BuiltinsRegistry registry;

    registry.RegisterCommand("echo", [](BuiltinsRegistry& reg, const std::string& args)
    {
        std::vector<std::string> argsVector = GenerateArguments(args, true);
        if (argsVector.size() > 0)
        {
            for (std::string arg : argsVector)
            {
                std::cout << arg;
            }
            std::cout << std::endl;
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
            std::vector<std::string> pathParts = string_utils::split(currentPath, pathDelimiter);
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

            newPath = string_utils::path_join(pathParts, pathDelimiter) + pathDelimiter + newPath;
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
            if (string_utils::check_path(args, outDir))
            {
                std::cout << args << " is " << outDir << "\n";
            }
            else
            {
                std::cout << args << ": not found" << std::endl;
            }
        }
    });

    registry.RegisterCommand("cat", [](BuiltinsRegistry& reg, const std::string& args)
    {
        std::vector<std::string> argsVector = GenerateArguments(args, true);
        if (argsVector.size() > 0)
        {
            for (std::string arg : argsVector)
            {
                if (arg == " " || arg == "")
                    continue;
                std::cout << read_file(arg);
            }
            std::cout << std::endl;
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

        if (string_utils::check_path(commandWithoutArgs, outDir))
        {
            std::system(command.c_str());
            continue;
        }

        std::cout << command << ": command not found" << std::endl;
    }
}
