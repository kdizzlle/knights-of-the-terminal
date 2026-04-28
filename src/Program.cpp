#include "ChessEngine.hpp"

#include <iostream>
#include <string>

using namespace CodingAdventureBot;

int main() {
    EngineUCI engine;
    std::string command;
    while (command != "quit" && std::getline(std::cin, command)) {
        engine.ReceiveCommand(command);
    }
    return 0;
}
