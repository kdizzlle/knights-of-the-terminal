#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

enum Color : int { WHITE = 0, BLACK = 1, COLOR_NB = 2 };
enum PieceType : int { PAWN = 0, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_NB, NO_PIECE_TYPE = -1 };
enum Piece : int {
    W_PAWN = 0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB, NO_PIECE = -1
};

enum CastlingRights : int {
    WHITE_OO = 1,
    WHITE_OOO = 2,
    BLACK_OO = 4,
    BLACK_OOO = 8
};

inline Color operator!(Color c) { return c == WHITE ? BLACK : WHITE; }
inline int sq_file(int sq) { return sq & 7; }
inline int sq_rank(int sq) { return sq >> 3; }
inline int make_sq(int f, int r) { return (r << 3) | f; }
inline uint64_t bb_of(int sq) { return 1ULL << sq; }

#endif
