#pragma once

#include <string>
#include <vector>

class ICommand {
public:
    std::string CommandName, Usage, Description;
    std::vector<std::string> Aliases;

public:
    ICommand(std::string cmd_name, std::string usage = "No info was provided", std::string desc = "No info was provided");

    // return true if successful, otherwise return false
    bool Execute(const std::string& args);

};
