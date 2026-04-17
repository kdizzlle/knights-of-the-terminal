#include "MoveOrdering.h"
#include <algorithm>

namespace {
    constexpr int MAX_PLY = 128;
    Move killer[2][MAX_PLY];
    int history[64][64];

    int pieceValue(int p) {
        if (p == NO_PIECE) return 0;
        static const int vals[6] = {100, 320, 330, 500, 900, 20000};
        return vals[p % 6];
    }

    bool sameMove(const Move& a, const Move& b) {
        return a.data == b.data;
    }

    int moveScore(Board& board, const Move& m, int ply, const Move& ttMove) {
        if (!ttMove.isNull() && sameMove(m, ttMove)) return 2000000000;
        if (m.isCapture()) {
            int victim = board.pieceOn(m.to());
            int attacker = board.pieceOn(m.from());
            return 1000000 + 16 * pieceValue(victim) - pieceValue(attacker);
        }
        if (m.isPromotion()) return 900000;
        if (ply < MAX_PLY) {
            if (sameMove(m, killer[0][ply])) return 800000;
            if (sameMove(m, killer[1][ply])) return 700000;
        }
        return history[m.from()][m.to()];
    }
}

namespace MoveOrdering {
    void clear() {
        for (int i = 0; i < 2; ++i)
            for (int p = 0; p < MAX_PLY; ++p)
                killer[i][p] = Move();
        for (int i = 0; i < 64; ++i)
            for (int j = 0; j < 64; ++j)
                history[i][j] = 0;
    }

    void addHistoryBonus(const Move& m, int depth) {
        history[m.from()][m.to()] += depth * depth;
    }

    void addKiller(const Move& m, int ply) {
        if (ply >= MAX_PLY) return;
        if (killer[0][ply].data != m.data) {
            killer[1][ply] = killer[0][ply];
            killer[0][ply] = m;
        }
    }

    void orderMoves(Board& board, std::vector<Move>& moves, int ply, const Move& ttMove) {
        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            return moveScore(board, a, ply, ttMove) > moveScore(board, b, ply, ttMove);
        });
    }
}
