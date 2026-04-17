#include "MakeMove.h"

#include <cctype>

static bool isWhitePiece(char p)
{
    return p >= 'A' && p <= 'Z';
}

static bool isBlackPiece(char p)
{
    return p >= 'a' && p <= 'z';
}

Position makeMove(const Position &pos, const Move &m)
{
    Position np = pos;

    const char piece = np.b[m.from];
    const char captured = np.b[m.to];

    // Clear source square
    np.b[m.from] = '.';

    // Determine placed piece (promotion or normal move)
    char placed = piece;
    if (m.promo != 0 && (piece == 'P' || piece == 'p'))
    {
        if (piece == 'P')
            placed = static_cast<char>(std::toupper(static_cast<unsigned char>(m.promo)));
        else
            placed = static_cast<char>(std::tolower(static_cast<unsigned char>(m.promo)));
    }

    // Move the main piece
    np.b[m.to] = placed;

    // ---------------------------------------------------------------------
    // Castling rook movement
    // ---------------------------------------------------------------------
    if (piece == 'K')
    {
        // White kingside: e1 -> g1, rook h1 -> f1
        if (m.from == 4 && m.to == 6)
        {
            np.b[7] = '.';
            np.b[5] = 'R';
        }
        // White queenside: e1 -> c1, rook a1 -> d1
        else if (m.from == 4 && m.to == 2)
        {
            np.b[0] = '.';
            np.b[3] = 'R';
        }
    }
    else if (piece == 'k')
    {
        // Black kingside: e8 -> g8, rook h8 -> f8
        if (m.from == 60 && m.to == 62)
        {
            np.b[63] = '.';
            np.b[61] = 'r';
        }
        // Black queenside: e8 -> c8, rook a8 -> d8
        else if (m.from == 60 && m.to == 58)
        {
            np.b[56] = '.';
            np.b[59] = 'r';
        }
    }

    // ---------------------------------------------------------------------
    // Update castling rights when king moves
    // ---------------------------------------------------------------------
    if (piece == 'K')
    {
        np.white_can_castle_king = false;
        np.white_can_castle_queen = false;
    }
    else if (piece == 'k')
    {
        np.black_can_castle_king = false;
        np.black_can_castle_queen = false;
    }

    // ---------------------------------------------------------------------
    // Update castling rights when rooks move
    // ---------------------------------------------------------------------
    if (piece == 'R')
    {
        if (m.from == 0)
            np.white_can_castle_queen = false;
        else if (m.from == 7)
            np.white_can_castle_king = false;
    }
    else if (piece == 'r')
    {
        if (m.from == 56)
            np.black_can_castle_queen = false;
        else if (m.from == 63)
            np.black_can_castle_king = false;
    }

    // ---------------------------------------------------------------------
    // Update castling rights when rooks are captured
    // ---------------------------------------------------------------------
    if (captured == 'R')
    {
        if (m.to == 0)
            np.white_can_castle_queen = false;
        else if (m.to == 7)
            np.white_can_castle_king = false;
    }
    else if (captured == 'r')
    {
        if (m.to == 56)
            np.black_can_castle_queen = false;
        else if (m.to == 63)
            np.black_can_castle_king = false;
    }

    // ---------------------------------------------------------------------
    // Halfmove clock
    // Reset on pawn move or capture, otherwise increment
    // ---------------------------------------------------------------------
    if (piece == 'P' || piece == 'p' || captured != '.')
        np.halfmove_clock = 0;
    else
        np.halfmove_clock = pos.halfmove_clock + 1;

    // ---------------------------------------------------------------------
    // Fullmove number
    // Increment after black makes a move
    // ---------------------------------------------------------------------
    np.fullmove_number = pos.fullmove_number;
    if (!pos.white_to_move)
        np.fullmove_number++;

    // Flip side to move
    np.white_to_move = !pos.white_to_move;

    return np;
}