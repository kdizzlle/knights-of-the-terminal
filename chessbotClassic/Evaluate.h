#ifndef EVALUATE_H
#define EVALUATE_H

#include "Position.h"

/**
 * @class Evaluate
 * @brief Static evaluation for chess positions.
 *
 * Positive scores favor White.
 * Negative scores favor Black.
 *
 * The evaluation includes:
 * - material balance
 * - piece-square tables
 * - simple mobility
 * - basic checkmate/stalemate handling is left to search
 */
class Evaluate
{
public:
    /**
     * @brief Evaluates a position from White's perspective.
     *
     * @param pos Position to evaluate.
     * @return Positive if White is better, negative if Black is better.
     */
    static int eval(const Position &pos);

private:
    /**
     * @brief Returns the material value of a piece.
     */
    static int pieceValue(char p);

    /**
     * @brief Returns the piece-square value for a piece on a square.
     */
    static int pieceSquareValue(char p, int sq);

    /**
     * @brief Mirrors a square vertically for black-piece table lookup.
     *
     * Example:
     * - a1 (0) <-> a8 (56)
     * - e2 (12) <-> e7 (52)
     */
    static int mirrorSquare(int sq);
};

#endif