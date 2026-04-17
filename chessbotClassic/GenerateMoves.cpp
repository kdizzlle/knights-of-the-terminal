#include "GenerateMoves.h"
#include "Attack.h"

bool GenerateMoves::isWhitePiece(char p)
{
    return p >= 'A' && p <= 'Z';
}

bool GenerateMoves::isBlackPiece(char p)
{
    return p >= 'a' && p <= 'z';
}

bool GenerateMoves::isEmpty(char p)
{
    return p == '.' || p == 0;
}

Move *GenerateMoves::addMove(Move *out, int from, int to, char promo)
{
    *out++ = Move(from, to, promo);
    return out;
}

Move *GenerateMoves::addPromotionMoves(Move *out, int from, int to)
{
    *out++ = Move(from, to, 'q');
    *out++ = Move(from, to, 'r');
    *out++ = Move(from, to, 'b');
    *out++ = Move(from, to, 'n');
    return out;
}

Move *GenerateMoves::genPawn(const Position &pos, Move *out)
{
    const bool white = pos.white_to_move;

    for (int from = 0; from < 64; ++from)
    {
        const char piece = pos.b[from];

        if (white)
        {
            if (piece != 'P')
                continue;

            const int file = from & 7;
            const int rank = from >> 3;
            const int oneStep = from + 8;
            const int twoStep = from + 16;

            if (rank < 7 && isEmpty(pos.b[oneStep]))
            {
                if (rank == 6)
                    out = addPromotionMoves(out, from, oneStep);
                else
                {
                    out = addMove(out, from, oneStep);

                    if (rank == 1 && isEmpty(pos.b[twoStep]))
                        out = addMove(out, from, twoStep);
                }
            }

            if (file > 0 && rank < 7)
            {
                const int to = from + 7;
                if (isBlackPiece(pos.b[to]))
                {
                    if (rank == 6)
                        out = addPromotionMoves(out, from, to);
                    else
                        out = addMove(out, from, to);
                }
            }

            if (file < 7 && rank < 7)
            {
                const int to = from + 9;
                if (isBlackPiece(pos.b[to]))
                {
                    if (rank == 6)
                        out = addPromotionMoves(out, from, to);
                    else
                        out = addMove(out, from, to);
                }
            }
        }
        else
        {
            if (piece != 'p')
                continue;

            const int file = from & 7;
            const int rank = from >> 3;
            const int oneStep = from - 8;
            const int twoStep = from - 16;

            if (rank > 0 && isEmpty(pos.b[oneStep]))
            {
                if (rank == 1)
                    out = addPromotionMoves(out, from, oneStep);
                else
                {
                    out = addMove(out, from, oneStep);

                    if (rank == 6 && isEmpty(pos.b[twoStep]))
                        out = addMove(out, from, twoStep);
                }
            }

            if (file > 0 && rank > 0)
            {
                const int to = from - 9;
                if (isWhitePiece(pos.b[to]))
                {
                    if (rank == 1)
                        out = addPromotionMoves(out, from, to);
                    else
                        out = addMove(out, from, to);
                }
            }

            if (file < 7 && rank > 0)
            {
                const int to = from - 7;
                if (isWhitePiece(pos.b[to]))
                {
                    if (rank == 1)
                        out = addPromotionMoves(out, from, to);
                    else
                        out = addMove(out, from, to);
                }
            }
        }
    }

    return out;
}

Move *GenerateMoves::genKnight(const Position &pos, Move *out)
{
    static const int offsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}};

    const bool white = pos.white_to_move;
    const char knight = white ? 'N' : 'n';

    for (int from = 0; from < 64; ++from)
    {
        if (pos.b[from] != knight)
            continue;

        const int row = from >> 3;
        const int col = from & 7;

        for (int i = 0; i < 8; ++i)
        {
            const int toRow = row + offsets[i][1];
            const int toCol = col + offsets[i][0];

            if (toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7)
                continue;

            const int to = toRow * 8 + toCol;
            const char target = pos.b[to];

            if (!isEmpty(target) && isWhitePiece(target) == white)
                continue;

            out = addMove(out, from, to);
        }
    }

    return out;
}

Move *GenerateMoves::genSliding(const Position &pos, Move *out, char piece,
                                const int dirs[][2], int dirCount)
{
    const bool white = pos.white_to_move;

    for (int from = 0; from < 64; ++from)
    {
        if (pos.b[from] != piece)
            continue;

        const int row = from >> 3;
        const int col = from & 7;

        for (int d = 0; d < dirCount; ++d)
        {
            int r = row + dirs[d][1];
            int c = col + dirs[d][0];

            while (r >= 0 && r < 8 && c >= 0 && c < 8)
            {
                const int to = r * 8 + c;
                const char target = pos.b[to];

                if (isEmpty(target))
                {
                    out = addMove(out, from, to);
                }
                else
                {
                    if (isWhitePiece(target) != white)
                        out = addMove(out, from, to);
                    break;
                }

                r += dirs[d][1];
                c += dirs[d][0];
            }
        }
    }

    return out;
}

Move *GenerateMoves::genBishop(const Position &pos, Move *out)
{
    static const int dirs[4][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    return genSliding(pos, out, pos.white_to_move ? 'B' : 'b', dirs, 4);
}

Move *GenerateMoves::genRook(const Position &pos, Move *out)
{
    static const int dirs[4][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    return genSliding(pos, out, pos.white_to_move ? 'R' : 'r', dirs, 4);
}

Move *GenerateMoves::genQueen(const Position &pos, Move *out)
{
    static const int dirs[8][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    return genSliding(pos, out, pos.white_to_move ? 'Q' : 'q', dirs, 8);
}

Move *GenerateMoves::addCastleKingSide(const Position &pos, Move *out)
{
    if (pos.white_to_move)
    {
        // White: e1 -> g1, rook h1 -> f1
        if (!pos.white_can_castle_king)
            return out;

        if (pos.b[4] != 'K' || pos.b[7] != 'R')
            return out;

        if (!isEmpty(pos.b[5]) || !isEmpty(pos.b[6]))
            return out;

        if (isSquareAttacked(pos, 4, false))
            return out;
        if (isSquareAttacked(pos, 5, false))
            return out;
        if (isSquareAttacked(pos, 6, false))
            return out;

        out = addMove(out, 4, 6);
    }
    else
    {
        // Black: e8 -> g8, rook h8 -> f8
        if (!pos.black_can_castle_king)
            return out;

        if (pos.b[60] != 'k' || pos.b[63] != 'r')
            return out;

        if (!isEmpty(pos.b[61]) || !isEmpty(pos.b[62]))
            return out;

        if (isSquareAttacked(pos, 60, true))
            return out;
        if (isSquareAttacked(pos, 61, true))
            return out;
        if (isSquareAttacked(pos, 62, true))
            return out;

        out = addMove(out, 60, 62);
    }

    return out;
}

Move *GenerateMoves::addCastleQueenSide(const Position &pos, Move *out)
{
    if (pos.white_to_move)
    {
        // White: e1 -> c1, rook a1 -> d1
        if (!pos.white_can_castle_queen)
            return out;

        if (pos.b[4] != 'K' || pos.b[0] != 'R')
            return out;

        if (!isEmpty(pos.b[1]) || !isEmpty(pos.b[2]) || !isEmpty(pos.b[3]))
            return out;

        if (isSquareAttacked(pos, 4, false))
            return out;
        if (isSquareAttacked(pos, 3, false))
            return out;
        if (isSquareAttacked(pos, 2, false))
            return out;

        out = addMove(out, 4, 2);
    }
    else
    {
        // Black: e8 -> c8, rook a8 -> d8
        if (!pos.black_can_castle_queen)
            return out;

        if (pos.b[60] != 'k' || pos.b[56] != 'r')
            return out;

        if (!isEmpty(pos.b[57]) || !isEmpty(pos.b[58]) || !isEmpty(pos.b[59]))
            return out;

        if (isSquareAttacked(pos, 60, true))
            return out;
        if (isSquareAttacked(pos, 59, true))
            return out;
        if (isSquareAttacked(pos, 58, true))
            return out;

        out = addMove(out, 60, 58);
    }

    return out;
}

Move *GenerateMoves::genKing(const Position &pos, Move *out)
{
    static const int dirs[8][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    const bool white = pos.white_to_move;
    const char king = white ? 'K' : 'k';

    for (int from = 0; from < 64; ++from)
    {
        if (pos.b[from] != king)
            continue;

        const int row = from >> 3;
        const int col = from & 7;

        for (int i = 0; i < 8; ++i)
        {
            const int toRow = row + dirs[i][1];
            const int toCol = col + dirs[i][0];

            if (toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7)
                continue;

            const int to = toRow * 8 + toCol;
            const char target = pos.b[to];

            if (!isEmpty(target) && isWhitePiece(target) == white)
                continue;

            out = addMove(out, from, to);
        }

        // Only one king exists, so castling can be added once here.
        out = addCastleKingSide(pos, out);
        out = addCastleQueenSide(pos, out);
        break;
    }

    return out;
}

Move *GenerateMoves::genAllMoves(const Position &pos, Move *out)
{
    out = genPawn(pos, out);
    out = genKnight(pos, out);
    out = genBishop(pos, out);
    out = genRook(pos, out);
    out = genQueen(pos, out);
    out = genKing(pos, out);
    return out;
}