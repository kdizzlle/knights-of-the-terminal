#ifndef ATTACK_H
#define ATTACK_H

#include "Position.h"

/**
 * @brief Finds the king square for the requested side.
 *
 * @param pos Position to inspect.
 * @param white True for white king, false for black king.
 * @return Board index of the king, or -1 if not found.
 */
int findKing(const Position &pos, bool white);

/**
 * @brief Checks whether a square is attacked by the specified side.
 *
 * @param pos Position to inspect.
 * @param sq Target square index.
 * @param by_white True if checking attacks by white, false for black.
 * @return True if the square is attacked, otherwise false.
 */
bool isSquareAttacked(const Position &pos, int sq, bool by_white);

/**
 * @brief Checks whether the specified side is currently in check.
 *
 * @param pos Position to inspect.
 * @param white True for white, false for black.
 * @return True if that side is in check, otherwise false.
 */
bool inCheck(const Position &pos, bool white);

#endif