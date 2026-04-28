#include "ChessEngine.hpp"

#include <algorithm>

namespace Chess::Core {
RepetitionTable::RepetitionTable() { hashes.fill(0); startIndices.fill(0); }

void RepetitionTable::Init(Board& board) {
    count = static_cast<int>(board.RepetitionPositionHistory.size());
    for (int i = 0; i < count; i++) {
        hashes[i] = board.RepetitionPositionHistory[static_cast<std::size_t>(i)];
        startIndices[i] = 0;
    }
    startIndices[count] = 0;
}

void RepetitionTable::Push(std::uint64_t hash, bool reset) {
    if (count < static_cast<int>(hashes.size())) {
        hashes[count] = hash;
        startIndices[count + 1] = reset ? count : startIndices[count];
    }
    count++;
}

void RepetitionTable::TryPop() { count = std::max(0, count - 1); }

bool RepetitionTable::Contains(std::uint64_t h) const {
    int s = startIndices[count];
    for (int i = s; i < count - 1; i++) if (hashes[i] == h) return true;
    return false;
}

TranspositionTable::TranspositionTable(Board& board_, int sizeMB) : board(&board_) {
    int ttEntrySizeBytes = static_cast<int>(sizeof(Entry));
    int desiredTableSizeInBytes = sizeMB * 1024 * 1024;
    int numEntries = desiredTableSizeInBytes / ttEntrySizeBytes;
    count = static_cast<std::uint64_t>(numEntries);
    entries.resize(static_cast<std::size_t>(numEntries));
}

void TranspositionTable::Clear() { std::fill(entries.begin(), entries.end(), Entry()); }
std::uint64_t TranspositionTable::Index() const { return board->CurrentGameState.zobristKey % count; }
Move TranspositionTable::TryGetStoredMove() const { return entries[static_cast<std::size_t>(Index())].move; }

int TranspositionTable::LookupEvaluation(int depth, int plyFromRoot, int alpha, int beta) {
    if (!enabled) return LookupFailed;
    Entry entry = entries[static_cast<std::size_t>(Index())];
    if (entry.key == board->CurrentGameState.zobristKey) {
        if (entry.depth >= depth) {
            int correctedScore = CorrectRetrievedMateScore(entry.value, plyFromRoot);
            if (entry.nodeType == Exact) return correctedScore;
            if (entry.nodeType == UpperBound && correctedScore <= alpha) return correctedScore;
            if (entry.nodeType == LowerBound && correctedScore >= beta) return correctedScore;
        }
    }
    return LookupFailed;
}

void TranspositionTable::StoreEvaluation(int depth, int numPlySearched, int eval, int evalType, Move move) {
    if (!enabled) return;
    std::uint64_t index = Index();
    entries[static_cast<std::size_t>(index)] = Entry(board->CurrentGameState.zobristKey, CorrectMateScoreForStorage(eval, numPlySearched), static_cast<std::uint8_t>(depth), static_cast<std::uint8_t>(evalType), move);
}

TranspositionTable::Entry TranspositionTable::GetEntry(std::uint64_t zobristKey) const {
    return entries[static_cast<std::size_t>(zobristKey % static_cast<std::uint64_t>(entries.size()))];
}

int TranspositionTable::CorrectMateScoreForStorage(int score, int numPlySearched) const {
    if (Searcher::IsMateScore(score)) {
        int sign = (score > 0) - (score < 0);
        return (score * sign + numPlySearched) * sign;
    }
    return score;
}

int TranspositionTable::CorrectRetrievedMateScore(int score, int numPlySearched) const {
    if (Searcher::IsMateScore(score)) {
        int sign = (score > 0) - (score < 0);
        return (score * sign - numPlySearched) * sign;
    }
    return score;
}
} // namespace Chess::Core
