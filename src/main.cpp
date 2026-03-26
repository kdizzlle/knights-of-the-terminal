#include <iostream>
#include <vector>
#include <cctype>
#include <cstring>

using namespace std;

struct Move;
struct Position;

int pseudoLegalMoves();
Position makeMove(const Move &m);
vector<Move> legalMoves();

Move moves[] = {};

struct Move
{
    int from;
    int to;
    char promo;     // 0 = none, else 'q'/'r'/'b'/'n'

    Move() : from(0), to(0), promo(0) {}
    Move(int f, int t, char p = 0) : from(f), to(t), promo(p) {}

    static int squareIndex(const char* sq) {
        int file = sq[0] - 'a';
        int rank = sq[1] - '1';
        return rank * 8 + file;
    }

    // parse a UCI move string
    static Move fromUci(const char* s) {
        if (!s || strlen(s) < 4)
            return Move(0, 0, 0);
        char fromSq[3] = {s[0], s[1], '\0'};
        char toSq[3] = {s[2], s[3], '\0'};
        int from = squareIndex(fromSq);
        int to = squareIndex(toSq);
        char promo = (strlen(s) >= 5) ? s[4] : 0;
        return Move(from, to, promo);
    }

    // Convert back to UCI string
    static void indexToSq(int idx, char out[3]) {
        out[0] = 'a' + (idx % 8);
        out[1] = '1' + (idx / 8);
        out[2] = '\0';
    }
 
    void toUci(char out[6]) const {
        char f[3], t[3];
        indexToSq(from, f);
        indexToSq(to,   t);
        out[0] = f[0]; out[1] = f[1];
        out[2] = t[0]; out[3] = t[1];
        if (promo) { out[4] = promo; out[5] = '\0'; }
        else        { out[4] = '\0'; }
    }
};

//---------------------------------------------------------------

struct Position
{
    bool white_to_move;
    char b[64];

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


static void genKnight(const Pos* p, int from, bool white, Move* moves, int* n) {
    static const int offsets[8][2] = {
        {-2, -1}, {-2, 1},
        {-1, -2}, {-1, 2},
        { 1, -2}, { 1, 2},
        { 2, -1}, { 2, 1}
    };

    int fromRow = from / 8;
    int fromCol = from % 8;

    for (int i = 0; i < 8; i++) {
        int toRow = fromRow + offsets[i][0];
        int toCol = fromCol + offsets[i][1];

        if (toRow < 0 || toRow > 7 || toCol < 0 || toCol > 7) continue;

        int to = toRow * 8 + toCol;
        char target = p->b[to];

        if (target != '.' && isWhitePiece(target) == white) continue;

        addMove(moves, n, from, to, 0);
    }
}



static int pseudoLegalMoves(const Pos* p, Move* moves) {
    int n = 0;
    const bool usWhite = p->white_to_move;

    for (int i = 0; i < 64; i++) {
        const char pc = p->b[i];
        if (pc == '.') continue;

        const bool white = isWhitePiece(pc);
        if (white != usWhite) continue;

        const char up = static_cast<char>(std::toupper(static_cast<unsigned char>(pc)));

        if (up == 'P') {
            genPawn(p, i, white, moves, &n);
        }
        else if (up == 'N') {
            genKnight(p, i, white, moves, &n);
        }
        else if (up == 'B') {
            static const int d[4][2] = { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
            genBishop(p, i, white, d, 4, moves, &n);
        }
        else if (up == 'R') {
            static const int d[4][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
            genRook(p, i, white, d, 4, moves, &n);
        }
        else if (up == 'Q') {
            static const int d[8][2] = { {1,1},{1,-1},{-1,1},{-1,-1},
                                         {1,0},{-1,0},{0,1},{0,-1} };
            genQueen(p, i, white, d, 8, moves, &n);
        }
        else if (up == 'K') {
            genKing(p, i, white, moves, &n);
        }
    }

    return n;
}

Position makeMove(const Position &pos, const Move &m)
{
    Position np;

    //copy the board
    memcpy(np.b, pos.b, 64);

    //grab the piece that is moving
    char piece = np.b[m.from];

    //vacate the source square
    np.b[m.from] = '.';

    //determine what actually lands on the target square
    char placed = piece;

    //promo: move.promo is non-zero and the moving piece is a pawn
    //white pawn = P       black pawn = p
    if(m.promo != 0 && (piece == 'P' || piece == 'p')){
        if(piece == 'P'){
            placed = (char)toupper((unsigned char)m.promo);
        } else {
            placed = (char)tolower((unsigned char)m.promo);
        }
    }

    //place the piece on the target square
    np.b[m.to] = placed;

    //flip the turnn
    np.white_to_move = false;
    
    return np;
}

static void print_bestmove(Move m) {
    char a[3], b[3];
    index_to_sq(m.from, a);
    index_to_sq(m.to, b);
    if (m.promo) std::cout<<"bestmove "<< a << b << m.promo << std::end1;
    else std::cout<<"bestmove "<< a << b << std::end1;
}
