#include "MoveGenerator.h"
#include "Precomputed.h"
#include "Bitboard.h"

namespace {
    bool sameColorPiece(int p, Color c) { return p != NO_PIECE && ((p < 6) == (c == WHITE)); }
    PieceType typeOf(int p) { return p == NO_PIECE ? NO_PIECE_TYPE : static_cast<PieceType>(p % 6); }
    int makePiece(Color c, PieceType pt) { return (c == WHITE ? 0 : 6) + pt; }
    bool inb(int f,int r){return f>=0&&f<8&&r>=0&&r<8;}

    template<bool TacticalOnly>
    void gen(Board& b, std::vector<Move>& out) {
        out.clear();
        Color us = b.sideToMove();
        Color them = !us;
        uint64_t occUs = b.occupancy(us);
        uint64_t occThem = b.occupancy(them);
        uint64_t occ = b.occupancy();

        auto pushIfLegal = [&](Move m) {
            UndoState u;
            if (b.makeMove(m, u)) { out.push_back(m); b.unmakeMove(m, u); }
        };

        // Pawns
        uint64_t pawns = b.piecesBB(static_cast<Piece>(makePiece(us, PAWN)));
        while (pawns) {
            int sq = pop_lsb(pawns);
            int f = sq_file(sq), r = sq_rank(sq);
            int dir = us == WHITE ? 1 : -1;
            int startRank = us == WHITE ? 1 : 6;
            int promoRank = us == WHITE ? 6 : 1;
            int one = sq + 8 * dir;
            if constexpr (!TacticalOnly) {
                if (one >= 0 && one < 64 && b.pieceOn(one) == NO_PIECE) {
                    if (r == promoRank) {
                        pushIfLegal(Move(sq, one, Move::PROMO_N));
                        pushIfLegal(Move(sq, one, Move::PROMO_B));
                        pushIfLegal(Move(sq, one, Move::PROMO_R));
                        pushIfLegal(Move(sq, one, Move::PROMO_Q));
                    } else {
                        pushIfLegal(Move(sq, one));
                        int two = sq + 16 * dir;
                        if (r == startRank && b.pieceOn(two) == NO_PIECE) pushIfLegal(Move(sq, two, Move::DOUBLE_PAWN));
                    }
                }
            }
            for (int df : {-1, 1}) {
                int tf = f + df, tr = r + dir;
                if (!inb(tf, tr)) continue;
                int to = make_sq(tf, tr);
                if (b.pieceOn(to) != NO_PIECE && sameColorPiece(b.pieceOn(to), them)) {
                    if (r == promoRank) {
                        pushIfLegal(Move(sq, to, Move::CAP_PROMO_N));
                        pushIfLegal(Move(sq, to, Move::CAP_PROMO_B));
                        pushIfLegal(Move(sq, to, Move::CAP_PROMO_R));
                        pushIfLegal(Move(sq, to, Move::CAP_PROMO_Q));
                    } else pushIfLegal(Move(sq, to, Move::CAPTURE));
                }
                if (to == b.epSquare) pushIfLegal(Move(sq, to, Move::EN_PASSANT));
            }
        }

        // Knights
        uint64_t knights = b.piecesBB(static_cast<Piece>(makePiece(us, KNIGHT)));
        while (knights) {
            int sq = pop_lsb(knights);
            uint64_t att = Precomputed::knightAttacks[sq] & ~occUs;
            while (att) {
                int to = pop_lsb(att);
                bool cap = occThem & bb_of(to);
                if constexpr (TacticalOnly) { if (!cap) continue; }
                pushIfLegal(Move(sq, to, cap ? Move::CAPTURE : Move::QUIET));
            }
        }

        // Sliders via magic bitboards
        for (PieceType pt : {BISHOP, ROOK, QUEEN}) {
            uint64_t bb = b.piecesBB(static_cast<Piece>(makePiece(us, pt)));
            while (bb) {
                int sq = pop_lsb(bb);
                uint64_t attacks = 0ULL;
                if (pt == BISHOP) attacks = Precomputed::bishopAttacks(sq, occ);
                else if (pt == ROOK) attacks = Precomputed::rookAttacks(sq, occ);
                else attacks = Precomputed::queenAttacks(sq, occ);
                attacks &= ~occUs;

                while (attacks) {
                    int to = pop_lsb(attacks);
                    bool cap = (occThem & bb_of(to)) != 0ULL;
                    if constexpr (TacticalOnly) { if (!cap) continue; }
                    pushIfLegal(Move(sq, to, cap ? Move::CAPTURE : Move::QUIET));
                }
            }
        }

        // King
        int ksq = b.kingSquare(us);
        uint64_t katt = Precomputed::kingAttacks[ksq] & ~occUs;
        while (katt) {
            int to = pop_lsb(katt);
            bool cap = occThem & bb_of(to);
            if constexpr (TacticalOnly) { if (!cap) continue; }
            pushIfLegal(Move(ksq, to, cap ? Move::CAPTURE : Move::QUIET));
        }
        if constexpr (!TacticalOnly) {
            if (us == WHITE) {
                if ((b.castlingRights & WHITE_OO) && b.pieceOn(5)==NO_PIECE && b.pieceOn(6)==NO_PIECE && !b.inCheck(WHITE) && !b.isSquareAttacked(5,BLACK) && !b.isSquareAttacked(6,BLACK)) pushIfLegal(Move(4,6,Move::KING_CASTLE));
                if ((b.castlingRights & WHITE_OOO) && b.pieceOn(1)==NO_PIECE && b.pieceOn(2)==NO_PIECE && b.pieceOn(3)==NO_PIECE && !b.inCheck(WHITE) && !b.isSquareAttacked(3,BLACK) && !b.isSquareAttacked(2,BLACK)) pushIfLegal(Move(4,2,Move::QUEEN_CASTLE));
            } else {
                if ((b.castlingRights & BLACK_OO) && b.pieceOn(61)==NO_PIECE && b.pieceOn(62)==NO_PIECE && !b.inCheck(BLACK) && !b.isSquareAttacked(61,WHITE) && !b.isSquareAttacked(62,WHITE)) pushIfLegal(Move(60,62,Move::KING_CASTLE));
                if ((b.castlingRights & BLACK_OOO) && b.pieceOn(57)==NO_PIECE && b.pieceOn(58)==NO_PIECE && b.pieceOn(59)==NO_PIECE && !b.inCheck(BLACK) && !b.isSquareAttacked(59,WHITE) && !b.isSquareAttacked(58,WHITE)) pushIfLegal(Move(60,58,Move::QUEEN_CASTLE));
            }
        }
    }
}

namespace MoveGenerator {
    void generateLegalMoves(Board& b, std::vector<Move>& out) { gen<false>(b, out); }
    void generateTacticalMoves(Board& b, std::vector<Move>& out) { gen<true>(b, out); }
}
