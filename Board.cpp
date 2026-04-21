#include "Board.h"
#include "Precomputed.h"
#include "Bitboard.h"
#include "Zobrist.h"
#include <sstream>
#include <cctype>

namespace {
    bool sameColorPiece(int p, Color c) {
        if (p == NO_PIECE) return false;
        return (p < 6) == (c == WHITE);
    }
    PieceType typeOf(int p) { return p == NO_PIECE ? NO_PIECE_TYPE : static_cast<PieceType>(p % 6); }
    Color colorOf(int p) { return p < 6 ? WHITE : BLACK; }
    int makePiece(Color c, PieceType pt) { return (c == WHITE ? 0 : 6) + pt; }
    char pieceChar(int p) {
        static const char map[] = {'P','N','B','R','Q','K','p','n','b','r','q','k'};
        return p == NO_PIECE ? '.' : map[p];
    }
}

Board::Board(){ Precomputed::init(); Zobrist::init(); setStartPos(); }

void Board::clear() {
    board.fill(NO_PIECE);
    pieces.fill(0ULL);
    colors.fill(0ULL);
    side = WHITE;
    castlingRights = 0;
    epSquare = -1;
    halfmoveClock = 0;
    fullmoveNumber = 1;
    key = 0;
    keyHistory.clear();
}

void Board::putPiece(int piece, int sq) {
    board[sq] = piece;
    pieces[piece] |= bb_of(sq);
    colors[colorOf(piece)] |= bb_of(sq);
}
void Board::removePiece(int piece, int sq) {
    board[sq] = NO_PIECE;
    pieces[piece] &= ~bb_of(sq);
    colors[colorOf(piece)] &= ~bb_of(sq);
}
void Board::movePiece(int piece, int from, int to) {
    pieces[piece] ^= bb_of(from) | bb_of(to);
    colors[colorOf(piece)] ^= bb_of(from) | bb_of(to);
    board[from] = NO_PIECE;
    board[to] = piece;
}

int Board::kingSquare(Color c) const {
    uint64_t bb = pieces[makePiece(c, KING)];
    if (!bb) return -1;
#if defined(__GNUG__)
    return __builtin_ctzll(bb);
#else
    int sq=0; while(((bb>>sq)&1ULL)==0)++sq; return sq;
#endif
}

bool Board::isSquareAttacked(int sq, Color by) const {
    uint64_t occ = occupancy();
    if (Precomputed::pawnAttacks[!by][sq] & pieces[makePiece(by, PAWN)]) return true;
    if (Precomputed::knightAttacks[sq] & pieces[makePiece(by, KNIGHT)]) return true;
    if (Precomputed::kingAttacks[sq] & pieces[makePiece(by, KING)]) return true;

    uint64_t batt = Precomputed::bishopAttacks(sq, occ);
    uint64_t ratt = Precomputed::rookAttacks(sq, occ);
    if (batt & (pieces[makePiece(by, BISHOP)] | pieces[makePiece(by, QUEEN)])) return true;
    if (ratt & (pieces[makePiece(by, ROOK)] | pieces[makePiece(by, QUEEN)])) return true;
    return false;
}

bool Board::inCheck(Color c) const { int k = kingSquare(c); return k >= 0 && isSquareAttacked(k, !c); }

void Board::updateKey() { key = Zobrist::hash(*this); }

void Board::setStartPos() {
    setFromFen("rn1qkbnr/pppb1ppp/4p3/3p4/8/5NP1/PPPPPPBP/RNBQ1RK1 w kq - 0 1");
    // immediately override with standard startpos
    setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool Board::setFromFen(const std::string& fen) {
    clear();
    std::istringstream ss(fen);
    std::string boardPart, stm, castle, ep;
    ss >> boardPart >> stm >> castle >> ep >> halfmoveClock >> fullmoveNumber;
    if (boardPart.empty()) return false;
    int r = 7, f = 0;
    for (char c : boardPart) {
        if (c == '/') { --r; f = 0; continue; }
        if (std::isdigit(static_cast<unsigned char>(c))) { f += c - '0'; continue; }
        Piece pc = NO_PIECE;
        switch (c) {
            case 'P': pc=W_PAWN; break; case 'N': pc=W_KNIGHT; break; case 'B': pc=W_BISHOP; break;
            case 'R': pc=W_ROOK; break; case 'Q': pc=W_QUEEN; break; case 'K': pc=W_KING; break;
            case 'p': pc=B_PAWN; break; case 'n': pc=B_KNIGHT; break; case 'b': pc=B_BISHOP; break;
            case 'r': pc=B_ROOK; break; case 'q': pc=B_QUEEN; break; case 'k': pc=B_KING; break;
            default: return false;
        }
        putPiece(pc, make_sq(f, r));
        ++f;
    }
    side = (stm == "b") ? BLACK : WHITE;
    if (castle.find('K') != std::string::npos) castlingRights |= WHITE_OO;
    if (castle.find('Q') != std::string::npos) castlingRights |= WHITE_OOO;
    if (castle.find('k') != std::string::npos) castlingRights |= BLACK_OO;
    if (castle.find('q') != std::string::npos) castlingRights |= BLACK_OOO;
    if (ep != "-" && ep.size() == 2) epSquare = make_sq(ep[0]-'a', ep[1]-'1');
    updateKey();
    keyHistory.push_back(key);
    return true;
}

bool Board::makeMove(Move m, UndoState& u) {
    u.capturedPiece = NO_PIECE;
    u.castlingRights = castlingRights;
    u.epSquare = epSquare;
    u.halfmoveClock = halfmoveClock;
    u.key = key;

    int from = m.from(), to = m.to();
    int piece = board[from];
    if (piece == NO_PIECE || colorOf(piece) != side) return false;
    int captured = board[to];
    epSquare = -1;
    if (captured != NO_PIECE) {
        u.capturedPiece = captured;
        removePiece(captured, to);
        halfmoveClock = 0;
    }
    if (typeOf(piece) == PAWN) halfmoveClock = 0; else ++halfmoveClock;

    if (m.flag() == Move::EN_PASSANT) {
        int capSq = side == WHITE ? to - 8 : to + 8;
        int capPiece = board[capSq];
        u.capturedPiece = capPiece;
        removePiece(capPiece, capSq);
    }

    movePiece(piece, from, to);

    if (typeOf(piece) == PAWN) {
        if (m.flag() == Move::DOUBLE_PAWN) epSquare = side == WHITE ? to - 8 : to + 8;
        if (m.isPromotion()) {
            removePiece(piece, to);
            PieceType pt = QUEEN;
            switch (m.flag() & 3) {
                case 0: pt = KNIGHT; break;
                case 1: pt = BISHOP; break;
                case 2: pt = ROOK; break;
                default: pt = QUEEN; break;
            }
            putPiece(makePiece(side, pt), to);
        }
    }

    if (m.flag() == Move::KING_CASTLE) {
        if (side == WHITE) movePiece(W_ROOK, 7, 5);
        else movePiece(B_ROOK, 63, 61);
    } else if (m.flag() == Move::QUEEN_CASTLE) {
        if (side == WHITE) movePiece(W_ROOK, 0, 3);
        else movePiece(B_ROOK, 56, 59);
    }

    if (piece == W_KING) castlingRights &= ~(WHITE_OO | WHITE_OOO);
    if (piece == B_KING) castlingRights &= ~(BLACK_OO | BLACK_OOO);
    if (from == 0 || to == 0) castlingRights &= ~WHITE_OOO;
    if (from == 7 || to == 7) castlingRights &= ~WHITE_OO;
    if (from == 56 || to == 56) castlingRights &= ~BLACK_OOO;
    if (from == 63 || to == 63) castlingRights &= ~BLACK_OO;

    if (side == BLACK) ++fullmoveNumber;
    side = !side;
    updateKey();

    keyHistory.push_back(key);
    if (inCheck(!side)) {
        unmakeMove(m, u);
        return false;
    }
    return true;
}

void Board::unmakeMove(Move m, const UndoState& u) {
    keyHistory.pop_back();
    side = !side;
    if (side == BLACK) --fullmoveNumber;
    castlingRights = u.castlingRights;
    epSquare = u.epSquare;
    halfmoveClock = u.halfmoveClock;
    key = u.key;

    int from = m.from(), to = m.to();
    int moved = board[to];
    if (m.isPromotion()) {
        removePiece(moved, to);
        moved = makePiece(side, PAWN);
        putPiece(moved, to);
    }

    if (m.flag() == Move::KING_CASTLE) {
        if (side == WHITE) movePiece(W_ROOK, 5, 7);
        else movePiece(B_ROOK, 61, 63);
    } else if (m.flag() == Move::QUEEN_CASTLE) {
        if (side == WHITE) movePiece(W_ROOK, 3, 0);
        else movePiece(B_ROOK, 59, 56);
    }

    movePiece(moved, to, from);

    if (m.flag() == Move::EN_PASSANT) {
        int capSq = side == WHITE ? to - 8 : to + 8;
        putPiece(makePiece(!side, PAWN), capSq);
    } else if (u.capturedPiece != NO_PIECE) {
        putPiece(u.capturedPiece, to);
    }
}
