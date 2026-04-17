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
#include "Search.h"

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
    p.white_can_castle_king = false;
    p.white_can_castle_queen = false;
    p.black_can_castle_king = false;
    p.black_can_castle_queen = false;
    p.halfmove_clock = 0;
    p.fullmove_number = 1;

    istringstream ss(fen);
    string placement, stm, castling, unusedField;
    int halfmove = 0;
    int fullmove = 1;

    ss >> placement >> stm >> castling >> unusedField >> halfmove >> fullmove;

    if (!stm.empty())
        p.white_to_move = (stm == "w");

    int rank = 7;
    int file = 0;

    for (char c : placement)
    {
        if (c == '/')
        {
            rank--;
            file = 0;
        }
        else if (isdigit(static_cast<unsigned char>(c)))
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

    if (!castling.empty() && castling != "-")
    {
        for (char c : castling)
        {
            if (c == 'K')
                p.white_can_castle_king = true;
            else if (c == 'Q')
                p.white_can_castle_queen = true;
            else if (c == 'k')
                p.black_can_castle_king = true;
            else if (c == 'q')
                p.black_can_castle_queen = true;
        }
    }

    p.halfmove_clock = halfmove;
    p.fullmove_number = fullmove;

    return p;
}

static Position posStart()
{
    return posFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
        if (!(ss >> token))
            return pos;
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
        }

        string fen;
        for (int i = 0; i < k; ++i)
        {
            if (i)
                fen += ' ';
            fen += fenParts[i];
        }

        if (!fen.empty())
            pos = posFromFen(fen);
    }

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
// Basic go parsing
// ---------------------------------------------------------------------------

static int parseGoDepth(const string &line)
{
    istringstream ss(line);
    string token;
    int depth = 3; // safe default

    ss >> token; // "go"

    while (ss >> token)
    {
        if (token == "depth")
        {
            int d;
            if (ss >> d && d > 0)
                depth = d;
        }
        else if (token == "movetime")
        {
            int ms;
            if (ss >> ms)
            {
                if (ms <= 100)
                    depth = 2;
                else if (ms <= 500)
                    depth = 3;
                else if (ms <= 2000)
                    depth = 4;
                else
                    depth = 5;
            }
        }
    }

    return depth;
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
        while (!line.empty() &&
               (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
        {
            line.pop_back();
        }

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
        else if (line.size() >= 8 && line.substr(0, 8) == "position")
        {
            pos = parsePosition(line);
        }
        else if (line.size() >= 2 && line.substr(0, 2) == "go")
        {
            const vector<Move> moves = legalMoves(pos);

            if (moves.empty())
            {
                cout << "bestmove 0000\n";
                cout.flush();
            }
            else
            {
                const int depth = parseGoDepth(line);
                const Move best = Search::findBestMove(pos, depth);
                printBestMove(best);
            }
        }
        else if (line == "quit")
        {
            break;
        }
    }

    return 0;
}