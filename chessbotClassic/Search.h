#ifndef SEARCH_H
#define SEARCH_H

#include "Move.h"
#include "Position.h"

/**
 * @class Search
 * @brief Negamax alpha-beta search for selecting the best chess move.
 *
 * This search layer uses:
 * - legal move generation from the existing framework
 * - static evaluation from Evaluate
 * - alpha-beta pruning
 * - fixed-depth search
 *
 * This version does not include:
 * - transposition tables
 * - iterative deepening
 * - quiescence search
 * - time management
 *
 * Those can be added later once the base engine is stable.
 */
class Search
{
public:
    /**
     * @brief Finds the best move for the current side to move.
     *
     * @param pos Current position.
     * @param depth Search depth in plies.
     * @return Best legal move found. If no legal move exists, returns a default move.
     */
    static Move findBestMove(const Position &pos, int depth);

private:
    /**
     * @brief Negamax search with alpha-beta pruning.
     *
     * @param pos Current position.
     * @param depth Remaining depth.
     * @param alpha Alpha bound.
     * @param beta Beta bound.
     * @return Score from the perspective of the side to move.
     */
    static int search(const Position &pos, int depth, int alpha, int beta);

    /**
     * @brief Converts static evaluation into side-to-move perspective.
     *
     * Evaluate::eval returns scores from White's perspective.
     * Negamax needs scores from the current side-to-move perspective.
     */
    static int scoreForSideToMove(const Position &pos);

    /**
     * @brief Large score used for checkmate detection.
     */
    static const int MATE_SCORE = 100000;

    /**
     * @brief Practical infinity for alpha-beta.
     */
    static const int INF = 1000000;
};

#endif