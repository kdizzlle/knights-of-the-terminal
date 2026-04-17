#include "Searcher.h"
#include "MoveGenerator.h"
#include "Evaluation.h"
#include "Zobrist.h"
#include "RepetitionTable.h"
#include "MoveOrdering.h"
#include <chrono>
#include <vector>

namespace {
    using Clock = std::chrono::steady_clock;
    constexpr int INF = 1000000;
    constexpr int MATE = 100000;
    constexpr int MAX_PLY = 128;
    constexpr int TT_SIZE = 1 << 20;

    enum TTFlag { TT_EXACT = 0, TT_ALPHA = 1, TT_BETA = 2 };

    struct TTEntry {
        uint64_t key = 0;
        int depth = -1;
        int score = 0;
        int flag = TT_EXACT;
        Move best;
        bool valid = false;
    };

    TTEntry tt[TT_SIZE];
    Clock::time_point deadline;
    bool stopSearch = false;
    RepetitionTable repetitionTable;
    int nodeCount = 0;

    bool timeUp() {
        if (stopSearch) return true;
        if (Clock::now() >= deadline) {
            stopSearch = true;
            return true;
        }
        return false;
    }

    int scoreForSideToMove(const Board& board) {
        int score = Evaluation::evaluate(board);
        return board.sideToMove() == WHITE ? score : -score;
    }

    int toTTScore(int score, int ply) {
        if (score > MATE - 1000) return score + ply;
        if (score < -MATE + 1000) return score - ply;
        return score;
    }

    int fromTTScore(int score, int ply) {
        if (score > MATE - 1000) return score - ply;
        if (score < -MATE + 1000) return score + ply;
        return score;
    }

    void clearTT() {
        for (int i = 0; i < TT_SIZE; ++i) tt[i] = TTEntry();
    }

    void storeTT(uint64_t key, int depth, int score, int flag, const Move& bestMove, int ply) {
        TTEntry& e = tt[key & (TT_SIZE - 1)];
        if (!e.valid || depth >= e.depth) {
            e.key = key;
            e.depth = depth;
            e.score = toTTScore(score, ply);
            e.flag = flag;
            e.best = bestMove;
            e.valid = true;
        }
    }

    bool probeTT(uint64_t key, int depth, int alpha, int beta, int ply, int& outScore, Move& outMove) {
        TTEntry& e = tt[key & (TT_SIZE - 1)];
        if (!e.valid || e.key != key) return false;
        outMove = e.best;
        if (e.depth < depth) return false;
        int score = fromTTScore(e.score, ply);
        if (e.flag == TT_EXACT) { outScore = score; return true; }
        if (e.flag == TT_ALPHA && score <= alpha) { outScore = score; return true; }
        if (e.flag == TT_BETA && score >= beta) { outScore = score; return true; }
        return false;
    }

    int quiescence(Board& board, int alpha, int beta, int ply) {
        if (timeUp()) return scoreForSideToMove(board);

        int standPat = scoreForSideToMove(board);
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;

        std::vector<Move> moves;
        MoveGenerator::generateTacticalMoves(board, moves);
        MoveOrdering::orderMoves(board, moves, ply, Move());

        for (const Move& m : moves) {
            if (timeUp()) break;
            UndoState u;
            if (!board.makeMove(m, u)) continue;
            bool irreversible = m.isCapture() || board.pieceOn(m.to()) % 6 == PAWN || u.capturedPiece != NO_PIECE;
            repetitionTable.push(board.key, irreversible);
            int score = -quiescence(board, -beta, -alpha, ply + 1);
            repetitionTable.pop();
            board.unmakeMove(m, u);
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
        return alpha;
    }

    int search(Board& board, int depth, int alpha, int beta, int ply) {
        ++nodeCount;
        if ((nodeCount & 2047) == 0 && timeUp()) return scoreForSideToMove(board);
        if (timeUp()) return scoreForSideToMove(board);
        if (repetitionTable.contains(board.key) || board.halfmoveClock >= 100) return 0;
        if (depth <= 0) return quiescence(board, alpha, beta, ply);

        uint64_t key = board.key;
        Move ttMove;
        int ttScore;
        if (probeTT(key, depth, alpha, beta, ply, ttScore, ttMove)) return ttScore;

        std::vector<Move> moves;
        MoveGenerator::generateLegalMoves(board, moves);
        if (moves.empty()) return board.inCheck(board.sideToMove()) ? -MATE + ply : 0;

        MoveOrdering::orderMoves(board, moves, ply, ttMove);

        const int alphaStart = alpha;
        int bestScore = -INF;
        Move bestMove;
        bool firstMove = true;

        for (size_t i = 0; i < moves.size(); ++i) {
            const Move m = moves[i];
            UndoState u;
            if (!board.makeMove(m, u)) continue;

            bool movedPawn = (board.pieceOn(m.to()) != NO_PIECE) && ((board.pieceOn(m.to()) % 6) == PAWN);
            bool irreversible = m.isCapture() || movedPawn || u.capturedPiece != NO_PIECE;
            repetitionTable.push(board.key, irreversible);

            int extension = 0;
            if (board.inCheck(board.sideToMove()) || m.isPromotion()) extension = 1;

            int newDepth = depth - 1 + extension;
            int score;
            if (!firstMove && depth >= 3 && i >= 4 && !m.isCapture() && !m.isPromotion() && extension == 0) {
                score = -search(board, newDepth - 1, -alpha - 1, -alpha, ply + 1);
                if (!stopSearch && score > alpha) {
                    score = -search(board, newDepth, -beta, -alpha, ply + 1);
                }
            } else if (firstMove) {
                score = -search(board, newDepth, -beta, -alpha, ply + 1);
                firstMove = false;
            } else {
                score = -search(board, newDepth, -alpha - 1, -alpha, ply + 1);
                if (!stopSearch && score > alpha && score < beta) {
                    score = -search(board, newDepth, -beta, -alpha, ply + 1);
                }
            }

            repetitionTable.pop();
            board.unmakeMove(m, u);

            if (stopSearch) return alpha;

            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
            }
            if (score > alpha) alpha = score;
            if (alpha >= beta) {
                if (!m.isCapture() && !m.isPromotion()) {
                    MoveOrdering::addKiller(m, ply);
                    MoveOrdering::addHistoryBonus(m, depth);
                }
                storeTT(key, depth, beta, TT_BETA, m, ply);
                return beta;
            }
        }

        int flag = TT_EXACT;
        if (bestScore <= alphaStart) flag = TT_ALPHA;
        else if (bestScore >= beta) flag = TT_BETA;
        storeTT(key, depth, bestScore, flag, bestMove, ply);
        return bestScore;
    }
}

namespace Searcher {
    void init() {
        Zobrist::init();
        MoveOrdering::clear();
        clearTT();
    }

    Move findBestMove(Board& board, int depth) {
        deadline = Clock::now() + std::chrono::hours(1);
        stopSearch = false;
        nodeCount = 0;
        repetitionTable.init(board.key);

        std::vector<Move> moves;
        MoveGenerator::generateLegalMoves(board, moves);
        if (moves.empty()) return Move();

        Move bestMove = moves[0];
        int bestScore = -INF;

        for (const Move& m : moves) {
            UndoState u;
            if (!board.makeMove(m, u)) continue;
            bool movedPawn = (board.pieceOn(m.to()) != NO_PIECE) && ((board.pieceOn(m.to()) % 6) == PAWN);
            bool irreversible = m.isCapture() || movedPawn || u.capturedPiece != NO_PIECE;
            repetitionTable.push(board.key, irreversible);
            int score = -search(board, depth - 1, -INF, INF, 1);
            repetitionTable.pop();
            board.unmakeMove(m, u);
            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
            }
        }
        return bestMove;
    }

    Move findBestMoveTimed(Board& board, int timeMs) {
        int safe = timeMs > 50 ? timeMs - 25 : (timeMs > 0 ? timeMs : 1000);
        deadline = Clock::now() + std::chrono::milliseconds(safe);
        stopSearch = false;
        nodeCount = 0;
        repetitionTable.init(board.key);

        std::vector<Move> rootMoves;
        MoveGenerator::generateLegalMoves(board, rootMoves);
        if (rootMoves.empty()) return Move();

        Move bestMove = rootMoves[0];
        int previousScore = 0;

        for (int depth = 1; depth < 64 && !timeUp(); ++depth) {
            int alpha = depth == 1 ? -INF : previousScore - 30;
            int beta = depth == 1 ? INF : previousScore + 30;

            while (!timeUp()) {
                int bestScore = -INF;
                Move iterationBest = bestMove;

                for (const Move& m : rootMoves) {
                    if (timeUp()) break;
                    UndoState u;
                    if (!board.makeMove(m, u)) continue;
                    bool movedPawn = (board.pieceOn(m.to()) != NO_PIECE) && ((board.pieceOn(m.to()) % 6) == PAWN);
                    bool irreversible = m.isCapture() || movedPawn || u.capturedPiece != NO_PIECE;
                    repetitionTable.push(board.key, irreversible);
                    int score = -search(board, depth - 1, -beta, -alpha, 1);
                    repetitionTable.pop();
                    board.unmakeMove(m, u);
                    if (stopSearch) break;
                    if (score > bestScore) {
                        bestScore = score;
                        iterationBest = m;
                    }
                    if (score > alpha) alpha = score;
                }

                if (timeUp()) break;
                if (bestScore <= alpha - 30) { alpha -= 100; beta += 20; continue; }
                if (bestScore >= beta) { alpha -= 20; beta += 100; continue; }
                bestMove = iterationBest;
                previousScore = bestScore;
                break;
            }
        }

        return bestMove;
    }
}
