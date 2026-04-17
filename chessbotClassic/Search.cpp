#include "Search.h"

#include <vector>

#include "Attack.h"
#include "Evaluate.h"
#include "GenerateMoves.h"
#include "MakeMove.h"

namespace
{
    std::vector<Move> pseudoLegalMoves(const Position &pos)
    {
        Move buffer[256];
        Move *end = GenerateMoves::genAllMoves(pos, buffer);
        return std::vector<Move>(buffer, end);
    }

    std::vector<Move> legalMoves(const Position &pos)
    {
        std::vector<Move> out;

        for (const Move &m : pseudoLegalMoves(pos))
        {
            const Position np = makeMove(pos, m);

            // After makeMove, side-to-move has flipped.
            // We check whether the side that just moved is in check.
            if (!np.inCheck(!np.white_to_move))
                out.push_back(m);
        }

        return out;
    }
}

int Search::scoreForSideToMove(const Position &pos)
{
    const int whiteScore = Evaluate::eval(pos);
    return pos.white_to_move ? whiteScore : -whiteScore;
}

int Search::search(const Position &pos, int depth, int alpha, int beta)
{
    const std::vector<Move> moves = legalMoves(pos);

    if (depth == 0)
        return scoreForSideToMove(pos);

    if (moves.empty())
    {
        if (pos.inCheck(pos.white_to_move))
            return -MATE_SCORE + (10 - depth); // prefer faster mate
        return 0; // stalemate
    }

    int bestScore = -INF;

    for (const Move &m : moves)
    {
        const Position np = makeMove(pos, m);
        const int score = -search(np, depth - 1, -beta, -alpha);

        if (score > bestScore)
            bestScore = score;

        if (score > alpha)
            alpha = score;

        if (alpha >= beta)
            break;
    }

    return bestScore;
}

Move Search::findBestMove(const Position &pos, int depth)
{
    const std::vector<Move> moves = legalMoves(pos);

    if (moves.empty())
        return Move();

    Move bestMove = moves[0];
    int bestScore = -INF;
    int alpha = -INF;
    const int beta = INF;

    for (const Move &m : moves)
    {
        const Position np = makeMove(pos, m);
        const int score = -search(np, depth - 1, -beta, -alpha);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = m;
        }

        if (score > alpha)
            alpha = score;
    }

    return bestMove;
}