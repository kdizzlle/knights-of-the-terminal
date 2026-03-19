#include <iostream>
#include <vector>
#include <cctype>

using namespace std;

struct Move;
struct Position;

int pseudoLegalMoves();
Position makeMove(const Move &m);
vector<Move> legalMoves();

Move moves[] = {};

struct Move
{
    int from, to;
    char promo;
};

struct Position
{
    bool white_to_move;
    char b;

    bool inCheck(bool side) const
    {
        // replace with your real logic
        return false;
    }
};

// from regular sample code, may change later if it's more c++-like
static int sq_index(const char *s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return rank * 8 + file;
}

static void index_to_sq(int idx, char out[3]) {
    out[0] = (char) ('a' + (idx % 8));
    out[1] = (char) ('1' + (idx / 8));
    out[2] = 0;
}

int main()
{
    vector<Move> moves = legalMoves();
    cout << "Number of legal moves: " << moves.size() << endl;
    return 0;
}

/*


*/

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

static int pseudoLegalMoves(const Position *p, Move *moves)
{
    int n = 0;
    const bool usWhite = p->white_to_move;

    for (int i = 0; i < 64; i++)
    {
        const char pc = p->b[i];
        if (pc == '.')
            continue;

        bool white = isWhitePiece(pc);
        if (white != usWhite)
            continue;

        const char up = static_cast<char>(std::toupper(static_cast<unsigned char>(pc)));

        switch (up)
        {
        case 'P':
            genPawn(p, i, white, moves, &n);
            break;
        case 'N':
            genKnite(p, i, white, moves, &n);
            break;
        case 'B':
            static const int d = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
            genBishop(p, i, white, moves, &n);
            break;
        case 'R':
            d = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            genRook(p, i, white, moves, &n);
            break;
        case 'Q':
            static const int d = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
            genQueen(p, i, white, moves, &n);
            break;
        case 'K':
            static const int d[8][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            genKing(p, i, white, moves, &n);
            break;
        default:
            // TODO: handle exception
            break;
        }
        return n;
    }
}
Position makeMove(const Move &m)
{
    Position p;
    p.white_to_move = false;
    return p;
}

static void print_bestmove(Move m) {
    char a[3], b[3];
    index_to_sq(m.from, a);
    index_to_sq(m.to, b);
    if (m.promo) std::cout<<"bestmove "<< a << b << m.promo << std::end1;
    else std::cout<<"bestmove "<< a << b << std::end1;
}
