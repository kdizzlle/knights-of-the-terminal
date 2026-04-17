#include "Bot.h"
#include "OpeningBook.h"
#include "Searcher.h"
#include "Zobrist.h"

namespace Bot {
    void init() {
        Zobrist::init();
        OpeningBook::init();
    }

    int chooseThinkTime(const Board& board, const SearchLimits& limits) {
        if (limits.movetimeMs > 0) {
            return limits.movetimeMs;
        }

        const bool white = board.sideToMove() == WHITE;
        const int myTimeRemainingMs = white ? limits.wtime : limits.btime;
        const int myIncrementMs = white ? limits.winc : limits.binc;

        if (myTimeRemainingMs > 0) {
            double baseTime = myTimeRemainingMs / 40.0;
            double incrementBuffer = myIncrementMs * 0.8;
            int thinkTime = static_cast<int>(baseTime + incrementBuffer);
            int minThinkTime = std::min(50, std::max(1, myTimeRemainingMs / 4));
            if (thinkTime < minThinkTime) thinkTime = minThinkTime;
            if (thinkTime > myTimeRemainingMs) thinkTime = myTimeRemainingMs;
            return thinkTime;
        }

        return 1000;
    }

    Move chooseMove(Board& board, const SearchLimits& limits, const std::vector<std::string>& movesPlayed) {
        Move bookMove = OpeningBook::probe(movesPlayed);
        if (!bookMove.isNull()) {
            return bookMove;
        }

        if (limits.depth > 0 && limits.movetimeMs <= 0 && limits.wtime <= 0 && limits.btime <= 0) {
            return Searcher::findBestMove(board, limits.depth);
        }

        return Searcher::findBestMoveTimed(board, chooseThinkTime(board, limits));
    }
}
