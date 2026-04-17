#ifndef BOT_H
#define BOT_H

#include <vector>
#include <string>
#include "Board.h"
#include "Move.h"

struct SearchLimits {
    int movetimeMs = -1;
    int depth = -1;
    int wtime = -1;
    int btime = -1;
    int winc = 0;
    int binc = 0;
};

namespace Bot {
    void init();
    int chooseThinkTime(const Board& board, const SearchLimits& limits);
    Move chooseMove(Board& board, const SearchLimits& limits, const std::vector<std::string>& movesPlayed);
}

#endif
