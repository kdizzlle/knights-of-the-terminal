#include "ChessEngine.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

namespace Chess::Core {
OpeningBook::OpeningBook(const std::string& file) : rng(std::random_device{}()) {
    std::string trimmed = file;
    while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\n' || trimmed.back() == '\r' || trimmed.back() == '\t')) trimmed.pop_back();
    std::size_t pos = 0;
    while ((pos = trimmed.find("pos", pos)) != std::string::npos) {
        std::size_t next = trimmed.find("pos", pos + 3);
        std::string entry = trimmed.substr(pos + 3, next == std::string::npos ? std::string::npos : next - (pos + 3));
        while (!entry.empty() && (entry.front() == ' ' || entry.front() == '\n' || entry.front() == '\r' || entry.front() == '\t')) entry.erase(entry.begin());
        std::stringstream ss(entry);
        std::string line;
        std::getline(ss, line);
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) line.pop_back();
        std::string positionFen = line;
        std::vector<BookMove> bookMoves;
        while (std::getline(ss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            std::stringstream ls(line);
            std::string move; int n = 0;
            ls >> move >> n;
            if (!move.empty()) bookMoves.emplace_back(move, n);
        }
        if (!bookMoves.empty()) movesByPosition.emplace(positionFen, bookMoves);
        if (next == std::string::npos) break;
        pos = next;
    }
}

bool OpeningBook::HasBookMove(const std::string& positionFen) const {
    return movesByPosition.contains(RemoveMoveCountersFromFEN(positionFen));
}

bool OpeningBook::TryGetBookMove(Board& board, std::string& moveString, double weightPow) {
    std::string positionFen = FenUtility::CurrentFen(board, false);
    weightPow = std::clamp(weightPow, 0.0, 1.0);
    auto it = movesByPosition.find(RemoveMoveCountersFromFEN(positionFen));
    if (it != movesByPosition.end()) {
        const auto& moves = it->second;
        auto weighted = [&](int playCount) { return static_cast<int>(std::ceil(std::pow(playCount, weightPow))); };
        int totalPlayCount = 0;
        for (const auto& m : moves) totalPlayCount += weighted(m.numTimesPlayed);
        std::vector<double> weights(moves.size());
        double weightSum = 0;
        for (std::size_t i = 0; i < moves.size(); i++) {
            double weight = weighted(moves[i].numTimesPlayed) / static_cast<double>(totalPlayCount);
            weightSum += weight;
            weights[i] = weight;
        }
        std::vector<double> probCumul(moves.size());
        for (std::size_t i = 0; i < weights.size(); i++) {
            double prob = weights[i] / weightSum;
            probCumul[i] = (i == 0 ? 0.0 : probCumul[i-1]) + prob;
        }
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double random = dist(rng);
        for (std::size_t i = 0; i < moves.size(); i++) {
            if (random <= probCumul[i]) { moveString = moves[i].moveString; return true; }
        }
    }
    moveString = "Null";
    return false;
}

std::string OpeningBook::RemoveMoveCountersFromFEN(const std::string& fen) {
    auto a = fen.substr(0, fen.find_last_of(' '));
    return a.substr(0, a.find_last_of(' '));
}
} // namespace Chess::Core
