#ifndef MOVE_H
#define MOVE_H

#include <cstdint>
#include <cstring>

struct Move {
    uint16_t data;

    enum Flag : uint16_t {
        QUIET = 0,
        DOUBLE_PAWN = 1,
        KING_CASTLE = 2,
        QUEEN_CASTLE = 3,
        CAPTURE = 4,
        EN_PASSANT = 5,
        PROMO_N = 8,
        PROMO_B = 9,
        PROMO_R = 10,
        PROMO_Q = 11,
        CAP_PROMO_N = 12,
        CAP_PROMO_B = 13,
        CAP_PROMO_R = 14,
        CAP_PROMO_Q = 15
    };

    Move() : data(0) {}
    Move(int from, int to, Flag flag = QUIET) : data((from & 63) | ((to & 63) << 6) | ((flag & 15) << 12)) {}

    int from() const { return data & 63; }
    int to() const { return (data >> 6) & 63; }
    int flag() const { return (data >> 12) & 15; }

    bool isNull() const { return data == 0; }
    bool isCapture() const { return flag() == CAPTURE || flag() == EN_PASSANT || flag() >= CAP_PROMO_N; }
    bool isPromotion() const { return flag() >= PROMO_N; }
    bool isCastle() const { return flag() == KING_CASTLE || flag() == QUEEN_CASTLE; }

    static Move fromUci(const char* s) {
        if (!s || std::strlen(s) < 4) return Move();
        int ff = s[0] - 'a', fr = s[1] - '1';
        int tf = s[2] - 'a', tr = s[3] - '1';
        int from = fr * 8 + ff, to = tr * 8 + tf;
        if (std::strlen(s) < 5) return Move(from, to, QUIET);
        switch (s[4]) {
            case 'n': return Move(from, to, PROMO_N);
            case 'b': return Move(from, to, PROMO_B);
            case 'r': return Move(from, to, PROMO_R);
            case 'q': return Move(from, to, PROMO_Q);
            default: return Move(from, to, QUIET);
        }
    }

    void toUci(char out[6]) const {
        out[0] = 'a' + (from() & 7);
        out[1] = '1' + (from() >> 3);
        out[2] = 'a' + (to() & 7);
        out[3] = '1' + (to() >> 3);
        if (isPromotion()) {
            switch (flag() & 3) {
                case 0: out[4] = 'n'; break;
                case 1: out[4] = 'b'; break;
                case 2: out[4] = 'r'; break;
                default: out[4] = 'q'; break;
            }
            out[5] = '\0';
        } else {
            out[4] = '\0';
        }
    }

    bool operator==(const Move& other) const { return data == other.data; }
    bool operator!=(const Move& other) const { return data != other.data; }
};

#endif
