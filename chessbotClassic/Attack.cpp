#include "Attack.h"

static bool isWhitePiece(char p)
{
    return p >= 'A' && p <= 'Z';
}

static bool isBlackPiece(char p)
{
    return p >= 'a' && p <= 'z';
}

int findKing(const Position &pos, bool white)
{
    const char king = white ? 'K' : 'k';

    for (int i = 0; i < 64; ++i)
    {
        if (pos.b[i] == king)
            return i;
    }

    return -1;
}

bool isSquareAttacked(const Position &pos, int sq, bool by_white)
{
    const int row = sq >> 3;
    const int col = sq & 7;

    // ---------------------------------------------------------------------
    // Pawn attacks
    // ---------------------------------------------------------------------
    if (by_white)
    {
        if (row > 0)
        {
            if (col > 0 && pos.b[sq - 9] == 'P')
                return true;
            if (col < 7 && pos.b[sq - 7] == 'P')
                return true;
        }
    }
    else
    {
        if (row < 7)
        {
            if (col > 0 && pos.b[sq + 7] == 'p')
                return true;
            if (col < 7 && pos.b[sq + 9] == 'p')
                return true;
        }
    }

    // ---------------------------------------------------------------------
    // Knight attacks
    // ---------------------------------------------------------------------
    static const int knightOffsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}};

    for (int i = 0; i < 8; ++i)
    {
        int r = row + knightOffsets[i][1];
        int c = col + knightOffsets[i][0];

        if (r < 0 || r > 7 || c < 0 || c > 7)
            continue;

        int idx = r * 8 + c;
        char p = pos.b[idx];

        if (by_white && p == 'N')
            return true;
        if (!by_white && p == 'n')
            return true;
    }

    // ---------------------------------------------------------------------
    // Sliding pieces (bishop/rook/queen)
    // ---------------------------------------------------------------------
    static const int directions[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},   // rook
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}  // bishop
    };

    for (int d = 0; d < 8; ++d)
    {
        int r = row + directions[d][1];
        int c = col + directions[d][0];

        while (r >= 0 && r < 8 && c >= 0 && c < 8)
        {
            int idx = r * 8 + c;
            char p = pos.b[idx];

            if (p != '.')
            {
                if (by_white)
                {
                    if ((d < 4 && p == 'R') ||
                        (d >= 4 && p == 'B') ||
                        p == 'Q')
                        return true;
                }
                else
                {
                    if ((d < 4 && p == 'r') ||
                        (d >= 4 && p == 'b') ||
                        p == 'q')
                        return true;
                }
                break;
            }

            r += directions[d][1];
            c += directions[d][0];
        }
    }

    // ---------------------------------------------------------------------
    // King attacks (adjacent squares)
    // ---------------------------------------------------------------------
    static const int kingOffsets[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (int i = 0; i < 8; ++i)
    {
        int r = row + kingOffsets[i][1];
        int c = col + kingOffsets[i][0];

        if (r < 0 || r > 7 || c < 0 || c > 7)
            continue;

        int idx = r * 8 + c;
        char p = pos.b[idx];

        if (by_white && p == 'K')
            return true;
        if (!by_white && p == 'k')
            return true;
    }

    return false;
}

bool inCheck(const Position &pos, bool white)
{
    int kingSq = findKing(pos, white);
    if (kingSq == -1)
        return false;

    return isSquareAttacked(pos, kingSq, !white);
}

bool Position::inCheck(bool side) const
{
    return ::inCheck(*this, side);
}