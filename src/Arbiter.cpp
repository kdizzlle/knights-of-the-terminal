#include "ChessEngine.hpp"

namespace Chess::Core {
bool Arbiter::IsDrawResult(GameResult result) {
    return result == GameResult::DrawByArbiter || result == GameResult::FiftyMoveRule || result == GameResult::Repetition || result == GameResult::Stalemate || result == GameResult::InsufficientMaterial;
}

bool Arbiter::IsWinResult(GameResult result) { return IsWhiteWinsResult(result) || IsBlackWinsResult(result); }

bool Arbiter::IsWhiteWinsResult(GameResult result) {
    return result == GameResult::BlackIsMated || result == GameResult::BlackTimeout || result == GameResult::BlackIllegalMove;
}

bool Arbiter::IsBlackWinsResult(GameResult result) {
    return result == GameResult::WhiteIsMated || result == GameResult::WhiteTimeout || result == GameResult::WhiteIllegalMove;
}

GameResult Arbiter::GetGameState(Board& board) {
    MoveGenerator moveGenerator;
    auto moves = moveGenerator.GenerateMoves(board);
    if (moves.empty()) {
        if (moveGenerator.InCheck()) return board.IsWhiteToMove ? GameResult::WhiteIsMated : GameResult::BlackIsMated;
        return GameResult::Stalemate;
    }
    if (board.FiftyMoveCounter() >= 100) return GameResult::FiftyMoveRule;
    int repCount = 0;
    for (std::uint64_t x : board.RepetitionPositionHistory) if (x == board.ZobristKey()) repCount++;
    if (repCount == 3) return GameResult::Repetition;
    if (InsufficentMaterial(board)) return GameResult::InsufficientMaterial;
    return GameResult::InProgress;
}

bool Arbiter::InsufficentMaterial(Board& board) {
    if (board.Pawns[Board::WhiteIndex].Count() > 0 || board.Pawns[Board::BlackIndex].Count() > 0) return false;
    if (board.FriendlyOrthogonalSliders != 0 || board.EnemyOrthogonalSliders != 0) return false;
    int numWhiteBishops = board.Bishops[Board::WhiteIndex].Count();
    int numBlackBishops = board.Bishops[Board::BlackIndex].Count();
    int numWhiteKnights = board.Knights[Board::WhiteIndex].Count();
    int numBlackKnights = board.Knights[Board::BlackIndex].Count();
    int numWhiteMinors = numWhiteBishops + numWhiteKnights;
    int numBlackMinors = numBlackBishops + numBlackKnights;
    int numMinors = numWhiteMinors + numBlackMinors;
    if (numMinors <= 1) return true;
    if (numMinors == 2 && numWhiteBishops == 1 && numBlackBishops == 1) {
        bool whiteBishopIsLightSquare = BoardHelper::LightSquare(board.Bishops[Board::WhiteIndex][0]);
        bool blackBishopIsLightSquare = BoardHelper::LightSquare(board.Bishops[Board::BlackIndex][0]);
        return whiteBishopIsLightSquare == blackBishopIsLightSquare;
    }
    return false;
}
} // namespace Chess::Core
