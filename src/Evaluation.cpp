#include "ChessEngine.hpp"

#include <algorithm>

namespace Chess::Core {
int Evaluation::EvaluationData::Sum() const { return materialScore + mopUpScore + pieceSquareScore + pawnScore + pawnShieldScore; }

Evaluation::MaterialInfo::MaterialInfo(int numPawns_, int numKnights, int numBishops_, int numQueens_, int numRooks_, std::uint64_t myPawns, std::uint64_t enemyPawns_)
    : numPawns(numPawns_), numBishops(numBishops_), numQueens(numQueens_), numRooks(numRooks_), pawns(myPawns), enemyPawns(enemyPawns_) {
    numMajors = numRooks + numQueens;
    numMinors = numBishops + numKnights;
    materialScore = 0;
    materialScore += numPawns * PawnValue;
    materialScore += numKnights * KnightValue;
    materialScore += numBishops * BishopValue;
    materialScore += numRooks * RookValue;
    materialScore += numQueens * QueenValue;

    constexpr int queenEndgameWeight = 45;
    constexpr int rookEndgameWeight = 20;
    constexpr int bishopEndgameWeight = 10;
    constexpr int knightEndgameWeight = 10;
    constexpr int endgameStartWeight = 2 * rookEndgameWeight + 2 * bishopEndgameWeight + 2 * knightEndgameWeight + queenEndgameWeight;
    int endgameWeightSum = numQueens * queenEndgameWeight + numRooks * rookEndgameWeight + numBishops * bishopEndgameWeight + numKnights * knightEndgameWeight;
    endgameT = 1.f - std::min(1.f, endgameWeightSum / static_cast<float>(endgameStartWeight));
}

int Evaluation::Evaluate(Board& board_) {
    board = &board_;
    whiteEval = EvaluationData();
    blackEval = EvaluationData();

    MaterialInfo whiteMaterial = GetMaterialInfo(Board::WhiteIndex);
    MaterialInfo blackMaterial = GetMaterialInfo(Board::BlackIndex);

    whiteEval.materialScore = whiteMaterial.materialScore;
    blackEval.materialScore = blackMaterial.materialScore;
    whiteEval.pieceSquareScore = EvaluatePieceSquareTables(true, blackMaterial.endgameT);
    blackEval.pieceSquareScore = EvaluatePieceSquareTables(false, whiteMaterial.endgameT);
    whiteEval.mopUpScore = MopUpEval(true, whiteMaterial, blackMaterial);
    blackEval.mopUpScore = MopUpEval(false, blackMaterial, whiteMaterial);

    whiteEval.pawnScore = EvaluatePawns(Board::WhiteIndex);
    blackEval.pawnScore = EvaluatePawns(Board::BlackIndex);

    whiteEval.pawnShieldScore = KingPawnShield(Board::WhiteIndex, blackMaterial, static_cast<float>(blackEval.pieceSquareScore));
    blackEval.pawnShieldScore = KingPawnShield(Board::BlackIndex, whiteMaterial, static_cast<float>(whiteEval.pieceSquareScore));

    int perspective = board_.IsWhiteToMove ? 1 : -1;
    int eval = whiteEval.Sum() - blackEval.Sum();
    return eval * perspective;
}

int Evaluation::KingPawnShield(int colourIndex, const MaterialInfo& enemyMaterial, float enemyPieceSquareScore) {
    if (enemyMaterial.endgameT >= 1) return 0;

    int penalty = 0;
    bool isWhite = colourIndex == Board::WhiteIndex;
    int friendlyPawn = Piece::MakePiece(Piece::Pawn, isWhite);
    int kingSquare = board->KingSquare[colourIndex];
    int kingFile = BoardHelper::FileIndex(kingSquare);
    int uncastledKingPenalty = 0;

    if (kingFile <= 2 || kingFile >= 5) {
        const auto& squares = isWhite ? PrecomputedEvaluationData::PawnShieldSquaresWhite[kingSquare] : PrecomputedEvaluationData::PawnShieldSquaresBlack[kingSquare];
        for (std::size_t i = 0; i < squares.size() / 2; i++) {
            int shieldSquareIndex = squares[i];
            if (board->Square[shieldSquareIndex] != friendlyPawn) {
                if (squares.size() > 3 && board->Square[squares[i + 3]] == friendlyPawn) penalty += kingPawnShieldScores[i + 3];
                else penalty += kingPawnShieldScores[i];
            }
        }
        penalty *= penalty;
    } else {
        float enemyDevelopmentScore = std::clamp((enemyPieceSquareScore + 10.0f) / 130.0f, 0.0f, 1.0f);
        uncastledKingPenalty = static_cast<int>(50 * enemyDevelopmentScore);
    }

    int openFileAgainstKingPenalty = 0;
    if (enemyMaterial.numRooks > 1 || (enemyMaterial.numRooks > 0 && enemyMaterial.numQueens > 0)) {
        int clampedKingFile = std::clamp(kingFile, 1, 6);
        std::uint64_t myPawns = enemyMaterial.enemyPawns;
        for (int attackFile = clampedKingFile; attackFile <= clampedKingFile + 1; attackFile++) {
            std::uint64_t fileMask = Bits::FileMask[attackFile];
            bool isKingFile = attackFile == kingFile;
            if ((enemyMaterial.pawns & fileMask) == 0) {
                openFileAgainstKingPenalty += isKingFile ? 25 : 15;
                if ((myPawns & fileMask) == 0) {
                    openFileAgainstKingPenalty += isKingFile ? 15 : 10;
                }
            }
        }
    }

    float pawnShieldWeight = 1 - enemyMaterial.endgameT;
    if (board->Queens[1 - colourIndex].Count() == 0) pawnShieldWeight *= 0.6f;
    return static_cast<int>((-penalty - uncastledKingPenalty - openFileAgainstKingPenalty) * pawnShieldWeight);
}

int Evaluation::EvaluatePawns(int colourIndex) {
    PieceList& pawns = board->Pawns[colourIndex];
    bool isWhite = colourIndex == Board::WhiteIndex;
    std::uint64_t opponentPawns = board->PieceBitboards[Piece::MakePiece(Piece::Pawn, isWhite ? Piece::Black : Piece::White)];
    std::uint64_t friendlyPawns = board->PieceBitboards[Piece::MakePiece(Piece::Pawn, isWhite ? Piece::White : Piece::Black)];
    auto& masks = isWhite ? Bits::WhitePassedPawnMask : Bits::BlackPassedPawnMask;
    int bonus = 0;
    int numIsolatedPawns = 0;

    for (int i = 0; i < pawns.Count(); i++) {
        int square = pawns[i];
        std::uint64_t passedMask = masks[square];
        if ((opponentPawns & passedMask) == 0) {
            int rank = BoardHelper::RankIndex(square);
            int numSquaresFromPromotion = isWhite ? 7 - rank : rank;
            bonus += passedPawnBonuses[numSquaresFromPromotion];
        }
        if ((friendlyPawns & Bits::AdjacentFileMasks[BoardHelper::FileIndex(square)]) == 0) numIsolatedPawns++;
    }
    return bonus + isolatedPawnPenaltyByCount[numIsolatedPawns];
}

int Evaluation::MopUpEval(bool isWhite, const MaterialInfo& myMaterial, const MaterialInfo& enemyMaterial) {
    if (myMaterial.materialScore > enemyMaterial.materialScore + PawnValue * 2 && enemyMaterial.endgameT > 0) {
        int mopUpScore = 0;
        int friendlyIndex = isWhite ? Board::WhiteIndex : Board::BlackIndex;
        int opponentIndex = isWhite ? Board::BlackIndex : Board::WhiteIndex;
        int friendlyKingSquare = board->KingSquare[friendlyIndex];
        int opponentKingSquare = board->KingSquare[opponentIndex];
        mopUpScore += (14 - PrecomputedMoveData::OrthogonalDistance[friendlyKingSquare][opponentKingSquare]) * 4;
        mopUpScore += PrecomputedMoveData::CentreManhattanDistance[opponentKingSquare] * 10;
        return static_cast<int>(mopUpScore * enemyMaterial.endgameT);
    }
    return 0;
}

int Evaluation::CountMaterial(int colourIndex) const {
    int material = 0;
    material += board->Pawns[colourIndex].Count() * PawnValue;
    material += board->Knights[colourIndex].Count() * KnightValue;
    material += board->Bishops[colourIndex].Count() * BishopValue;
    material += board->Rooks[colourIndex].Count() * RookValue;
    material += board->Queens[colourIndex].Count() * QueenValue;
    return material;
}

int Evaluation::EvaluatePieceSquareTables(bool isWhite, float endgameT) const {
    int value = 0;
    int colourIndex = isWhite ? Board::WhiteIndex : Board::BlackIndex;
    value += EvaluatePieceSquareTable(PieceSquareTable::Rooks, board->Rooks[colourIndex], isWhite);
    value += EvaluatePieceSquareTable(PieceSquareTable::Knights, board->Knights[colourIndex], isWhite);
    value += EvaluatePieceSquareTable(PieceSquareTable::Bishops, board->Bishops[colourIndex], isWhite);
    value += EvaluatePieceSquareTable(PieceSquareTable::Queens, board->Queens[colourIndex], isWhite);

    int pawnEarly = EvaluatePieceSquareTable(PieceSquareTable::Pawns, board->Pawns[colourIndex], isWhite);
    int pawnLate = EvaluatePieceSquareTable(PieceSquareTable::PawnsEnd, board->Pawns[colourIndex], isWhite);
    value += static_cast<int>(pawnEarly * (1 - endgameT));
    value += static_cast<int>(pawnLate * endgameT);

    int kingEarlyPhase = PieceSquareTable::Read(PieceSquareTable::KingStart, board->KingSquare[colourIndex], isWhite);
    value += static_cast<int>(kingEarlyPhase * (1 - endgameT));
    int kingLatePhase = PieceSquareTable::Read(PieceSquareTable::KingEnd, board->KingSquare[colourIndex], isWhite);
    value += static_cast<int>(kingLatePhase * endgameT);
    return value;
}

int Evaluation::EvaluatePieceSquareTable(const std::array<int,64>& table, const PieceList& pieceList, bool isWhite) {
    int value = 0;
    for (int i = 0; i < pieceList.Count(); i++) value += PieceSquareTable::Read(table, pieceList[i], isWhite);
    return value;
}

Evaluation::MaterialInfo Evaluation::GetMaterialInfo(int colourIndex) const {
    int numPawns = board->Pawns[colourIndex].Count();
    int numKnights = board->Knights[colourIndex].Count();
    int numBishops = board->Bishops[colourIndex].Count();
    int numRooks = board->Rooks[colourIndex].Count();
    int numQueens = board->Queens[colourIndex].Count();
    bool isWhite = colourIndex == Board::WhiteIndex;
    std::uint64_t myPawns = board->PieceBitboards[Piece::MakePiece(Piece::Pawn, isWhite)];
    std::uint64_t enemyPawns = board->PieceBitboards[Piece::MakePiece(Piece::Pawn, !isWhite)];
    return MaterialInfo(numPawns, numKnights, numBishops, numQueens, numRooks, myPawns, enemyPawns);
}
} // namespace Chess::Core
