#ifndef POSITION_H
#define POSITION_H

#include <cctype>

struct Position
{
    bool white_to_move;
    char b[64];

    bool inCheck(bool side) const
    {
        // replace with your real logic
        return false;
    }
};

#endif