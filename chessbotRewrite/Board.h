#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <string>
#include <vector>
#include <cstdint>
#include "Types.h"
#include "Move.h"

struct UndoState {
    int capturedPiece = NO_PIECE;
    int castlingRights = 0;
    int epSquare = -1;
    int halfmoveClock = 0;
    uint64_t key = 0;
};

class Board {
public:
    Board();
    void setStartPos();
    bool setFromFen(const std::string& fen);

    Color sideToMove() const { return side; }
    uint64_t occupancy() const { return colors[WHITE] | colors[BLACK]; }
    uint64_t occupancy(Color c) const { return colors[c]; }
    uint64_t piecesBB(Piece p) const { return pieces[p]; }
    int pieceOn(int sq) const { return board[sq]; }
    int kingSquare(Color c) const;

    bool inCheck(Color c) const;
    bool isSquareAttacked(int sq, Color by) const;

    bool makeMove(Move m, UndoState& u);
    void unmakeMove(Move m, const UndoState& u);

    std::vector<uint64_t> keyHistory;

    int castlingRights = 0;
    int epSquare = -1;
    int halfmoveClock = 0;
    int fullmoveNumber = 1;
    uint64_t key = 0;

private:
    std::array<int, 64> board;
    std::array<uint64_t, PIECE_NB> pieces;
    std::array<uint64_t, COLOR_NB> colors;
    Color side = WHITE;

    void clear();
    void putPiece(int piece, int sq);
    void removePiece(int piece, int sq);
    void movePiece(int piece, int from, int to);
    void updateKey();
    friend uint64_t Zobrist_hash_adapter(const Board&);
};

#endif
