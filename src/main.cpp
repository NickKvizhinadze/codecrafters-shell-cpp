#include <iostream>
#include <string>

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

        if (command.starts_with("type"))
        {
            command = command.substr(5, command.length() - 5);
            if (command == "exit" || command == "echo" || command == "type")
                std::cout << command << " is a shell builtin" << std::endl;
            else
                std::cout << command << ": not found" << std::endl;

            continue;
        }

        std::cout << command << ": command not found" << std::endl;
    }
}
