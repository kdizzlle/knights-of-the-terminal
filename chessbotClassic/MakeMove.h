#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "Move.h"
#include "Position.h"

/**
 * @brief Applies a move to a position and returns the resulting position.
 *
 * This function handles:
 * - normal moves
 * - captures
 * - promotions
 * - castling king movement
 * - castling rook movement
 * - castling-right updates
 * - side-to-move flip
 * - halfmove/fullmove counters
 *
 * This version does NOT implement en passant.
 *
 * @param pos Original position.
 * @param m Move to apply.
 * @return New position after the move is made.
 */
Position makeMove(const Position &pos, const Move &m);

#endif