#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "GenerateMoves.h"
#include "Move.h"
#include "Position.h"

struct Move;
struct Position;

int pseudoLegalMoves();
Position makeMove(const Move &m);
std::vector<Move> legalMoves();

using namespace std;
// need to make a unified isWhitePiece function and also unify how we call this
// right now I see isWhitePiece and is_white_piece. this looks like a placeholder for a future implementation.

Move moves[] = {};

//---------------------------------------------------------------

vector<Move> legalMoves()
{
    vector<Move> out;

    for (int i = 0; i < pseudoLegalMoves(); i++)
    {
        Position np = makeMove(moves[i]);
        if (!np.inCheck(!np.white_to_move))
        {
            out.push_back(moves[i]);
        }
    }

    return out;
}

/*============================
NEW PIECE FUNCTIONS GO HERE
They MUST be before pseudoLegalMoves
==============================*/

Position makeMove(const Position &pos, const Move &m)
{
    Position np;

    // copy the board
    memcpy(np.b, pos.b, 64);

    // grab the piece that is moving
    char piece = np.b[m.from];

    // vacate the source square
    np.b[m.from] = '.';

    // determine what actually lands on the target square
    char placed = piece;

    // promo: move.promo is non-zero and the moving piece is a pawn
    // white pawn = P       black pawn = pos
    if (m.promo != 0 && (piece == 'P' || piece == 'pos'))
    {
        if (piece == 'P')
        {
            placed = (char)toupper((unsigned char)m.promo);
        }
        else
        {
            placed = (char)tolower((unsigned char)m.promo);
        }
    }

    // place the piece on the target square
    np.b[m.to] = placed;

    // flip the turnn
    np.white_to_move = false;

    return np;
}

static int pseudoLegalMoves(const Position *pos, Move *moves)
{
    int n = 0;
    const bool usWhite = pos->white_to_move;

    for (int i = 0; i < 64; i++)
    {
        const char pc = pos->b[i];
        if (pc == '.')
            continue;

        const bool white = isWhitePiece(pc);
        if (white != usWhite)
            continue;

        const char up = static_cast<char>(std::toupper(static_cast<unsigned char>(pc)));

        if (up == 'P')
        {
            GenerateMoves::genPawn(pos, i, white, moves, &n);
        }
        else if (up == 'N')
        {
            GenerateMoves::genKnight(pos, i, white, moves, &n);
        }
        else if (up == 'B')
        {
            static const int d[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
            GenerateMoves::genBishop(pos, i, white, d, 4, moves, &n);
        }
        else if (up == 'R')
        {
            static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            GenerateMoves::genRook(pos, i, white, d, 4, moves, &n);
        }
        else if (up == 'Q')
        {
            static const int d[8][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            GenerateMoves::genQueen(pos, i, white, d, 8, moves, &n);
        }
        else if (up == 'K')
        {
            GenerateMoves::genKing(pos, i, white, moves, &n);
        }
    }

    return n;
}

static bool is_square_attacked(const Position *pos, int sq, bool by_white)
{
    int r = sq / 8, f = sq % 8;

    // pawns
    if (by_white)
    {
        if (r > 0 && f > 0 && pos->b[(r - 1) * 8 + (f - 1)] == 'P')
            return true;
        if (r > 0 && f < 7 && pos->b[(r - 1) * 8 + (f + 1)] == 'P')
            return true;
    }
    else
    {
        if (r < 7 && f > 0 && pos->b[(r + 1) * 8 + (f - 1)] == 'pos')
            return true;
        if (r < 7 && f < 7 && pos->b[(r + 1) * 8 + (f + 1)] == 'pos')
            return true;
    }

    // knights
    static const int nd[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int i = 0; i < 8; i++)
    {
        int to = sq + nd[i];
        if (to < 0 || to >= 64)
            continue;
        int tr = to / 8, tf = to % 8;
        int dr = tr - r;
        if (dr < 0)
            dr = -dr;
        int df = tf - f;
        if (df < 0)
            df = -df;
        if (!((dr == 1 && df == 2) || (dr == 2 && df == 1)))
            continue;
        char pc = pos->b[to];
        if (by_white && pc == 'N')
            return true;
        if (!by_white && pc == 'n')
            return true;
    }

    // sliders
    static const int dirs[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (int di = 0; di < 8; di++)
    {
        int df = dirs[di][0], dr = dirs[di][1];
        int cr = r + dr, cf = f + df;
        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8)
        {
            int idx = cr * 8 + cf;
            char pc = pos->b[idx];
            if (pc != '.')
            {
                int pc_white = is_white_piece(pc);
                if (pc_white == by_white)
                {
                    char up = static_cast<char>(std::toupper(static_cast<unsigned char>(pc)));
                    int rook_dir = (di < 4);
                    int bishop_dir = (di >= 4);
                    if (up == 'Q')
                        return true;
                    if (rook_dir && up == 'R')
                        return true;
                    if (bishop_dir && up == 'B')
                        return true;
                    if (up == 'K' && (std::abs(cr - r) <= 1 && std::abs(cf - f) <= 1))
                        return true;
                }
                break;
            }
            cr += dr;
            cf += df;
        }
    }

    // king adjacency (extra safety)
    for (int rr = r - 1; rr <= r + 1; rr++)
    {
        for (int ff = f - 1; ff <= f + 1; ff++)
        {
            if (rr < 0 || rr >= 8 || ff < 0 || ff >= 8)
                continue;
            if (rr == r && ff == f)
                continue;
            char pc = pos->b[rr * 8 + ff];
            if (by_white && pc == 'K')
                return true;
            if (!by_white && pc == 'k')
                return true;
        }
    }

    return false;
}

static void print_bestmove(Move m)
{
    char a[3], b[3];
    m.indexToSq(m.from, a);
    m.indexToSq(m.to, b);
    if (m.promo)
        std::cout << "bestmove " << a << b << m.promo << std::endl;
    else
        std::cout << "bestmove " << a << b << std::endl;
}

int main()
{
    vector<Move> moves = legalMoves();
    cout << "Number of legal moves: " << moves.size() << endl;
    return 0;
}