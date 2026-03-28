#ifndef POSITION_H
#define POSITION_H

/**
 * @struct Position
 * @brief Represents a chess position using side-to-move and a 64-square board.
 *
 * The board is stored as a flat array of 64 characters:
 * - index 0  = a1
 * - index 7  = h1
 * - index 56 = a8
 * - index 63 = h8
 *
 * Piece encoding:
 * - White pieces: uppercase letters ('P', 'N', 'B', 'R', 'Q', 'K')
 * - Black pieces: lowercase letters ('p', 'n', 'b', 'r', 'q', 'k')
 * - Empty square: '.'
 */
struct Position
{
    /**
     * @brief Indicates which side is to move.
     *
     * - true  = White to move
     * - false = Black to move
     */
    bool white_to_move;

    /**
     * @brief Board contents as a 64-element array.
     */
    char b[64];

    /**
     * @brief Checks whether the given side is currently in check.
     *
     * @param side Side to test.
     *             - true  = White
     *             - false = Black
     * @return True if that side's king is under attack, otherwise false.
     */
    bool inCheck(bool side) const;
};

#endif