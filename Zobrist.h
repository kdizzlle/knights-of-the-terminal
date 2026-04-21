#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <cstdint>
#include "Types.h"

class Board;

namespace Zobrist {
    extern uint64_t piece[PIECE_NB][64];
    extern uint64_t castling[16];
    extern uint64_t ep[8];
    extern uint64_t side;
    void init();
    uint64_t hash(const Board& b);
}

#endif
