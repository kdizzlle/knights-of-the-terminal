#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H

#include "Move.h"
#include "Position.h"

class GenerateMoves
{
public:
    static Move *genPawn(const Position &pos, Move *out);
    static Move *genKnight(const Position &pos, Move *out);
    static Move *genBishop(const Position &pos, Move *out);
    static Move *genRook(const Position &pos, Move *out);
    static Move *genQueen(const Position &pos, Move *out);
    static Move *genKing(const Position &pos, Move *out);
    static Move *genAllMoves(const Position &pos, Move *out);

private:
    static bool isWhitePiece(char p);
    static bool isBlackPiece(char p);
    static bool isEmpty(char p);

    static Move *addMove(Move *out, int from, int to, char promo = 0);
    static Move *addPromotionMoves(Move *out, int from, int to);
    static Move *genSliding(const Position &pos, Move *out, char piece, const int dirs[][2], int dirCount);
};

#endif