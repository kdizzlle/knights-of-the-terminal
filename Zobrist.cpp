#include "Zobrist.h"
#include "Board.h"

namespace Zobrist {
    uint64_t piece[PIECE_NB][64];
    uint64_t castling[16];
    uint64_t ep[8];
    uint64_t side;

    static uint64_t next(uint64_t& x) {
        x ^= x >> 12; x ^= x << 25; x ^= x >> 27; return x * 2685821657736338717ULL;
    }

    void init() {
        static bool done = false; if (done) return;
        uint64_t seed = 0x9E3779B97F4A7C15ULL;
        for (int p=0;p<PIECE_NB;++p) for (int sq=0;sq<64;++sq) piece[p][sq] = next(seed);
        for (int i=0;i<16;++i) castling[i] = next(seed);
        for (int i=0;i<8;++i) ep[i] = next(seed);
        side = next(seed);
        done = true;
    }

    uint64_t hash(const Board& b) {
        uint64_t h = 0ULL;
        for (int sq=0;sq<64;++sq) {
            int p = b.pieceOn(sq);
            if (p != NO_PIECE) h ^= piece[p][sq];
        }
        h ^= castling[b.castlingRights & 15];
        if (b.epSquare != -1) h ^= ep[sq_file(b.epSquare)];
        if (b.sideToMove() == BLACK) h ^= side;
        return h;
    }
}
