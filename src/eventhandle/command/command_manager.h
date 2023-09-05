#pragma once

#include <vector>

#include "commands/ICommand.h"

class CommandManager {
public:
    static void Init();
    static void Execute(std::string& command_name, std::string& args);

private:

};
