#ifndef PRECOMPUTED_H
#define PRECOMPUTED_H

#include <cstdint>

namespace Precomputed {
    extern uint64_t knightAttacks[64];
    extern uint64_t kingAttacks[64];
    extern uint64_t pawnAttacks[2][64];

    extern uint64_t bishopMasks[64];
    extern uint64_t rookMasks[64];

    uint64_t bishopAttacks(int sq, uint64_t occ);
    uint64_t rookAttacks(int sq, uint64_t occ);
    inline uint64_t queenAttacks(int sq, uint64_t occ) { return bishopAttacks(sq, occ) | rookAttacks(sq, occ); }

    void init();
}

#endif
