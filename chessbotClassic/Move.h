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
    int from;
    int to;
    char promo;

    Move() : from(0), to(0), promo(0) {}
    Move(int f, int t, char p = 0) : from(f), to(t), promo(p) {}

    static int squareIndex(const char *sq)
    {
        int file = sq[0] - 'a';
        int rank = sq[1] - '1';
        return rank * 8 + file;
    }

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

    static void indexToSq(int idx, char out[3])
    {
        out[0] = 'a' + (idx % 8);
        out[1] = '1' + (idx / 8);
        out[2] = '\0';
    }

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

    bool isNull() const
    {
        return from == 0 && to == 0 && promo == 0;
    }

    bool operator==(const Move &other) const
    {
        return from == other.from && to == other.to && promo == other.promo;
    }

    bool operator!=(const Move &other) const
    {
        return !(*this == other);
    }
};

#endif