#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H

#include "Move.h"
#include "Position.h"

/**
 * @class GenerateMoves
 * @brief Generates pseudo-legal chess moves for the side to move.
 *
 * These methods generate pseudo-legal moves only. They do not remove moves
 * that leave the king in check. Legal filtering should still be done by
 * making the move and checking king safety.
 *
 * This version supports:
 * - normal moves
 * - captures
 * - promotions
 * - castling
 *
 * This version does NOT support en passant.
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
     * @brief Generates all pseudo-legal king moves for the side to move,
     *        including castling when available.
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
    /**
     * @brief Piece helpers.
     */
    static bool isWhitePiece(char p);
    static bool isBlackPiece(char p);
    static bool isEmpty(char p);

    /**
     * @brief Appends a move to the output buffer.
     */
    static Move *addMove(Move *out, int from, int to, char promo = 0);

    /**
     * @brief Appends all four promotion choices to the output buffer.
     */
    static Move *addPromotionMoves(Move *out, int from, int to);

    /**
     * @brief Shared helper for bishop/rook/queen sliding moves.
     */
    static Move *genSliding(const Position &pos, Move *out, char piece, const int dirs[][2], int dirCount);

    /**
     * @brief Adds kingside castling if available and pseudo-legally valid.
     *
     * This only checks board occupancy and castling rights here.
     * Final legality is still confirmed by normal legal move filtering.
     */
    static Move *addCastleKingSide(const Position &pos, Move *out);

    /**
     * @brief Adds queenside castling if available and pseudo-legally valid.
     *
     * This only checks board occupancy and castling rights here.
     * Final legality is still confirmed by normal legal move filtering.
     */
    static Move *addCastleQueenSide(const Position &pos, Move *out);
};

#endif