#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include <vector>
#include "Board.h"
#include "Move.h"

namespace MoveOrdering {
    void clear();
    void addHistoryBonus(const Move& m, int depth);
    void addKiller(const Move& m, int ply);
    void orderMoves(Board& board, std::vector<Move>& moves, int ply, const Move& ttMove);
}

#endif
