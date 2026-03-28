#include <iostream>
#include <vector>

#include "GenerateMoves.h"
#include "MakeMove.h"
#include "Move.h"
#include "Position.h"

using namespace std;

/**
 * @brief Generates all pseudo-legal moves using GenerateMoves.
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
    const vector<Move> pseudo = pseudoLegalMoves(pos);
    vector<Move> out;

    for (const Move &m : pseudo)
    {
        const Position np = makeMove(pos, m);

        if (!np.inCheck(!np.white_to_move))
            out.push_back(m);
    }

    return out;
}

/**
 * @brief Prints a move in UCI format using "bestmove" prefix.
 * @param m Move to print.
 */
static void printBestMove(const Move &m)
{
    char uci[6];
    m.toUci(uci);
    cout << "bestmove " << uci << '\n';
}

int main()
{
    Position pos{};
    pos.white_to_move = true;

    for (int i = 0; i < 64; ++i)
        pos.b[i] = '.';

    // Example position
    pos.b[Move::squareIndex("e1")] = 'K';
    pos.b[Move::squareIndex("e2")] = 'P';
    pos.b[Move::squareIndex("e8")] = 'k';
    pos.b[Move::squareIndex("d3")] = 'p';
    pos.b[Move::squareIndex("f3")] = 'p';

    const vector<Move> moves = legalMoves(pos);

    cout << "Number of legal moves: " << moves.size() << '\n';

    for (const Move &m : moves)
    {
        char uci[6];
        m.toUci(uci);
        cout << uci << '\n';
    }

    if (!moves.empty())
        printBestMove(moves[0]);

    return 0;
}