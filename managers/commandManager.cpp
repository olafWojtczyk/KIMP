#include <iostream>
#include <utility>
#include "commandManager.h"

void CommandManager::registerCommand(std::string_view name, std::function<void()> action) {
    __int64 addr = *(__int64 *)(char *)&action;
    //commands->insert(std::move(name), addr);
    commands.insert({name, action});
}

bool CommandManager::run(std::string_view command) {
    //__int64 addr = commands->search(std::move(command));
    std::function<void()> addr;
    try {
        auto addr = commands.at(command);
        addr();
        return true;
    }
    catch (const std::out_of_range &e) {}

    return false;
}