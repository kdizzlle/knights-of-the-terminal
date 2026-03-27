#include "GenerateMoves.h"

void *GenerateMoves::genRook(const Position *pos, int from, bool white, Move *moves, int *n, bool castleflag)
{
    const int d[4][2] = {{1, 0},
                         {-1, 0},
                         {0, 1},
                         {0, -1}};
    bool finish;
    bool finishl, finishr, finishu, finishd;
    Move move[28];

    if (!castleflag)
    {
        while (!finish)
        {
            for (int i = 1; i <= 7; i++)
            {
                int idxl = from + (i * d[0]);
                int idxr = from + (i * d[2]);
                int idxu = from + (i * d[6]);
                int idxd = from + (i * d[8]);
                if (pos->b[idxl] != '.' && !finishl)
                {
                    if (IsWhitePiece(pos->b[idxl]))
                        idxl = -1; // can't move this direction
                    else
                    {
                        move[i - 1].from = from;
                        move[i - 1].to = idxl;
                        move[i - 1].promo = 0;
                        n++;
                        finishl = true; // capture piece and mark direction as closed
                    }
                }
                else if (pos->b[idxl] == '.' && !finishl)
                {
                    move[i - 1].from = from;
                    move[i - 1].to = idxl;
                    move[i - 1].promo = 0;
                    n++;
                }
                if (pos->b[idxr] != '.' && !finishr)
                {
                    if (IsWhitePiece(pos->b[idxr]))
                        idxl = -1; // can't move this direction
                    else
                    {
                        move[i - 1].from = from;
                        move[i - 1].to = idxr;
                        move[i - 1].promo = 0;
                        n++;
                        finishr = true; // capture piece and mark direction as closed
                    }
                }
                else if (pos->b[idxr] == '.' && finishr)
                {
                    move[i - 1].from = from;
                    move[i - 1].to = idxr;
                    move[i - 1].promo = 0;
                    n++;
                }
                if (pos->b[idxu] != '.' && !finishu)
                {
                    if (IsWhitePiece(pos->b[idxu]))
                        idxl = -1; // can't move this direction
                    else
                    {
                        move[i - 1].from = from;
                        move[i - 1].to = idxu;
                        move[i - 1].promo = 0;
                        finishr = true; // capture piece and mark direction as closed
                        n++;
                    }
                }
                else if (pos->b[idxu] == '.' && finishu)
                {
                    move[i - 1].from = from;
                    move[i - 1].to = idxu;
                    move[i - 1].promo = 0;
                    n++;
                }
                if (pos->b[idxd] != '.' && !finishd)
                {
                    if (IsWhitePiece(pos->b[idxd]))
                        idxl = -1; // can't move this direction
                    else
                    {
                        move[i - 1].from = from;
                        move[i - 1].to = idxd;
                        move[i - 1].promo = 0;
                        finishd = true; // capture piece and mark direction as closed
                        n++;
                    }
                }
                else if (pos->b[idxd] == '.' && finishd)
                {
                    move[i - 1].from = from;
                    move[i - 1].to = idxd;
                    move[i - 1].promo = 0;
                    n++;
                }
            }
        }
    }
    else
    {
        return from; // pass back the index - change later?
    }
}

void *GenerateMoves::genKnight(const Position *pos, int from, bool white, Move *moves, int *n)
{
    static const int offsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};

    int fromRow = from / 8;
    int fromCol = from % 8;

    for (int i = 0; i < 8; i++)
    {
        int toRow = fromRow + offsets[i][0];
        int toCol = fromCol + offsets[i][1];

        if (toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7)
            continue;

        int to = toRow * 8 + toCol;
        char target = pos->b[to];

        if (target != '.' && isWhitePiece(target) == white)
            continue;

        addMove(moves, n, from, to, 0);
    }
}

void *GenerateMoves::genQueen(const Position *pos, int from, bool white, Move *moves, int *n)
{
    static const int dirs[8][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    int r = from / 8;
    int f = from % 8;

    for (int di = 0; di < 8; di++)
    {
        int df = dirs[di][0];
        int dr = dirs[di][1];

        int cr = r + dr;
        int cf = f + df;

        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8)
        {
            int to = cr * 8 + cf;
            char target = pos->b[to];

            if (target == '.')
            {
                moves[*n] = Move(from, to);
                (*n)++;
            }
            else
            {
                bool targetWhite = isWhitePiece(target);
                if (targetWhite != white)
                {
                    moves[*n] = Move(from, to);
                    (*n)++;
                }
                break;
            }

            cr += dr;
            cf += df;
        }
    }
}

void *GenerateMoves::genKing(const Position *pos, int from, bool white, Move *moves, int *n)
{ // kelly
    // All 8 directions the king can move: diagonals + cardinal directions
    // Each entry is {file delta, rank delta}
    static const int dirs[8][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    // Convert the king's square index to row/col for bounds checking
    int fromRow = from / 8;
    int fromCol = from % 8;

    for (int i = 0; i < 8; i++)
    {
        int toRow = fromRow + dirs[i][1];
        int toCol = fromCol + dirs[i][0];

        // Skip squares that fall off the board
        if (toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7)
            continue;

        int to = toRow * 8 + toCol;
        char target = pos->b[to];

        // Skip squares occupied by a friendly piece
        if (target != '.' && isWhitePiece(target) == white)
            continue;

        // Square is empty or holds an enemy piece — add the move
        moves[*n] = Move(from, to);
        (*n)++;
    }
}

void *GenerateMoves::genBishop(const Position *pos, int from, bool white, const int dirs[][2], int dcount, Move *moves, int *n)
{
    int r = from / 8;
    int f = from % 8;

    for (int di = 0; di < dcount; di++)
    {
        int df = dirs[di][0];
        int dr = dirs[di][1];

        int cr = r + dr;
        int cf = f + df;

        // Slide diagonally until blocked
        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8)
        {
            int to = cr * 8 + cf;
            char target = pos->b[to];

            if (target == '.')
            {
                // Empty square
                moves[*n] = Move(from, to);
                (*n)++;
            }
            else
            {
                // Occupied square
                bool targetWhite = isWhitePiece(target);
                if (targetWhite != white)
                {
                    // Capture
                    moves[*n] = Move(from, to);
                    (*n)++;
                }
                // Stop sliding after any piece
                break;
            }
            cr += dr;
            cf += df;
        }
    }
}