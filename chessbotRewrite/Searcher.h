#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "Move.h"

namespace Searcher {
    void init();
    Move findBestMove(Board& board, int depth);
    Move findBestMoveTimed(Board& board, int timeMs);
}

#endif
