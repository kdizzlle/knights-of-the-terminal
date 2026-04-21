#include "Evaluation.h"
#include "Bitboard.h"
#include "Types.h"

#include <cstdlib>

namespace {
    const int pieceValue[6] = {100, 320, 330, 500, 900, 0};

    const int pstPawn[64] = {0,0,0,0,0,0,0,0,5,10,10,-20,-20,10,10,5,5,-5,-10,0,0,-10,-5,5,0,0,0,20,20,0,0,0,5,5,10,25,25,10,5,5,10,10,20,30,30,20,10,10,50,50,50,50,50,50,50,50,0,0,0,0,0,0,0,0};
    const int pstKnight[64] = {-50,-40,-30,-30,-30,-30,-40,-50,-40,-20,0,5,5,0,-20,-40,-30,5,10,15,15,10,5,-30,-30,0,15,20,20,15,0,-30,-30,5,15,20,20,15,5,-30,-30,0,10,15,15,10,0,-30,-40,-20,0,0,0,0,-20,-40,-50,-40,-30,-30,-30,-30,-40,-50};
    const int pstBishop[64] = {-20,-10,-10,-10,-10,-10,-10,-20,-10,5,0,0,0,0,5,-10,-10,10,10,10,10,10,10,-10,-10,0,10,10,10,10,0,-10,-10,5,5,10,10,5,5,-10,-10,0,5,10,10,5,0,-10,-10,0,0,0,0,0,0,-10,-20,-10,-10,-10,-10,-10,-10,-20};
    const int pstRook[64] = {0,0,5,10,10,5,0,0,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,5,10,10,10,10,10,10,5,0,0,5,10,10,5,0,0};
    const int pstQueen[64] = {-20,-10,-10,-5,-5,-10,-10,-20,-10,0,0,0,0,0,0,-10,-10,0,5,5,5,5,0,-10,-5,0,5,5,5,5,0,-5,0,0,5,5,5,5,0,-5,-10,5,5,5,5,5,0,-10,-10,0,5,0,0,0,0,-10,-20,-10,-10,-5,-5,-10,-10,-20};
    const int pstKingMG[64] = {-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-20,-30,-30,-40,-40,-30,-30,-20,-10,-20,-20,-20,-20,-20,-20,-10,20,20,0,0,0,0,20,20,20,30,10,0,0,10,30,20};
    const int pstKingEG[64] = {-50,-40,-30,-20,-20,-30,-40,-50,-30,-20,-10,0,0,-10,-20,-30,-30,-10,20,30,30,20,-10,-30,-30,-10,30,40,40,30,-10,-30,-30,-10,30,40,40,30,-10,-30,-30,-10,20,30,30,20,-10,-30,-30,-30,0,0,0,0,-30,-30,-50,-30,-30,-30,-30,-30,-30,-50};

    int mirror(int sq) { return ((7 - (sq >> 3)) << 3) | (sq & 7); }
    int fileOf(int sq) { return sq & 7; }
    int rankOf(int sq) { return sq >> 3; }
    bool isWhite(int p) { return p != NO_PIECE && p < 6; }
    bool isBlack(int p) { return p != NO_PIECE && p >= 6; }
    int typeOf(int p) { return p == NO_PIECE ? -1 : p % 6; }

    int pstFor(int piece, int sq, bool endgame) {
        bool white = isWhite(piece);
        int tsq = white ? sq : mirror(sq);
        switch (typeOf(piece)) {
            case PAWN: return pstPawn[tsq];
            case KNIGHT: return pstKnight[tsq];
            case BISHOP: return pstBishop[tsq];
            case ROOK: return pstRook[tsq];
            case QUEEN: return pstQueen[tsq];
            case KING: return endgame ? pstKingEG[tsq] : pstKingMG[tsq];
            default: return 0;
        }
    }

    int gamePhase(const Board& b) {
        int phase = 0;
        for (int sq = 0; sq < 64; ++sq) {
            int p = b.pieceOn(sq);
            if (p == NO_PIECE) continue;
            switch (typeOf(p)) {
                case KNIGHT:
                case BISHOP: phase += 1; break;
                case ROOK: phase += 2; break;
                case QUEEN: phase += 4; break;
                default: break;
            }
        }
        if (phase > 24) phase = 24;
        return phase;
    }

    int countMobilityKnight(const Board& b, int sq, Color us) {
        static const int jumps[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
        int f = fileOf(sq), r = rankOf(sq), score = 0;
        for (auto &d : jumps) {
            int ff = f + d[0], rr = r + d[1];
            if (ff < 0 || ff > 7 || rr < 0 || rr > 7) continue;
            int p = b.pieceOn(make_sq(ff, rr));
            if (p == NO_PIECE || (us == WHITE ? isBlack(p) : isWhite(p))) score++;
        }
        return score;
    }

    int countMobilitySliding(const Board& b, int sq, const int dirs[][2], int dn, Color us) {
        int f = fileOf(sq), r = rankOf(sq), score = 0;
        for (int i = 0; i < dn; ++i) {
            int ff = f + dirs[i][0], rr = r + dirs[i][1];
            while (ff >= 0 && ff < 8 && rr >= 0 && rr < 8) {
                int p = b.pieceOn(make_sq(ff, rr));
                if (p == NO_PIECE) {
                    score++;
                } else {
                    if (us == WHITE ? isBlack(p) : isWhite(p)) score++;
                    break;
                }
                ff += dirs[i][0]; rr += dirs[i][1];
            }
        }
        return score;
    }

    int pawnStructure(const Board& b) {
        int whiteFiles[8] = {0}, blackFiles[8] = {0};
        for (int sq = 0; sq < 64; ++sq) {
            int p = b.pieceOn(sq);
            if (p == W_PAWN) whiteFiles[fileOf(sq)]++;
            else if (p == B_PAWN) blackFiles[fileOf(sq)]++;
        }

        int score = 0;
        for (int f = 0; f < 8; ++f) {
            if (whiteFiles[f] > 1) score -= 12 * (whiteFiles[f] - 1);
            if (blackFiles[f] > 1) score += 12 * (blackFiles[f] - 1);
        }

        for (int sq = 0; sq < 64; ++sq) {
            int p = b.pieceOn(sq);
            if (p != W_PAWN && p != B_PAWN) continue;
            bool white = p == W_PAWN;
            int f = fileOf(sq), r = rankOf(sq);
            bool leftFriendly = (f > 0) && (white ? whiteFiles[f - 1] > 0 : blackFiles[f - 1] > 0);
            bool rightFriendly = (f < 7) && (white ? whiteFiles[f + 1] > 0 : blackFiles[f + 1] > 0);
            if (!leftFriendly && !rightFriendly) score += white ? -10 : 10;

            bool passed = true;
            if (white) {
                for (int rr = r + 1; rr < 8 && passed; ++rr) {
                    for (int ff = f - 1; ff <= f + 1; ++ff) {
                        if (ff < 0 || ff > 7) continue;
                        if (b.pieceOn(make_sq(ff, rr)) == B_PAWN) { passed = false; break; }
                    }
                }
                if (passed) score += 15 + 8 * r;
            } else {
                for (int rr = r - 1; rr >= 0 && passed; --rr) {
                    for (int ff = f - 1; ff <= f + 1; ++ff) {
                        if (ff < 0 || ff > 7) continue;
                        if (b.pieceOn(make_sq(ff, rr)) == W_PAWN) { passed = false; break; }
                    }
                }
                if (passed) score -= 15 + 8 * (7 - r);
            }
        }
        return score;
    }

    int bishopPair(const Board& b) {
        int wb = popcount(b.piecesBB(W_BISHOP));
        int bb = popcount(b.piecesBB(B_BISHOP));
        int score = 0;
        if (wb >= 2) score += 35;
        if (bb >= 2) score -= 35;
        return score;
    }

    int rookQueenFiles(const Board& b) {
        int score = 0;
        for (int sq = 0; sq < 64; ++sq) {
            int p = b.pieceOn(sq);
            if (p != W_ROOK && p != B_ROOK) continue;
            int f = fileOf(sq);
            bool ownPawn = false, oppPawn = false;
            for (int rr = 0; rr < 8; ++rr) {
                int x = b.pieceOn(make_sq(f, rr));
                if (p == W_ROOK) { if (x == W_PAWN) ownPawn = true; if (x == B_PAWN) oppPawn = true; }
                else { if (x == B_PAWN) ownPawn = true; if (x == W_PAWN) oppPawn = true; }
            }
            if (!ownPawn && !oppPawn) score += (p == W_ROOK ? 25 : -25);
            else if (!ownPawn) score += (p == W_ROOK ? 12 : -12);
        }
        return score;
    }

    int mobility(const Board& b) {
        static const int bdirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
        static const int rdirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        static const int qdirs[8][2] = {{1,1},{1,-1},{-1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}};
        int score = 0;
        for (int sq = 0; sq < 64; ++sq) {
            int p = b.pieceOn(sq);
            if (p == NO_PIECE) continue;
            Color us = isWhite(p) ? WHITE : BLACK;
            int s = 0;
            switch (typeOf(p)) {
                case KNIGHT: s = 4 * countMobilityKnight(b, sq, us); break;
                case BISHOP: s = 3 * countMobilitySliding(b, sq, bdirs, 4, us); break;
                case ROOK: s = 2 * countMobilitySliding(b, sq, rdirs, 4, us); break;
                case QUEEN: s = countMobilitySliding(b, sq, qdirs, 8, us); break;
                default: break;
            }
            score += us == WHITE ? s : -s;
        }
        return score;
    }

    int kingScore(const Board& b, bool endgame) {
        int wk = b.kingSquare(WHITE), bk = b.kingSquare(BLACK);
        if (wk < 0 || bk < 0) return 0;
        if (!endgame) {
            int score = 0;
            if (b.castlingRights & WHITE_OO) score += 12;
            if (b.castlingRights & WHITE_OOO) score += 8;
            if (b.castlingRights & BLACK_OO) score -= 12;
            if (b.castlingRights & BLACK_OOO) score -= 8;
            return score;
        }
        auto distCenter = [](int sq){ return std::abs(fileOf(sq) - 3) + std::abs(rankOf(sq) - 3); };
        return -6 * distCenter(wk) + 6 * distCenter(bk);
    }
}

namespace Evaluation {
    int evaluate(const Board& b) {
        int phase = gamePhase(b);
        bool endgame = phase <= 8;
        int score = 0;

        for (int sq = 0; sq < 64; ++sq) {
            int p = b.pieceOn(sq);
            if (p == NO_PIECE) continue;
            int s = pieceValue[typeOf(p)] + pstFor(p, sq, endgame);
            score += isWhite(p) ? s : -s;
        }

        score += pawnStructure(b);
        score += bishopPair(b);
        score += rookQueenFiles(b);
        score += mobility(b);
        score += kingScore(b, endgame);

        return score;
    }
}
