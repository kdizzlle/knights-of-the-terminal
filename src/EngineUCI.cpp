#include "ChessEngine.hpp"
#include "InternalUtils.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

namespace CodingAdventureBot {
using namespace Chess::Core;

EngineUCI::EngineUCI() = default;

void EngineUCI::EnsurePlayer() {
    if (!player) {
        player = std::make_unique<Bot>();
        player->OnMoveChosen = [this](const std::string& move) { OnMoveChosen(move); };
    }
}

void EngineUCI::ReceiveCommand(const std::string& messageRaw) {
    LogToFile("Command received: " + messageRaw);
    std::string message = Detail::trim_copy(messageRaw);
    if (message.empty()) return;
    std::string messageType;
    {
        std::istringstream iss(message);
        iss >> messageType;
        messageType = Detail::lower_copy(messageType);
    }

    if (messageType == "uci") {
        Respond("id name ChessBot");
        Respond("id author KnightsoftheTerminal");
        Respond("uciok");
    } else if (messageType == "isready") {
        EnsurePlayer();
        Respond("readyok");
    } else if (messageType == "ucinewgame") {
        EnsurePlayer();
        player->NotifyNewGame();
    } else if (messageType == "position") {
        ProcessPositionCommand(message);
    } else if (messageType == "go") {
        ProcessGoCommand(message);
    } else if (messageType == "stop") {
        if (player && player->IsThinking()) player->StopThinking();
    } else if (messageType == "setoption" || messageType == "ponderhit") {
        // No-op for GUI compatibility.
    } else if (messageType == "quit") {
        if (player) player->Quit();
    } else if (messageType == "d") {
        EnsurePlayer();
        std::cout << player->GetBoardDiagram();
    } else {
        LogToFile("Unrecognized command: " + messageType);
    }
}

void EngineUCI::OnMoveChosen(const std::string& move) {
    LogToFile(std::string("OnMoveChosen: book move = ") + ((player && player->LatestMoveIsBookMove) ? "true" : "false"));
    Respond("bestmove " + move);
}

void EngineUCI::ProcessGoCommand(const std::string& message) {
    std::vector<std::string> labels(goLabels.begin(), goLabels.end());
    if (message.find("movetime") != std::string::npos) {
        int moveTimeMs = TryGetLabelledValueInt(message, "movetime", labels, 0);
        EnsurePlayer();
        player->ThinkTimed(moveTimeMs);
    } else {
        int timeRemainingWhiteMs = TryGetLabelledValueInt(message, "wtime", labels, 0);
        int timeRemainingBlackMs = TryGetLabelledValueInt(message, "btime", labels, 0);
        int incrementWhiteMs = TryGetLabelledValueInt(message, "winc", labels, 0);
        int incrementBlackMs = TryGetLabelledValueInt(message, "binc", labels, 0);
        EnsurePlayer();
        int thinkTime = player->ChooseThinkTime(timeRemainingWhiteMs, timeRemainingBlackMs, incrementWhiteMs, incrementBlackMs);
        LogToFile("Thinking for: " + std::to_string(thinkTime) + " ms.");
        player->ThinkTimed(thinkTime);
    }
}

void EngineUCI::ProcessPositionCommand(const std::string& message) {
    std::vector<std::string> posLabels(positionLabels.begin(), positionLabels.end());
    std::string lower = Detail::lower_copy(message);
    if (lower.find("startpos") != std::string::npos) {
        EnsurePlayer();
        player->SetPosition(FenUtility::StartPositionFEN);
    } else if (lower.find("fen") != std::string::npos) {
        std::string customFen = TryGetLabelledValue(message, "fen", posLabels);
        EnsurePlayer();
        player->SetPosition(customFen);
    } else {
        std::cout << "Invalid position command (expected 'startpos' or 'fen')" << std::endl;
    }
    std::string allMoves = TryGetLabelledValue(message, "moves", posLabels);
    if (!allMoves.empty()) {
        std::istringstream iss(allMoves);
        std::string move;
        int count = 0;
        while (iss >> move) {
            player->MakeMove(move);
            count++;
        }
        LogToFile("Make moves after setting position: " + std::to_string(count));
    }
}

void EngineUCI::Respond(const std::string& response) {
    std::cout << response << std::endl;
    LogToFile("Response sent: " + response);
}

int EngineUCI::TryGetLabelledValueInt(const std::string& text, const std::string& label, const std::vector<std::string>& allLabels, int defaultValue) {
    std::string valueString = TryGetLabelledValue(text, label, allLabels, std::to_string(defaultValue));
    std::istringstream iss(valueString);
    int result = defaultValue;
    if (iss >> result) return result;
    return defaultValue;
}

std::string EngineUCI::TryGetLabelledValue(const std::string& textRaw, const std::string& label, const std::vector<std::string>& allLabels, const std::string& defaultValue) {
    std::string text = Detail::trim_copy(textRaw);
    auto pos = Detail::lower_copy(text).find(label);
    if (pos != std::string::npos) {
        std::size_t valueStart = pos + label.size();
        std::size_t valueEnd = text.size();
        std::string lower = Detail::lower_copy(text);
        for (const std::string& other : allLabels) {
            if (other != label) {
                auto otherPos = lower.find(other);
                if (otherPos != std::string::npos && otherPos > valueStart && otherPos < valueEnd) valueEnd = otherPos;
            }
        }
        return Detail::trim_copy(text.substr(valueStart, valueEnd - valueStart));
    }
    return defaultValue;
}

void EngineUCI::LogToFile(const std::string& text) {
    if (!logToFile) return;
    std::filesystem::create_directories(AppDataPath());
    std::ofstream out(std::filesystem::path(AppDataPath()) / "UCI_Log.txt", std::ios::app);
    out << text << '\n';
}

std::string EngineUCI::AppDataPath() {
    const char* home = std::getenv("HOME");
    std::filesystem::path base = home ? std::filesystem::path(home) / ".local" / "share" : std::filesystem::temp_directory_path();
    return (base / "Chess-Coding-Adventure").string();
}
} // namespace CodingAdventureBot
