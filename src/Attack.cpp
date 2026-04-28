#include "Attack.h"

#include <cctype>
#include <cstdlib>

static bool isWhitePiece(char p)
{
    return p >= 'A' && p <= 'Z';
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
    const int r = sq / 8;
    const int f = sq % 8;

    if (by_white)
    {
        if (r > 0 && f > 0 && pos.b[(r - 1) * 8 + (f - 1)] == 'P')
            return true;
        if (r > 0 && f < 7 && pos.b[(r - 1) * 8 + (f + 1)] == 'P')
            return true;
    }
    else
    {
        if (r < 7 && f > 0 && pos.b[(r + 1) * 8 + (f - 1)] == 'p')
            return true;
        if (r < 7 && f < 7 && pos.b[(r + 1) * 8 + (f + 1)] == 'p')
            return true;
    }

    static const int knightOffsets[8] = {-17, -15, -10, -6, 6, 10, 15, 17};

    for (int i = 0; i < 8; ++i)
    {
        const int to = sq + knightOffsets[i];
        if (to < 0 || to >= 64)
            continue;

        const int tr = to / 8;
        const int tf = to % 8;
        const int dr = std::abs(tr - r);
        const int df = std::abs(tf - f);

        if (!((dr == 1 && df == 2) || (dr == 2 && df == 1)))
            continue;

        const char pc = pos.b[to];
        if (by_white && pc == 'N')
            return true;
        if (!by_white && pc == 'n')
            return true;
    }

    static const int dirs[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (int di = 0; di < 8; ++di)
    {
        const int df = dirs[di][0];
        const int dr = dirs[di][1];

        int cr = r + dr;
        int cf = f + df;
        int distance = 1;

        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8)
        {
            const int idx = cr * 8 + cf;
            const char pc = pos.b[idx];

            if (pc != '.')
            {
                if (isWhitePiece(pc) == by_white)
                {
                    const char up = static_cast<char>(std::toupper(static_cast<unsigned char>(pc)));
                    const bool rookDir = (di < 4);
                    const bool bishopDir = (di >= 4);

                    if (up == 'Q')
                        return true;
                    if (rookDir && up == 'R')
                        return true;
                    if (bishopDir && up == 'B')
                        return true;
                    if (distance == 1 && up == 'K')
                        return true;
                }
                break;
            }

            cr += dr;
            cf += df;
            ++distance;
        }
    }

    return false;
}

bool inCheck(const Position &pos, bool white)
{
    const int kingSq = findKing(pos, white);
    if (kingSq < 0)
        return false;

    return isSquareAttacked(pos, kingSq, !white);
}

bool Position::inCheck(bool side) const
{
    return ::inCheck(*this, side);
}