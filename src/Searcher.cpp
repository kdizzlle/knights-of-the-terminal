#include "ChessEngine.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

namespace Chess::Core {
void Searcher::StartSearch() {
    bestEvalThisIteration = bestEval = 0;
    bestMoveThisIteration = bestMove = Move::NullMove();
    isPlayingWhite = board.IsWhiteToMove;
    moveOrderer.ClearHistory();
    repetitionTable.Init(board);
    CurrentDepth = 0;
    debugInfo = "Starting search with FEN " + FenUtility::CurrentFen(board);
    bool cancelledBeforeStart = searchCancelled.exchange(false);
    searchDiagnostics = SearchDiagnostics();
    searchIterationTimer = std::chrono::steady_clock::now();
    searchTotalTimer = std::chrono::steady_clock::now();
    if (!cancelledBeforeStart) {
        RunIterativeDeepeningSearch();
    } else {
        debugInfo += "\nSearch cancelled before start";
    }
    if (bestMove.IsNull()) {
        auto moves = moveGenerator.GenerateMoves(board);
        if (!moves.empty()) bestMove = moves[0];
    }
    if (OnSearchComplete) OnSearchComplete(bestMove);
    searchCancelled = false;
}

void Searcher::RunIterativeDeepeningSearch() {
    for (int searchDepth = 1; searchDepth <= 256; searchDepth++) {
        hasSearchedAtLeastOneMove = false;
        debugInfo += "\nStarting Iteration: " + std::to_string(searchDepth);
        searchIterationTimer = std::chrono::steady_clock::now();
        currentIterationDepth = searchDepth;
        Search(searchDepth, 0, negativeInfinity, positiveInfinity);
        if (searchCancelled) {
            if (hasSearchedAtLeastOneMove) {
                bestMove = bestMoveThisIteration;
                bestEval = bestEvalThisIteration;
                searchDiagnostics.move = MoveUtility::GetMoveNameUCI(bestMove);
                searchDiagnostics.eval = bestEval;
                searchDiagnostics.moveIsFromPartialSearch = true;
                debugInfo += "\nUsing partial search result: " + MoveUtility::GetMoveNameUCI(bestMove) + " Eval: " + std::to_string(bestEval);
            }
            debugInfo += "\nSearch aborted";
            break;
        } else {
            CurrentDepth = searchDepth;
            bestMove = bestMoveThisIteration;
            bestEval = bestEvalThisIteration;
            debugInfo += "\nIteration result: " + MoveUtility::GetMoveNameUCI(bestMove) + " Eval: " + std::to_string(bestEval);
            if (IsMateScore(bestEval)) debugInfo += " Mate in ply: " + std::to_string(NumPlyToMateFromScore(bestEval));
            bestEvalThisIteration = std::numeric_limits<int>::min();
            bestMoveThisIteration = Move::NullMove();
            searchDiagnostics.numCompletedIterations = searchDepth;
            searchDiagnostics.move = MoveUtility::GetMoveNameUCI(bestMove);
            searchDiagnostics.eval = bestEval;
            if (IsMateScore(bestEval) && NumPlyToMateFromScore(bestEval) <= searchDepth) {
                debugInfo += "\nExitting search due to mate found within search depth";
                break;
            }
        }
    }
}

std::pair<Move,int> Searcher::GetSearchResult() const { return {bestMove, bestEval}; }
void Searcher::EndSearch() { searchCancelled = true; }

int Searcher::Search(int plyRemaining, int plyFromRoot, int alpha, int beta, int numExtensions, Move prevMove, bool prevWasCapture) {
    if (searchCancelled) return 0;

    if (plyFromRoot > 0) {
        if (board.CurrentGameState.fiftyMoveCounter >= 100 || repetitionTable.Contains(board.CurrentGameState.zobristKey)) {
            return 0;
        }
        alpha = std::max(alpha, -immediateMateScore + plyFromRoot);
        beta = std::min(beta, immediateMateScore - plyFromRoot);
        if (alpha >= beta) return alpha;
    }

    int ttVal = transpositionTable.LookupEvaluation(plyRemaining, plyFromRoot, alpha, beta);
    if (ttVal != TranspositionTable::LookupFailed) {
        if (plyFromRoot == 0) {
            bestMoveThisIteration = transpositionTable.TryGetStoredMove();
            bestEvalThisIteration = transpositionTable.entries[static_cast<std::size_t>(transpositionTable.Index())].value;
        }
        return ttVal;
    }

    if (plyRemaining == 0) return QuiescenceSearch(alpha, beta);

    std::array<Move,256> moveBuf{};
    std::span<Move> moves(moveBuf);
    moveGenerator.GenerateMoves(board, moves, false);
    Move prevBestMove = plyFromRoot == 0 ? bestMove : transpositionTable.TryGetStoredMove();
    moveOrderer.OrderMoves(prevBestMove, board, moves, moveGenerator.opponentAttackMap, moveGenerator.opponentPawnAttackMap, false, plyFromRoot);
    if (moves.empty()) {
        if (moveGenerator.InCheck()) {
            int mateScore = immediateMateScore - plyFromRoot;
            return -mateScore;
        }
        return 0;
    }

    if (plyFromRoot > 0) {
        bool wasPawnMove = Piece::PieceType(board.Square[prevMove.TargetSquare()]) == Piece::Pawn;
        repetitionTable.Push(board.CurrentGameState.zobristKey, prevWasCapture || wasPawnMove);
    }

    int evaluationBound = TranspositionTable::UpperBound;
    Move bestMoveInThisPosition = Move::NullMove();

    for (std::size_t i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        int capturedPieceType = Piece::PieceType(board.Square[move.TargetSquare()]);
        bool isCapture = capturedPieceType != Piece::None;
        board.MakeMove(move, true);

        int extension = 0;
        if (numExtensions < maxExtentions) {
            int movedPieceType = Piece::PieceType(board.Square[move.TargetSquare()]);
            int targetRank = BoardHelper::RankIndex(move.TargetSquare());
            if (board.IsInCheck()) extension = 1;
            else if (movedPieceType == Piece::Pawn && (targetRank == 1 || targetRank == 6)) extension = 1;
        }

        bool needsFullSearch = true;
        int eval = 0;
        if (extension == 0 && plyRemaining >= 3 && static_cast<int>(i) >= 3 && !isCapture) {
            constexpr int reduceDepth = 1;
            eval = -Search(plyRemaining - 1 - reduceDepth, plyFromRoot + 1, -alpha - 1, -alpha, numExtensions, move, isCapture);
            needsFullSearch = eval > alpha;
        }
        if (needsFullSearch) {
            eval = -Search(plyRemaining - 1 + extension, plyFromRoot + 1, -beta, -alpha, numExtensions + extension, move, isCapture);
        }
        board.UnmakeMove(move, true);

        if (searchCancelled) return 0;
        if (eval >= beta) {
            transpositionTable.StoreEvaluation(plyRemaining, plyFromRoot, beta, TranspositionTable::LowerBound, move);
            if (!isCapture) {
                if (plyFromRoot < MoveOrdering::maxKillerMovePly) moveOrderer.killerMoves[static_cast<std::size_t>(plyFromRoot)].Add(move);
                int historyScore = plyRemaining * plyRemaining;
                moveOrderer.History[board.MoveColourIndex()][move.StartSquare()][move.TargetSquare()] += historyScore;
            }
            if (plyFromRoot > 0) repetitionTable.TryPop();
            searchDiagnostics.numCutOffs++;
            return beta;
        }
        if (eval > alpha) {
            evaluationBound = TranspositionTable::Exact;
            bestMoveInThisPosition = move;
            alpha = eval;
            if (plyFromRoot == 0) {
                bestMoveThisIteration = move;
                bestEvalThisIteration = eval;
                hasSearchedAtLeastOneMove = true;
            }
        }
    }

    if (plyFromRoot > 0) repetitionTable.TryPop();
    transpositionTable.StoreEvaluation(plyRemaining, plyFromRoot, alpha, evaluationBound, bestMoveInThisPosition);
    return alpha;
}

int Searcher::QuiescenceSearch(int alpha, int beta) {
    if (searchCancelled) return 0;
    int eval = evaluation.Evaluate(board);
    searchDiagnostics.numPositionsEvaluated++;
    if (eval >= beta) {
        searchDiagnostics.numCutOffs++;
        return beta;
    }
    if (eval > alpha) alpha = eval;

    std::array<Move,128> moveBuf{};
    std::span<Move> moves(moveBuf);
    moveGenerator.GenerateMoves(board, moves, true);
    moveOrderer.OrderMoves(Move::NullMove(), board, moves, moveGenerator.opponentAttackMap, moveGenerator.opponentPawnAttackMap, true, 0);
    for (std::size_t i = 0; i < moves.size(); i++) {
        board.MakeMove(moves[i], true);
        eval = -QuiescenceSearch(-beta, -alpha);
        board.UnmakeMove(moves[i], true);
        if (eval >= beta) {
            searchDiagnostics.numCutOffs++;
            return beta;
        }
        if (eval > alpha) alpha = eval;
    }
    return alpha;
}

bool Searcher::IsMateScore(int score) {
    if (score == std::numeric_limits<int>::min()) return false;
    constexpr int maxMateDepth = 1000;
    return std::abs(score) > immediateMateScore - maxMateDepth;
}

int Searcher::NumPlyToMateFromScore(int score) { return immediateMateScore - std::abs(score); }

std::string Searcher::AnnounceMate() {
    if (IsMateScore(bestEvalThisIteration)) {
        int numPlyToMate = NumPlyToMateFromScore(bestEvalThisIteration);
        int numMovesToMate = static_cast<int>(std::ceil(numPlyToMate / 2.f));
        std::string sideWithMate = (bestEvalThisIteration * (board.IsWhiteToMove ? 1 : -1) < 0) ? "Black" : "White";
        return sideWithMate + " can mate in " + std::to_string(numMovesToMate) + " move" + (numMovesToMate > 1 ? "s" : "");
    }
    return "No mate found";
}

void Searcher::ClearForNewPosition() { transpositionTable.Clear(); moveOrderer.ClearKillers(); }
TranspositionTable& Searcher::GetTranspositionTable() { return transpositionTable; }

} // namespace Chess::Core
