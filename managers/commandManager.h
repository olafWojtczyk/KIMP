#pragma once

#include <functional>
#include <vector>
#include <string>
#include <cstring>
//#include "patricia.h"

class CommandManager {
public:
    void registerCommand(std::string_view name, std::function<void()> action);
    bool run(std::string_view command);

    //Patricia<std::string, __int64>* commands = new Patricia<std::string, __int64>();
    std::unordered_map<std::string_view, std::function<void()>> commands;

private:

};