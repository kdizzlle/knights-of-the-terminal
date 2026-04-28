#include <cctype>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Attack.h"
#include "GenerateMoves.h"
#include "MakeMove.h"
#include "Move.h"
#include "Position.h"

using namespace std;

// ---------------------------------------------------------------------------
// Move generation helpers
// ---------------------------------------------------------------------------

static vector<Move> pseudoLegalMoves(const Position &pos)
{
    Move buffer[256];
    Move *end = GenerateMoves::genAllMoves(pos, buffer);
    return vector<Move>(buffer, end);
}

static vector<Move> legalMoves(const Position &pos)
{
    vector<Move> out;
    for (const Move &m : pseudoLegalMoves(pos))
    {
        const Position np = makeMove(pos, m);
        if (!np.inCheck(!np.white_to_move))
            out.push_back(m);
    }
    return out;
}

// ---------------------------------------------------------------------------
// FEN / position parsing
// ---------------------------------------------------------------------------

static Position posFromFen(const string &fen)
{
    Position p{};
    for (int i = 0; i < 64; ++i)
        p.b[i] = '.';
    p.white_to_move = true;

    istringstream ss(fen);
    string placement, stm;
    ss >> placement >> stm;

    if (!stm.empty())
        p.white_to_move = (stm == "w");

    int rank = 7, file = 0;
    for (char c : placement)
    {
        if (c == '/')
        {
            rank--;
            file = 0;
        }
        else if (isdigit((unsigned char)c))
        {
            file += c - '0';
        }
        else
        {
            int idx = rank * 8 + file;
            if (idx >= 0 && idx < 64)
                p.b[idx] = c;
            file++;
        }
    }
    return p;
}

static Position posStart()
{
    return posFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
}

static void applyUciMove(Position &pos, const string &uci)
{
    Move m = Move::fromUci(uci.c_str());
    pos = makeMove(pos, m);
}

// "position startpos [moves ...]"
// "position fen <fen> [moves ...]"
static Position parsePosition(const string &line)
{
    istringstream ss(line);
    string token;
    ss >> token; // "position"

    Position pos = posStart();

    ss >> token;
    if (token == "startpos")
    {
        ss >> token; // consume optional "moves"
    }
    else if (token == "fen")
    {
        string fenParts[6];
        int k = 0;
        while (k < 6 && ss >> token)
        {
            if (token == "moves")
                break;
            fenParts[k++] = token;
            ss >> ws;
        }
        string fen;
        for (int i = 0; i < k; ++i)
        {
            if (i) fen += ' ';
            fen += fenParts[i];
        }
        pos = posFromFen(fen);
        // token may now be "moves" or another token — handled below
    }

    // Apply move list if present
    if (token == "moves")
    {
        while (ss >> token)
            applyUciMove(pos, token);
    }

    return pos;
}

// ---------------------------------------------------------------------------
// Output
// ---------------------------------------------------------------------------

static void printBestMove(const Move &m)
{
    char uci[6];
    m.toUci(uci);
    cout << "bestmove " << uci << '\n';
    cout.flush();
}

// ---------------------------------------------------------------------------
// UCI main loop
// ---------------------------------------------------------------------------

int main()
{
    Position pos = posStart();
    string line;

    while (getline(cin, line))
    {
        // strip trailing whitespace
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
            line.pop_back();

        if (line.empty())
            continue;

        if (line == "uci")
        {
            cout << "id name KnightsOfTheTerminal\n";
            cout << "id author kdizzlle\n";
            cout << "uciok\n";
            cout.flush();
        }
        else if (line == "isready")
        {
            cout << "readyok\n";
            cout.flush();
        }
        else if (line == "ucinewgame")
        {
            pos = posStart();
        }
        else if (line.substr(0, 8) == "position")
        {
            pos = parsePosition(line);
        }
        else if (line.substr(0, 2) == "go")
        {
            const vector<Move> moves = legalMoves(pos);
            if (moves.empty())
            {
                cout << "bestmove 0000\n";
                cout.flush();
            }
            else
            {
                printBestMove(moves[0]);
            }
        }
        else if (line == "quit")
        {
            break;
        }
    }

    return 0;
}