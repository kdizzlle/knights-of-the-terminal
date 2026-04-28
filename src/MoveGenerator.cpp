#include "ChessEngine.hpp"

#include <limits>

namespace Chess::Core {
std::span<Move> MoveGenerator::GenerateMoves(Board& board, bool capturesOnly) {
    static thread_local std::array<Move, MaxMoves> storage{};
    std::span<Move> moves(storage);
    GenerateMoves(board, moves, capturesOnly);
    return moves;
}

int MoveGenerator::GenerateMoves(Board& board_, std::span<Move>& moves, bool capturesOnly) {
    board = &board_;
    generateQuietMoves = !capturesOnly;
    Init();
    GenerateKingMoves(moves);
    if (!inDoubleCheck) {
        GenerateSlidingMoves(moves);
        GenerateKnightMoves(moves);
        GeneratePawnMoves(moves);
    }
    moves = moves.first(static_cast<std::size_t>(currMoveIndex));
    return static_cast<int>(moves.size());
}

bool MoveGenerator::InCheck() const { return inCheck; }

void MoveGenerator::Init() {
    currMoveIndex = 0;
    inCheck = false;
    inDoubleCheck = false;
    checkRayBitmask = 0;
    pinRays = 0;

    isWhiteToMove = board->MoveColour() == Piece::White;
    friendlyColour = board->MoveColour();
    opponentColour = board->OpponentColour();
    friendlyKingSquare = board->KingSquare[board->MoveColourIndex()];
    friendlyIndex = board->MoveColourIndex();
    enemyIndex = 1 - friendlyIndex;

    enemyPieces = board->ColourBitboards[enemyIndex];
    friendlyPieces = board->ColourBitboards[friendlyIndex];
    allPieces = board->AllPiecesBitboard;
    emptySquares = ~allPieces;
    emptyOrEnemySquares = emptySquares | enemyPieces;
    moveTypeMask = generateQuietMoves ? std::numeric_limits<std::uint64_t>::max() : enemyPieces;

    CalculateAttackData();
}

void MoveGenerator::GenerateKingMoves(std::span<Move> moves) {
    std::uint64_t legalMask = ~(opponentAttackMap | friendlyPieces);
    std::uint64_t kingMovesBoard = BitBoardUtility::KingMoves[friendlyKingSquare] & legalMask & moveTypeMask;
    while (kingMovesBoard != 0) {
        int targetSquare = BitBoardUtility::PopLSB(kingMovesBoard);
        moves[static_cast<std::size_t>(currMoveIndex++)] = Move(friendlyKingSquare, targetSquare);
    }

    if (!inCheck && generateQuietMoves) {
        std::uint64_t castleBlockers = opponentAttackMap | board->AllPiecesBitboard;
        if (board->CurrentGameState.HasKingsideCastleRight(board->IsWhiteToMove)) {
            std::uint64_t castleMask = board->IsWhiteToMove ? Bits::WhiteKingsideMask : Bits::BlackKingsideMask;
            if ((castleMask & castleBlockers) == 0) {
                int targetSquare = board->IsWhiteToMove ? BoardHelper::g1 : BoardHelper::g8;
                moves[static_cast<std::size_t>(currMoveIndex++)] = Move(friendlyKingSquare, targetSquare, Move::CastleFlag);
            }
        }
        if (board->CurrentGameState.HasQueensideCastleRight(board->IsWhiteToMove)) {
            std::uint64_t castleMask = board->IsWhiteToMove ? Bits::WhiteQueensideMask2 : Bits::BlackQueensideMask2;
            std::uint64_t castleBlockMask = board->IsWhiteToMove ? Bits::WhiteQueensideMask : Bits::BlackQueensideMask;
            if ((castleMask & castleBlockers) == 0 && (castleBlockMask & board->AllPiecesBitboard) == 0) {
                int targetSquare = board->IsWhiteToMove ? BoardHelper::c1 : BoardHelper::c8;
                moves[static_cast<std::size_t>(currMoveIndex++)] = Move(friendlyKingSquare, targetSquare, Move::CastleFlag);
            }
        }
    }
}

void MoveGenerator::GenerateSlidingMoves(std::span<Move> moves) {
    std::uint64_t moveMask = emptyOrEnemySquares & checkRayBitmask & moveTypeMask;
    std::uint64_t orthogonalSliders = board->FriendlyOrthogonalSliders;
    std::uint64_t diagonalSliders = board->FriendlyDiagonalSliders;
    if (inCheck) {
        orthogonalSliders &= ~pinRays;
        diagonalSliders &= ~pinRays;
    }

    while (orthogonalSliders != 0) {
        int startSquare = BitBoardUtility::PopLSB(orthogonalSliders);
        std::uint64_t moveSquares = Magic::GetRookAttacks(startSquare, allPieces) & moveMask;
        if (IsPinned(startSquare)) moveSquares &= PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare];
        while (moveSquares != 0) {
            int targetSquare = BitBoardUtility::PopLSB(moveSquares);
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare);
        }
    }

    while (diagonalSliders != 0) {
        int startSquare = BitBoardUtility::PopLSB(diagonalSliders);
        std::uint64_t moveSquares = Magic::GetBishopAttacks(startSquare, allPieces) & moveMask;
        if (IsPinned(startSquare)) moveSquares &= PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare];
        while (moveSquares != 0) {
            int targetSquare = BitBoardUtility::PopLSB(moveSquares);
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare);
        }
    }
}

void MoveGenerator::GenerateKnightMoves(std::span<Move> moves) {
    int friendlyKnightPiece = Piece::MakePiece(Piece::Knight, board->MoveColour());
    std::uint64_t knights = board->PieceBitboards[friendlyKnightPiece] & notPinRays;
    std::uint64_t moveMask = emptyOrEnemySquares & checkRayBitmask & moveTypeMask;
    while (knights != 0) {
        int knightSquare = BitBoardUtility::PopLSB(knights);
        std::uint64_t moveSquares = BitBoardUtility::KnightAttacks[knightSquare] & moveMask;
        while (moveSquares != 0) {
            int targetSquare = BitBoardUtility::PopLSB(moveSquares);
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(knightSquare, targetSquare);
        }
    }
}

void MoveGenerator::GeneratePawnMoves(std::span<Move> moves) {
    int pushDir = board->IsWhiteToMove ? 1 : -1;
    int pushOffset = pushDir * 8;
    int friendlyPawnPiece = Piece::MakePiece(Piece::Pawn, board->MoveColour());
    std::uint64_t pawns = board->PieceBitboards[friendlyPawnPiece];
    std::uint64_t promotionRankMask = board->IsWhiteToMove ? BitBoardUtility::Rank8 : BitBoardUtility::Rank1;
    std::uint64_t singlePush = BitBoardUtility::Shift(pawns, pushOffset) & emptySquares;
    std::uint64_t pushPromotions = singlePush & promotionRankMask & checkRayBitmask;

    std::uint64_t captureEdgeFileMask = board->IsWhiteToMove ? BitBoardUtility::notAFile : BitBoardUtility::notHFile;
    std::uint64_t captureEdgeFileMask2 = board->IsWhiteToMove ? BitBoardUtility::notHFile : BitBoardUtility::notAFile;
    std::uint64_t captureA = BitBoardUtility::Shift(pawns & captureEdgeFileMask, pushDir * 7) & enemyPieces;
    std::uint64_t captureB = BitBoardUtility::Shift(pawns & captureEdgeFileMask2, pushDir * 9) & enemyPieces;

    std::uint64_t singlePushNoPromotions = singlePush & ~promotionRankMask & checkRayBitmask;
    std::uint64_t capturePromotionsA = captureA & promotionRankMask & checkRayBitmask;
    std::uint64_t capturePromotionsB = captureB & promotionRankMask & checkRayBitmask;
    captureA &= checkRayBitmask & ~promotionRankMask;
    captureB &= checkRayBitmask & ~promotionRankMask;

    if (generateQuietMoves) {
        while (singlePushNoPromotions != 0) {
            int targetSquare = BitBoardUtility::PopLSB(singlePushNoPromotions);
            int startSquare = targetSquare - pushOffset;
            if (!IsPinned(startSquare) || PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare] == PrecomputedMoveData::alignMask[targetSquare][friendlyKingSquare]) {
                moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare);
            }
        }
        std::uint64_t doublePushTargetRankMask = board->IsWhiteToMove ? BitBoardUtility::Rank4 : BitBoardUtility::Rank5;
        std::uint64_t doublePush = BitBoardUtility::Shift(singlePush, pushOffset) & emptySquares & doublePushTargetRankMask & checkRayBitmask;
        while (doublePush != 0) {
            int targetSquare = BitBoardUtility::PopLSB(doublePush);
            int startSquare = targetSquare - pushOffset * 2;
            if (!IsPinned(startSquare) || PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare] == PrecomputedMoveData::alignMask[targetSquare][friendlyKingSquare]) {
                moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare, Move::PawnTwoUpFlag);
            }
        }
    }

    while (captureA != 0) {
        int targetSquare = BitBoardUtility::PopLSB(captureA);
        int startSquare = targetSquare - pushDir * 7;
        if (!IsPinned(startSquare) || PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare] == PrecomputedMoveData::alignMask[targetSquare][friendlyKingSquare]) {
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare);
        }
    }
    while (captureB != 0) {
        int targetSquare = BitBoardUtility::PopLSB(captureB);
        int startSquare = targetSquare - pushDir * 9;
        if (!IsPinned(startSquare) || PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare] == PrecomputedMoveData::alignMask[targetSquare][friendlyKingSquare]) {
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare);
        }
    }

    while (pushPromotions != 0) {
        int targetSquare = BitBoardUtility::PopLSB(pushPromotions);
        int startSquare = targetSquare - pushOffset;
        if (!IsPinned(startSquare)) GeneratePromotions(startSquare, targetSquare, moves);
    }
    while (capturePromotionsA != 0) {
        int targetSquare = BitBoardUtility::PopLSB(capturePromotionsA);
        int startSquare = targetSquare - pushDir * 7;
        if (!IsPinned(startSquare) || PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare] == PrecomputedMoveData::alignMask[targetSquare][friendlyKingSquare]) GeneratePromotions(startSquare, targetSquare, moves);
    }
    while (capturePromotionsB != 0) {
        int targetSquare = BitBoardUtility::PopLSB(capturePromotionsB);
        int startSquare = targetSquare - pushDir * 9;
        if (!IsPinned(startSquare) || PrecomputedMoveData::alignMask[startSquare][friendlyKingSquare] == PrecomputedMoveData::alignMask[targetSquare][friendlyKingSquare]) GeneratePromotions(startSquare, targetSquare, moves);
    }

    // Arena-compat mode: en passant move generation is disabled.
}

void MoveGenerator::GeneratePromotions(int startSquare, int targetSquare, std::span<Move> moves) {
    moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare, Move::PromoteToQueenFlag);
    if (generateQuietMoves) {
        if (promotionsToGenerate == PromotionMode::All) {
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare, Move::PromoteToKnightFlag);
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare, Move::PromoteToRookFlag);
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare, Move::PromoteToBishopFlag);
        } else if (promotionsToGenerate == PromotionMode::QueenAndKnight) {
            moves[static_cast<std::size_t>(currMoveIndex++)] = Move(startSquare, targetSquare, Move::PromoteToKnightFlag);
        }
    }
}

bool MoveGenerator::IsPinned(int square) const { return ((pinRays >> square) & 1ULL) != 0; }

void MoveGenerator::GenSlidingAttackMap() {
    opponentSlidingAttackMap = 0;
    auto updateSlideAttack = [&](std::uint64_t pieceBoard, bool ortho) {
        std::uint64_t blockers = board->AllPiecesBitboard & ~(1ULL << friendlyKingSquare);
        while (pieceBoard != 0) {
            int startSquare = BitBoardUtility::PopLSB(pieceBoard);
            std::uint64_t moveBoard = Magic::GetSliderAttacks(startSquare, blockers, ortho);
            opponentSlidingAttackMap |= moveBoard;
        }
    };
    updateSlideAttack(board->EnemyOrthogonalSliders, true);
    updateSlideAttack(board->EnemyDiagonalSliders, false);
}

void MoveGenerator::CalculateAttackData() {
    GenSlidingAttackMap();
    int startDirIndex = 0;
    int endDirIndex = 8;
    if (board->Queens[enemyIndex].Count() == 0) {
        startDirIndex = board->Rooks[enemyIndex].Count() > 0 ? 0 : 4;
        endDirIndex = board->Bishops[enemyIndex].Count() > 0 ? 8 : 4;
    }

    for (int dir = startDirIndex; dir < endDirIndex; dir++) {
        bool isDiagonal = dir > 3;
        std::uint64_t slider = isDiagonal ? board->EnemyDiagonalSliders : board->EnemyOrthogonalSliders;
        if ((PrecomputedMoveData::dirRayMask[dir][friendlyKingSquare] & slider) == 0) continue;

        int n = PrecomputedMoveData::numSquaresToEdge[friendlyKingSquare][dir];
        int directionOffset = PrecomputedMoveData::directionOffsets[dir];
        bool isFriendlyPieceAlongRay = false;
        std::uint64_t rayMask = 0;

        for (int i = 0; i < n; i++) {
            int squareIndex = friendlyKingSquare + directionOffset * (i + 1);
            rayMask |= 1ULL << squareIndex;
            int piece = board->Square[squareIndex];
            if (piece != Piece::None) {
                if (Piece::IsColour(piece, friendlyColour)) {
                    if (!isFriendlyPieceAlongRay) isFriendlyPieceAlongRay = true;
                    else break;
                } else {
                    int pieceType = Piece::PieceType(piece);
                    if ((isDiagonal && Piece::IsDiagonalSlider(pieceType)) || (!isDiagonal && Piece::IsOrthogonalSlider(pieceType))) {
                        if (isFriendlyPieceAlongRay) pinRays |= rayMask;
                        else {
                            checkRayBitmask |= rayMask;
                            inDoubleCheck = inCheck;
                            inCheck = true;
                        }
                        break;
                    } else break;
                }
            }
        }
        if (inDoubleCheck) break;
    }

    notPinRays = ~pinRays;
    std::uint64_t opponentKnightAttacks = 0;
    std::uint64_t knights = board->PieceBitboards[Piece::MakePiece(Piece::Knight, board->OpponentColour())];
    std::uint64_t friendlyKingBoard = board->PieceBitboards[Piece::MakePiece(Piece::King, board->MoveColour())];
    while (knights != 0) {
        int knightSquare = BitBoardUtility::PopLSB(knights);
        std::uint64_t knightAttacks = BitBoardUtility::KnightAttacks[knightSquare];
        opponentKnightAttacks |= knightAttacks;
        if ((knightAttacks & friendlyKingBoard) != 0) {
            inDoubleCheck = inCheck;
            inCheck = true;
            checkRayBitmask |= 1ULL << knightSquare;
        }
    }

    std::uint64_t opponentPawnsBoard = board->PieceBitboards[Piece::MakePiece(Piece::Pawn, board->OpponentColour())];
    opponentPawnAttackMap = BitBoardUtility::PawnAttacks(opponentPawnsBoard, !isWhiteToMove);
    if (BitBoardUtility::ContainsSquare(opponentPawnAttackMap, friendlyKingSquare)) {
        inDoubleCheck = inCheck;
        inCheck = true;
        std::uint64_t possiblePawnAttackOrigins = board->IsWhiteToMove ? BitBoardUtility::WhitePawnAttacks[friendlyKingSquare] : BitBoardUtility::BlackPawnAttacks[friendlyKingSquare];
        std::uint64_t pawnCheckMap = opponentPawnsBoard & possiblePawnAttackOrigins;
        checkRayBitmask |= pawnCheckMap;
    }

    int enemyKingSquare = board->KingSquare[enemyIndex];
    opponentAttackMapNoPawns = opponentSlidingAttackMap | opponentKnightAttacks | BitBoardUtility::KingMoves[enemyKingSquare];
    opponentAttackMap = opponentAttackMapNoPawns | opponentPawnAttackMap;
    if (!inCheck) checkRayBitmask = std::numeric_limits<std::uint64_t>::max();
}

bool MoveGenerator::InCheckAfterEnPassant(int startSquare, int targetSquare, int epCaptureSquare) {
    std::uint64_t enemyOrtho = board->EnemyOrthogonalSliders;
    if (enemyOrtho != 0) {
        std::uint64_t maskedBlockers = (allPieces ^ ((1ULL << epCaptureSquare) | (1ULL << startSquare) | (1ULL << targetSquare)));
        std::uint64_t rookAttacks = Magic::GetRookAttacks(friendlyKingSquare, maskedBlockers);
        return (rookAttacks & enemyOrtho) != 0;
    }
    return false;
}

MoveOrdering::MoveOrdering(MoveGenerator&, TranspositionTable& tt) : transpositionTable(&tt) {}

} // namespace Chess::Core
