#ifndef MOVE_H
#define MOVE_H

#include <cstring>

/**
 * @struct Move
 * @brief Represents a chess move using source square, destination square,
 *        and optional promotion piece.
 *
 * Squares are stored as 0-63 board indices, where:
 * - 0 = a1
 * - 7 = h1
 * - 56 = a8
 * - 63 = h8
 *
 * The promo field is 0 for a normal move, or one of:
 * - 'q' for queen promotion
 * - 'r' for rook promotion
 * - 'b' for bishop promotion
 * - 'n' for knight promotion
 */
struct Move
{
    /** @brief Source square index (0-63). */
    int from;

    /** @brief Destination square index (0-63). */
    int to;

    /**
     * @brief Promotion piece character.
     *
     * Value is 0 if the move is not a promotion. Otherwise it should be one
     * of 'q', 'r', 'b', or 'n'.
     */
    char promo;

    /**
     * @brief Constructs a default move.
     */
    Move() : from(0), to(0), promo(0) {}

    /**
     * @brief Constructs a move with source, destination, and optional promotion.
     *
     * @param f Source square index.
     * @param t Destination square index.
     * @param p Promotion piece character, or 0 if not a promotion.
     */
    Move(int f, int t, char p = 0) : from(f), to(t), promo(p) {}

    /**
     * @brief Converts an algebraic square string to a board index.
     *
     * Example:
     * - "a1" -> 0
     * - "e2" -> 12
     * - "h8" -> 63
     *
     * @param sq Null-terminated 2-character square string such as "e2".
     * @return Board index in the range 0-63.
     */
    static int squareIndex(const char *sq)
    {
        int file = sq[0] - 'a';
        int rank = sq[1] - '1';
        return rank * 8 + file;
    }

    /**
     * @brief Parses a move from a UCI string.
     *
     * Accepted examples:
     * - "e2e4"
     * - "e7e8q"
     * - "a7a8n"
     *
     * @param s Null-terminated UCI move string.
     * @return Parsed Move object, or (0,0,0) if input is invalid.
     */
    static Move fromUci(const char *s)
    {
        if (!s || std::strlen(s) < 4)
            return Move(0, 0, 0);

        char fromSq[3] = {s[0], s[1], '\0'};
        char toSq[3] = {s[2], s[3], '\0'};
        int from = squareIndex(fromSq);
        int to = squareIndex(toSq);
        char promo = (std::strlen(s) >= 5) ? s[4] : 0;
        return Move(from, to, promo);
    }

    /**
     * @brief Converts a board index to a square string.
     *
     * @param idx Board index in the range 0-63.
     * @param out Output buffer of size at least 3.
     */
    static void indexToSq(int idx, char out[3])
    {
        out[0] = 'a' + (idx % 8);
        out[1] = '1' + (idx / 8);
        out[2] = '\0';
    }

    /**
     * @brief Converts this move to a UCI string.
     *
     * Examples:
     * - normal move: "e2e4"
     * - promotion:   "e7e8q"
     *
     * @param out Output buffer of size at least 6.
     */
    void toUci(char out[6]) const
    {
        char f[3], t[3];
        indexToSq(from, f);
        indexToSq(to, t);

        out[0] = f[0];
        out[1] = f[1];
        out[2] = t[0];
        out[3] = t[1];

        if (promo)
        {
            out[4] = promo;
            out[5] = '\0';
        }
        else
        {
            out[4] = '\0';
        }
    }
};

#endif