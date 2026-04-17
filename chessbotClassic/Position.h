#ifndef POSITION_H
#define POSITION_H

/**
 * @struct Position
 * @brief Represents a chess position using side-to-move, board state,
 *        castling rights, and move counters.
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
     * @brief White can castle kingside.
     */
    bool white_can_castle_king;

    /**
     * @brief White can castle queenside.
     */
    bool white_can_castle_queen;

    /**
     * @brief Black can castle kingside.
     */
    bool black_can_castle_king;

    /**
     * @brief Black can castle queenside.
     */
    bool black_can_castle_queen;

    /**
     * @brief Halfmove clock (for completeness, even if unused).
     */
    int halfmove_clock;

    /**
     * @brief Fullmove number.
     */
    int fullmove_number;

    /**
     * @brief Creates a default empty position.
     */
    Position()
        : white_to_move(true),
          white_can_castle_king(false),
          white_can_castle_queen(false),
          black_can_castle_king(false),
          black_can_castle_queen(false),
          halfmove_clock(0),
          fullmove_number(1)
    {
        for (int i = 0; i < 64; ++i)
            b[i] = '.';
    }

    /**
     * @brief Checks whether the given side is currently in check.
     */
    bool inCheck(bool side) const;

    /**
     * @brief Utility helpers
     */
    static bool isValidSquare(int sq)
    {
        return sq >= 0 && sq < 64;
    }

    static int fileOf(int sq)
    {
        return sq & 7;
    }

    static int rankOf(int sq)
    {
        return sq >> 3;
    }

    static bool isWhitePiece(char p)
    {
        return p >= 'A' && p <= 'Z';
    }

    static bool isBlackPiece(char p)
    {
        return p >= 'a' && p <= 'z';
    }

    static bool isPiece(char p)
    {
        return isWhitePiece(p) || isBlackPiece(p);
    }

    static bool isEmpty(char p)
    {
        return p == '.' || p == 0;
    }
};

#endif