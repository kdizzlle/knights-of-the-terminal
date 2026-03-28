#include "GenerateMoves.h"

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
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};

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
            const int toRow = row + offsets[i][0];
            const int toCol = col + offsets[i][1];

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
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    return genSliding(pos, out, pos.white_to_move ? 'Q' : 'q', dirs, 8);
}

Move *GenerateMoves::genKing(const Position &pos, Move *out)
{
    static const int dirs[8][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

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