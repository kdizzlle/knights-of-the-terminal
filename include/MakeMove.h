#ifndef MAKEMOVE_H
#define MAKEMOVE_H

#include "Move.h"
#include "Position.h"

/**
 * @brief Applies a move to a position and returns the resulting position.
 *
 * This performs a simple board update:
 * - moves the piece
 * - handles promotion
 * - flips the side to move
 *
 * @param pos Original position.
 * @param m Move to apply.
 * @return New position after the move is made.
 */
Position makeMove(const Position &pos, const Move &m);

#endif