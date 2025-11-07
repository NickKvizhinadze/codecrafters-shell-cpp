//
// Created by Nick on 08.11.2025.
//

#include "BuiltinsRegistry.h"


void BuiltinsRegistry::RegisterCommand(const std::string& name,
                                       std::function<void(BuiltinsRegistry&, const std::string&)> func)
{
    commands[name] = func;
}

void BuiltinsRegistry::Execute(const std::string& name, const std::string& args)
{
    if (commands.contains(name))
    {
        commands[name](*this, args);
    }
}

std::vector<std::string> BuiltinsRegistry::GetKeys() const
{
    std::vector<std::string> names;
    for (const auto& pair : commands)
    {
        names.push_back(pair.first);
    }
    return names;
}

bool BuiltinsRegistry::HasCommand(const std::string& name) const
{
    return commands.contains(name);
}
