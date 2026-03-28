#include "MakeMove.h"

#include <cctype>

Position makeMove(const Position &pos, const Move &m)
{
    Position np = pos;

    char piece = np.b[m.from];
    np.b[m.from] = '.';

    char placed = piece;

    if (m.promo != 0 && (piece == 'P' || piece == 'p'))
    {
        if (piece == 'P')
            placed = static_cast<char>(std::toupper(static_cast<unsigned char>(m.promo)));
        else
            placed = static_cast<char>(std::tolower(static_cast<unsigned char>(m.promo)));
    }

    np.b[m.to] = placed;
    np.white_to_move = !pos.white_to_move;

    return np;
}