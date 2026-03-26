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

static void genQueen(const Position *p, int from, bool white, Move *moves, int *n)
{
    static const int dirs[8][2] = {
        { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1},
        { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1}
    };

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
            char target = p->b[to];

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

static bool is_square_attacked(const Position *p, int sq, bool by_white) {
    int r = sq / 8, f = sq % 8;

    // pawns
    if (by_white) {
        if (r > 0 && f > 0 && p->b[(r - 1) * 8 + (f - 1)] == 'P') return true;
        if (r > 0 && f < 7 && p->b[(r - 1) * 8 + (f + 1)] == 'P') return true;
    } else {
        if (r < 7 && f > 0 && p->b[(r + 1) * 8 + (f - 1)] == 'p') return true;
        if (r < 7 && f < 7 && p->b[(r + 1) * 8 + (f + 1)] == 'p') return true;
    }

    // knights
    static const int nd[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int i = 0; i < 8; i++) {
        int to = sq + nd[i];
        if (to < 0 || to >= 64) continue;
        int tr = to / 8, tf = to % 8;
        int dr = tr - r;
        if (dr < 0) dr = -dr;
        int df = tf - f;
        if (df < 0) df = -df;
        if (!((dr == 1 && df == 2) || (dr == 2 && df == 1))) continue;
        char pc = p->b[to];
        if (by_white && pc == 'N') return true;
        if (!by_white && pc == 'n') return true;
    }

    // sliders
    static const int dirs[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (int di = 0; di < 8; di++) {
        int df = dirs[di][0], dr = dirs[di][1];
        int cr = r + dr, cf = f + df;
        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8) {
            int idx = cr * 8 + cf;
            char pc = p->b[idx];
            if (pc != '.') {
                int pc_white = is_white_piece(pc);
                if (pc_white == by_white) {
                    char up = static_cast<char>(std::toupper(static_cast<unsigned char>(pc)));
                    int rook_dir = (di < 4);
                    int bishop_dir = (di >= 4);
                    if (up == 'Q') return true;
                    if (rook_dir && up == 'R') return true;
                    if (bishop_dir && up == 'B') return true;
                    if (up == 'K' && (std::abs(cr - r) <= 1 && std::abs(cf - f) <= 1)) return true;
                }
                break;
            }
            cr += dr;
            cf += df;
        }
    }

    // king adjacency (extra safety)
    for (int rr = r - 1; rr <= r + 1; rr++) {
        for (int ff = f - 1; ff <= f + 1; ff++) {
            if (rr < 0 || rr >= 8 || ff < 0 || ff >= 8) continue;
            if (rr == r && ff == f) continue;
            char pc = p->b[rr * 8 + ff];
            if (by_white && pc == 'K') return true;
            if (!by_white && pc == 'k') return true;
        }
    }

    return false;
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
