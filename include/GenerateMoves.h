#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H

#include "Move.h"
#include "Position.h"

/**
 * @class GenerateMoves
 * @brief Generates pseudo-legal chess moves for the side to move.
 *
 * These methods generate pseudo-legal moves only. They do not remove moves
 * that leave the king in check.
 */
class GenerateMoves
{
public:
    /**
     * @brief Generates all pseudo-legal pawn moves for the side to move.
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genPawn(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal knight moves for the side to move.
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genKnight(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal bishop moves for the side to move.
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genBishop(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal rook moves for the side to move.
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genRook(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal queen moves for the side to move.
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genQueen(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal king moves for the side to move.
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genKing(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal moves for the side to move.
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
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