#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H

#include "Move.h"
#include "Position.h"

class GenerateMoves
{
public:
    // Gen Moves
    static void *GenerateMoves::genRook(const Position *pos, int from, bool white, Move *moves, int *n, bool castleflag);
    static void *GenerateMoves::genKnight(const Position *pos, int from, bool white, Move *moves, int *n);
    static void *GenerateMoves::genQueen(const Position *pos, int from, bool white, Move *moves, int *n);
    static void *GenerateMoves::genKing(const Position *pos, int from, bool white, Move *moves, int *n);
    static void *GenerateMoves::genBishop(const Position *pos, int from, bool white, const int dirs[][2], int dcount, Move *moves, int *n);
};

#endif