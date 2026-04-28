#include "ChessEngine.hpp"
#include "InternalUtils.hpp"

#include <algorithm>

namespace Chess::Core {
void MoveOrdering::ClearHistory() { History = {}; }
void MoveOrdering::ClearKillers() { killerMoves = {}; }
void MoveOrdering::Clear() { ClearKillers(); ClearHistory(); }

int MoveOrdering::GetPieceValue(int pieceType) {
    switch (pieceType) {
        case Piece::Queen: return Evaluation::QueenValue;
        case Piece::Rook: return Evaluation::RookValue;
        case Piece::Knight: return Evaluation::KnightValue;
        case Piece::Bishop: return Evaluation::BishopValue;
        case Piece::Pawn: return Evaluation::PawnValue;
        default: return 0;
    }
}

void MoveOrdering::OrderMoves(Move hashMove, Board& board, std::span<Move> moves, std::uint64_t oppAttacks, std::uint64_t oppPawnAttacks, bool inQSearch, int ply) {
    for (std::size_t i = 0; i < moves.size(); i++) {
        Move move = moves[i];
        if (Move::SameMove(move, hashMove)) {
            moveScores[i] = hashMoveScore;
            continue;
        }
        int score = 0;
        int startSquare = move.StartSquare();
        int targetSquare = move.TargetSquare();
        int movePiece = board.Square[startSquare];
        int movePieceType = Piece::PieceType(movePiece);
        int capturePieceType = Piece::PieceType(board.Square[targetSquare]);
        bool isCapture = capturePieceType != Piece::None;
        int flag = move.MoveFlag();
        int pieceValue = GetPieceValue(movePieceType);

        if (isCapture) {
            int captureMaterialDelta = GetPieceValue(capturePieceType) - pieceValue;
            bool opponentCanRecapture = BitBoardUtility::ContainsSquare(oppPawnAttacks | oppAttacks, targetSquare);
            if (opponentCanRecapture) score += (captureMaterialDelta >= 0 ? winningCaptureBias : losingCaptureBias) + captureMaterialDelta;
            else score += winningCaptureBias + captureMaterialDelta;
        }

        if (movePieceType == Piece::Pawn) {
            if (flag == Move::PromoteToQueenFlag && !isCapture) score += promoteBias;
        } else if (movePieceType == Piece::King) {
        } else {
            int toScore = PieceSquareTable::Read(movePiece, targetSquare);
            int fromScore = PieceSquareTable::Read(movePiece, startSquare);
            score += toScore - fromScore;
            if (BitBoardUtility::ContainsSquare(oppPawnAttacks, targetSquare)) score -= 50;
            else if (BitBoardUtility::ContainsSquare(oppAttacks, targetSquare)) score -= 25;
        }

        if (!isCapture) {
            bool isKiller = !inQSearch && ply < maxKillerMovePly && killerMoves[static_cast<std::size_t>(ply)].Match(move);
            score += isKiller ? killerBias : regularBias;
            score += History[board.MoveColourIndex()][move.StartSquare()][move.TargetSquare()];
        }
        moveScores[i] = score;
    }
    if (!moves.empty()) {
        std::span<int> scores(moveScores.data(), moves.size());
        Detail::sort_moves_desc(moves, scores, 0, static_cast<int>(moves.size()) - 1);
    }
}

Searcher::Searcher(Board& board_)
    : transpositionTable(board_, transpositionTableSizeMB), moveOrderer(moveGenerator, transpositionTable), board(board_) {
    moveGenerator.promotionsToGenerate = MoveGenerator::PromotionMode::QueenAndKnight;
}

Move Searcher::BestMoveSoFar() const { return bestMove; }
int Searcher::BestEvalSoFar() const { return bestEval; }
} // namespace Chess::Core
