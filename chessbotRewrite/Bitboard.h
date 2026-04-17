#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>

inline int pop_lsb(uint64_t &bb) {
#if defined(__GNUG__)
    int sq = __builtin_ctzll(bb);
#else
    int sq = 0; while (((bb >> sq) & 1ULL) == 0) ++sq;
#endif
    bb &= bb - 1;
    return sq;
}

inline int popcount(uint64_t bb) {
#if defined(__GNUG__)
    return __builtin_popcountll(bb);
#else
    int c = 0; while (bb) { bb &= bb - 1; ++c; } return c;
#endif
}

#endif
