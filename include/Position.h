#ifndef POSITION_H
#define POSITION_H

#include <cctype>

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
 * Piece encoding is typically:
 * - White pieces: uppercase letters ('P', 'N', 'B', 'R', 'Q', 'K')
 * - Black pieces: lowercase letters ('p', 'n', 'b', 'r', 'q', 'k')
 * - Empty square: '.' or 0, depending on the rest of the engine
 *
 * This minimal version stores only:
 * - which side is to move
 * - the current board contents
 *
 * More complete engines often also store:
 * - castling rights
 * - en passant target square
 * - halfmove clock
 * - fullmove number
 * - king square locations
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
     *
     * Each entry represents one square on the board using a piece character
     * or an empty-square marker.
     */
    char b[64];

    /**
     * @brief Checks whether the given side is currently in check.
     *
     * This is a placeholder implementation and should be replaced with
     * real king-attack detection logic.
     *
     * @param side Side to test for check.
     *             - true  = White
     *             - false = Black
     * @return True if the specified side is in check, otherwise false.
     */
    bool inCheck(bool side) const
    {
        // replace with your real logic
        return false;
    }
};

#endif