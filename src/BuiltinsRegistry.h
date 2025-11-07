//
// Created by Nick on 08.11.2025.
//

#ifndef SHELL_STARTER_CPP_BUILTINSREGISTRY_H
#define SHELL_STARTER_CPP_BUILTINSREGISTRY_H
#include <unordered_map>
#include <functional>
#include <string>
#include <iostream>

class BuiltinsRegistry
{
private:
    std::unordered_map<std::string, std::function<void(BuiltinsRegistry&, const std::string&)>> commands;

public:
    void RegisterCommand(const std::string& name, std::function<void(BuiltinsRegistry&, const std::string&)> func);
    void Execute(const std::string& name, const std::string& args);
    std::vector<std::string> GetKeys() const;
    bool HasCommand(const std::string& name) const;
};


#endif //SHELL_STARTER_CPP_BUILTINSREGISTRY_H