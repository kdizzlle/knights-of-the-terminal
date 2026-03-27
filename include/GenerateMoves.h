#ifndef GENERATEMOVES_H
#define GENERATEMOVES_H

#include "Move.h"
#include "Position.h"

/**
 * @class GenerateMoves
 * @brief Generates pseudo-legal chess moves for the side to move.
 *
 * This class contains helper functions for generating moves for each piece
 * type. The generated moves are written sequentially into a caller-provided
 * move buffer, and each method returns a pointer to the next free slot.
 *
 * These methods generate pseudo-legal moves only. They do not remove moves
 * that leave the king in check.
 */
class GenerateMoves
{
public:
    /**
     * @brief Generates all pseudo-legal pawn moves for the side to move.
     *
     * Includes:
     * - single pawn pushes
     * - double pawn pushes from the starting rank
     * - diagonal captures
     * - promotion pushes
     * - promotion captures
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genPawn(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal knight moves for the side to move.
     *
     * Handles all 8 knight jump offsets and includes captures of enemy pieces.
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genKnight(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal bishop moves for the side to move.
     *
     * Bishops move diagonally until blocked by the board edge, a friendly
     * piece, or after capturing an enemy piece.
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genBishop(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal rook moves for the side to move.
     *
     * Rooks move horizontally and vertically until blocked by the board edge,
     * a friendly piece, or after capturing an enemy piece.
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genRook(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal queen moves for the side to move.
     *
     * Queens combine rook-like and bishop-like movement.
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genQueen(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal king moves for the side to move.
     *
     * Generates one-square moves in all 8 directions. Castling is not included
     * unless added separately.
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genKing(const Position &pos, Move *out);

    /**
     * @brief Generates all pseudo-legal moves for the side to move.
     *
     * Calls the individual move generators for pawns, knights, bishops,
     * rooks, queens, and kings.
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genAllMoves(const Position &pos, Move *out);

private:
    /**
     * @brief Checks whether a piece character represents a white piece.
     *
     * White pieces are stored as uppercase letters.
     *
     * @param p Piece character from the board.
     * @return True if the piece is white, otherwise false.
     */
    static bool isWhitePiece(char p);

    /**
     * @brief Checks whether a piece character represents a black piece.
     *
     * Black pieces are stored as lowercase letters.
     *
     * @param p Piece character from the board.
     * @return True if the piece is black, otherwise false.
     */
    static bool isBlackPiece(char p);

    /**
     * @brief Checks whether a board square is empty.
     *
     * This helper treats '.' and 0 as empty squares.
     *
     * @param p Board square contents.
     * @return True if the square is empty, otherwise false.
     */
    static bool isEmpty(char p);

    /**
     * @brief Adds a single move to the move buffer.
     *
     * Writes one move at the current output pointer and advances the pointer.
     *
     * @param out Current write position in the move buffer.
     * @param from Source square index.
     * @param to Destination square index.
     * @param promo Promotion piece character, or 0 if not a promotion.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *addMove(Move *out, int from, int to, char promo = 0);

    /**
     * @brief Adds all four promotion choices for a pawn move.
     *
     * The generated promotion moves are queen, rook, bishop, and knight.
     *
     * @param out Current write position in the move buffer.
     * @param from Source square index.
     * @param to Destination square index.
     * @return Pointer to the next free slot after the four promotion moves.
     */
    static Move *addPromotionMoves(Move *out, int from, int to);

    /**
     * @brief Generates moves for a sliding piece.
     *
     * This helper is used by bishops, rooks, and queens. It steps through
     * each provided direction until the piece is blocked by the board edge,
     * a friendly piece, or an enemy piece that can be captured.
     *
     * @param pos Current board position.
     * @param out Pointer to the move buffer where generated moves are written.
     * @param piece Piece character to generate moves for.
     * @param dirs Array of direction vectors in {fileDelta, rankDelta} format.
     * @param dirCount Number of direction vectors in dirs.
     * @return Pointer to the next free slot in the move buffer.
     */
    static Move *genSliding(const Position &pos, Move *out, char piece, const int dirs[][2], int dirCount);
};

#endif