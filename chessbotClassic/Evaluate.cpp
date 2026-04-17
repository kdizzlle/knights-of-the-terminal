#include "Evaluate.h"

namespace
{
    // -------------------------------------------------------------------------
    // Piece-square tables
    // Indexed from White's perspective:
    // a1 = 0, b1 = 1, ..., h8 = 63
    // Black pieces use mirrored lookup.
    // -------------------------------------------------------------------------

    const int PAWN_TABLE[64] = {
         0,   0,   0,   0,   0,   0,   0,   0,
         5,  10,  10, -20, -20,  10,  10,   5,
         5,  -5, -10,   0,   0, -10,  -5,   5,
         0,   0,   0,  20,  20,   0,   0,   0,
         5,   5,  10,  25,  25,  10,   5,   5,
        10,  10,  20,  30,  30,  20,  10,  10,
        50,  50,  50,  50,  50,  50,  50,  50,
         0,   0,   0,   0,   0,   0,   0,   0
    };

    const int KNIGHT_TABLE[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   5,   5,   0, -20, -40,
        -30,   5,  10,  15,  15,  10,   5, -30,
        -30,   0,  15,  20,  20,  15,   0, -30,
        -30,   5,  15,  20,  20,  15,   5, -30,
        -30,   0,  10,  15,  15,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    };

    const int BISHOP_TABLE[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    const int ROOK_TABLE[64] = {
         0,   0,   5,  10,  10,   5,   0,   0,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
         5,  10,  10,  10,  10,  10,  10,   5,
         0,   0,   5,  10,  10,   5,   0,   0
    };

    const int QUEEN_TABLE[64] = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,   5,   5,   5,   0, -10,
         -5,   0,   5,   5,   5,   5,   0,  -5,
          0,   0,   5,   5,   5,   5,   0,  -5,
        -10,   5,   5,   5,   5,   5,   0, -10,
        -10,   0,   5,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
    };

    const int KING_TABLE[64] = {
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
         20,  20,   0,   0,   0,   0,  20,  20,
         20,  30,  10,   0,   0,  10,  30,  20
    };
}

int Evaluate::mirrorSquare(int sq)
{
    return ((7 - (sq >> 3)) << 3) | (sq & 7);
}

int Evaluate::pieceValue(char p)
{
    switch (p)
    {
    case 'P':
    case 'p':
        return 100;
    case 'N':
    case 'n':
        return 320;
    case 'B':
    case 'b':
        return 330;
    case 'R':
    case 'r':
        return 500;
    case 'Q':
    case 'q':
        return 900;
    case 'K':
    case 'k':
        return 0;
    default:
        return 0;
    }
}

int Evaluate::pieceSquareValue(char p, int sq)
{
    const bool white = (p >= 'A' && p <= 'Z');
    const int tableSq = white ? sq : mirrorSquare(sq);

    switch (p)
    {
    case 'P':
    case 'p':
        return PAWN_TABLE[tableSq];
    case 'N':
    case 'n':
        return KNIGHT_TABLE[tableSq];
    case 'B':
    case 'b':
        return BISHOP_TABLE[tableSq];
    case 'R':
    case 'r':
        return ROOK_TABLE[tableSq];
    case 'Q':
    case 'q':
        return QUEEN_TABLE[tableSq];
    case 'K':
    case 'k':
        return KING_TABLE[tableSq];
    default:
        return 0;
    }
}

int Evaluate::eval(const Position &pos)
{
    int score = 0;

    for (int sq = 0; sq < 64; ++sq)
    {
        const char p = pos.b[sq];
        if (p == '.' || p == 0)
            continue;

        const int material = pieceValue(p);
        const int pst = pieceSquareValue(p, sq);
        const int total = material + pst;

        if (p >= 'A' && p <= 'Z')
            score += total;
        else
            score -= total;
    }

    // Small castling-right bonus for king safety / development potential
    if (pos.white_can_castle_king)
        score += 15;
    if (pos.white_can_castle_queen)
        score += 10;
    if (pos.black_can_castle_king)
        score -= 15;
    if (pos.black_can_castle_queen)
        score -= 10;

    return score;
}