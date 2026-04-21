#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Board.h"
#include "MoveGenerator.h"
#include "Bot.h"

using namespace std;

static SearchLimits parseGo(const string& line) {
    SearchLimits p;
    istringstream ss(line);
    string tok;
    ss >> tok;
    while (ss >> tok) {
        if (tok == "movetime") ss >> p.movetimeMs;
        else if (tok == "depth") ss >> p.depth;
        else if (tok == "wtime") ss >> p.wtime;
        else if (tok == "btime") ss >> p.btime;
        else if (tok == "winc") ss >> p.winc;
        else if (tok == "binc") ss >> p.binc;
    }
    return p;
}

static void printBestMove(const Move& m) {
    char uci[6];
    m.toUci(uci);
    cout << "bestmove " << (m.isNull() ? "0000" : uci) << '\n';
    cout.flush();
}

static bool parsePosition(Board& board, const string& line, vector<string>& movesPlayed) {
    movesPlayed.clear();
    istringstream ss(line);
    string tok;
    ss >> tok; // position
    ss >> tok;

    if (tok == "startpos") {
        board.setStartPos();
        if (!(ss >> tok)) return true;
    } else if (tok == "fen") {
        string fen, part;
        for (int i = 0; i < 6 && ss >> part; ++i) {
            if (i) fen += ' ';
            fen += part;
        }
        if (!board.setFromFen(fen)) return false;
        if (!(ss >> tok)) return true;
    }

    if (tok == "moves") {
        while (ss >> tok) {
            Move m = Move::fromUci(tok.c_str());
            UndoState u;
            if (!board.makeMove(m, u)) return false;
            movesPlayed.push_back(tok);
        }
    }

    return true;
}

int main() {
    Bot::init();
    Board board;
    vector<string> movesPlayed;
    string line;

    while (getline(cin, line)) {
        if (line == "uci") {
            cout << "id name KnightsoftheTerminal\n";
            cout << "id author Group2\n";
            cout << "uciok\n";
            cout.flush();
        } else if (line == "isready") {
            cout << "readyok\n";
            cout.flush();
        } else if (line == "ucinewgame") {
            board.setStartPos();
            movesPlayed.clear();
        } else if (line.rfind("position", 0) == 0) {
            parsePosition(board, line, movesPlayed);
        } else if (line.rfind("go", 0) == 0) {
            SearchLimits limits = parseGo(line);
            Move best = Bot::chooseMove(board, limits, movesPlayed);
            printBestMove(best);
        } else if (line == "quit") {
            break;
        }
    }

    return 0;
}
