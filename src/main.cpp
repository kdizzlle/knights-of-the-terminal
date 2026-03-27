#include <cctype>
#include <cstring>
#include <iostream>
#include <vector>

#include "GenerateMoves.h"
#include "Move.h"
#include "Position.h"

using namespace std;

/**
 * @brief Returns true if the given piece character is a white piece.
 * @param p Piece character from the board.
 * @return True if uppercase, otherwise false.
 */
static bool isWhitePiece(char p)
{
    return p >= 'A' && p <= 'Z';
}

/**
 * @brief Applies a move to a position and returns the resulting position.
 *
 * This performs a simple board update:
 * - moves the piece
 * - handles promotion
 * - flips the side to move
 *
 * @param pos Original position.
 * @param m Move to apply.
 * @return New position after the move is made.
 */
Position makeMove(const Position &pos, const Move &m)
{
    Position np = pos;

    // Grab moving piece
    char piece = np.b[m.from];

    // Vacate source square
    np.b[m.from] = '.';

    // Determine placed piece
    char placed = piece;

    if (m.promo != 0 && (piece == 'P' || piece == 'p'))
    {
        if (piece == 'P')
            placed = static_cast<char>(toupper(static_cast<unsigned char>(m.promo)));
        else
            placed = static_cast<char>(tolower(static_cast<unsigned char>(m.promo)));
    }

    // Place on target square
    np.b[m.to] = placed;

    // Flip side to move
    np.white_to_move = !pos.white_to_move;

    return np;
}

/**
 * @brief Finds the square index of the king for the specified side.
 * @param pos Position to inspect.
 * @param white True for white king, false for black king.
 * @return Board index of the king, or -1 if not found.
 */
static int findKing(const Position &pos, bool white)
{
    const char king = white ? 'K' : 'k';
    for (int i = 0; i < 64; ++i)
    {
        if (pos.b[i] == king)
            return i;
    }
    return -1;
}

/**
 * @brief Returns true if a square is attacked by the specified side.
 * @param pos Position to inspect.
 * @param sq Target square index.
 * @param by_white True if checking attacks by white, false for black.
 * @return True if the square is attacked, otherwise false.
 */
static bool is_square_attacked(const Position &pos, int sq, bool by_white)
{
    const int r = sq / 8;
    const int f = sq % 8;

    // Pawn attacks
    if (by_white)
    {
        if (r > 0 && f > 0 && pos.b[(r - 1) * 8 + (f - 1)] == 'P')
            return true;
        if (r > 0 && f < 7 && pos.b[(r - 1) * 8 + (f + 1)] == 'P')
            return true;
    }
    else
    {
        if (r < 7 && f > 0 && pos.b[(r + 1) * 8 + (f - 1)] == 'p')
            return true;
        if (r < 7 && f < 7 && pos.b[(r + 1) * 8 + (f + 1)] == 'p')
            return true;
    }

    // Knight attacks
    static const int nd[8] = {-17, -15, -10, -6, 6, 10, 15, 17};

    for (int i = 0; i < 8; ++i)
    {
        const int to = sq + nd[i];
        if (to < 0 || to >= 64)
            continue;

        const int tr = to / 8;
        const int tf = to % 8;
        const int dr = abs(tr - r);
        const int df = abs(tf - f);

        if (!((dr == 1 && df == 2) || (dr == 2 && df == 1)))
            continue;

        const char pc = pos.b[to];
        if (by_white && pc == 'N')
            return true;
        if (!by_white && pc == 'n')
            return true;
    }

    // Sliding piece attacks + king one-square attacks
    static const int dirs[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (int di = 0; di < 8; ++di)
    {
        const int df = dirs[di][0];
        const int dr = dirs[di][1];

        int cr = r + dr;
        int cf = f + df;
        int distance = 1;

        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8)
        {
            const int idx = cr * 8 + cf;
            const char pc = pos.b[idx];

            if (pc != '.')
            {
                if (isWhitePiece(pc) == by_white)
                {
                    const char up = static_cast<char>(toupper(static_cast<unsigned char>(pc)));
                    const bool rook_dir = (di < 4);
                    const bool bishop_dir = (di >= 4);

                    if (up == 'Q')
                        return true;
                    if (rook_dir && up == 'R')
                        return true;
                    if (bishop_dir && up == 'B')
                        return true;
                    if (distance == 1 && up == 'K')
                        return true;
                }
                break;
            }

            cr += dr;
            cf += df;
            ++distance;
        }
    }

    return false;
}

/**
 * @brief Checks whether the given side's king is in check.
 * @param pos Position to inspect.
 * @param white True for white, false for black.
 * @return True if that side is in check, otherwise false.
 */
static bool inCheckComputed(const Position &pos, bool white)
{
    const int kingSq = findKing(pos, white);
    if (kingSq < 0)
        return false; // fallback if king not found
    return is_square_attacked(pos, kingSq, !white);
}

/**
 * @brief Generates all pseudo-legal moves using GenerateMoves.cpp.
 * @param pos Current board position.
 * @return Vector containing all pseudo-legal moves.
 */
vector<Move> pseudoLegalMoves(const Position &pos)
{
    Move buffer[256];
    Move *end = GenerateMoves::genAllMoves(pos, buffer);

    vector<Move> out;
    for (Move *m = buffer; m != end; ++m)
        out.push_back(*m);

    return out;
}

/**
 * @brief Generates all legal moves by filtering pseudo-legal moves.
 * @param pos Current board position.
 * @return Vector containing legal moves only.
 */
vector<Move> legalMoves(const Position &pos)
{
    vector<Move> pseudo = pseudoLegalMoves(pos);
    vector<Move> out;

    for (const Move &m : pseudo)
    {
        Position np = makeMove(pos, m);

        // After making a move, the side that just moved is !np.white_to_move
        if (!inCheckComputed(np, !np.white_to_move))
            out.push_back(m);
    }

    return out;
}

/**
 * @brief Prints a move in UCI format using "bestmove" prefix.
 * @param m Move to print.
 */
static void print_bestmove(const Move &m)
{
    char uci[6];
    m.toUci(uci);
    cout << "bestmove " << uci << endl;
}

int main()
{
    Position pos{};
    pos.white_to_move = true;

    // Initialize board to empty
    for (int i = 0; i < 64; ++i)
        pos.b[i] = '.';

    // Example position:
    // White: king e1, pawn e2
    // Black: king e8, pawns d3 and f3
    pos.b[Move::squareIndex("e1")] = 'K';
    pos.b[Move::squareIndex("e2")] = 'P';
    pos.b[Move::squareIndex("e8")] = 'k';
    pos.b[Move::squareIndex("d3")] = 'p';
    pos.b[Move::squareIndex("f3")] = 'p';

    vector<Move> moves = legalMoves(pos);

    cout << "Number of legal moves: " << moves.size() << endl;

    for (const Move &m : moves)
    {
        char uci[6];
        m.toUci(uci);
        cout << uci << endl;
    }

    if (!moves.empty())
        print_bestmove(moves[0]);

    return 0;
}