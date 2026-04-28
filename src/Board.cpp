#include "ChessEngine.hpp"

#include <algorithm>

namespace Chess::Core {
Board::Board() = default;

int Board::MoveColour() const { return IsWhiteToMove ? Piece::White : Piece::Black; }
int Board::OpponentColour() const { return IsWhiteToMove ? Piece::Black : Piece::White; }
int Board::MoveColourIndex() const { return IsWhiteToMove ? WhiteIndex : BlackIndex; }
int Board::OpponentColourIndex() const { return IsWhiteToMove ? BlackIndex : WhiteIndex; }
int Board::FiftyMoveCounter() const { return CurrentGameState.fiftyMoveCounter; }
std::uint64_t Board::ZobristKey() const { return CurrentGameState.zobristKey; }
std::string Board::CurrentFEN() const { return FenUtility::CurrentFen(*this); }
std::string Board::GameStartFEN() const { return StartPositionInfo.fen; }

void Board::MakeMove(const Move& move, bool inSearch) {
    int startSquare = move.StartSquare();
    int targetSquare = move.TargetSquare();
    int moveFlag = move.MoveFlag();
    bool isPromotion = move.IsPromotion();
    bool isEnPassant = moveFlag == Move::EnPassantCaptureFlag;

    int movedPiece = Square[startSquare];
    int movedPieceType = Piece::PieceType(movedPiece);
    int capturedPiece = isEnPassant ? Piece::MakePiece(Piece::Pawn, OpponentColour()) : Square[targetSquare];
    int capturedPieceType = Piece::PieceType(capturedPiece);

    int prevCastleState = CurrentGameState.castlingRights;
    int prevEnPassantFile = CurrentGameState.enPassantFile;
    std::uint64_t newZobristKey = CurrentGameState.zobristKey;
    int newCastlingRights = CurrentGameState.castlingRights;
    int newEnPassantFile = 0;

    MovePiece(movedPiece, startSquare, targetSquare);

    if (capturedPieceType != Piece::None) {
        int captureSquare = targetSquare;
        if (isEnPassant) {
            captureSquare = targetSquare + (IsWhiteToMove ? -8 : 8);
            Square[captureSquare] = Piece::None;
        }
        if (capturedPieceType != Piece::Pawn) {
            TotalPieceCountWithoutPawnsAndKings--;
        }
        allPieceLists[capturedPiece]->RemovePieceAtSquare(captureSquare);
        BitBoardUtility::ToggleSquare(PieceBitboards[capturedPiece], captureSquare);
        BitBoardUtility::ToggleSquare(ColourBitboards[OpponentColourIndex()], captureSquare);
        newZobristKey ^= Zobrist::piecesArray[capturedPiece][captureSquare];
    }

    if (movedPieceType == Piece::King) {
        KingSquare[MoveColourIndex()] = targetSquare;
        newCastlingRights &= IsWhiteToMove ? 0b1100 : 0b0011;

        if (moveFlag == Move::CastleFlag) {
            int rookPiece = Piece::MakePiece(Piece::Rook, MoveColour());
            bool kingside = targetSquare == BoardHelper::g1 || targetSquare == BoardHelper::g8;
            int castlingRookFromIndex = kingside ? targetSquare + 1 : targetSquare - 2;
            int castlingRookToIndex = kingside ? targetSquare - 1 : targetSquare + 1;

            BitBoardUtility::ToggleSquares(PieceBitboards[rookPiece], castlingRookFromIndex, castlingRookToIndex);
            BitBoardUtility::ToggleSquares(ColourBitboards[MoveColourIndex()], castlingRookFromIndex, castlingRookToIndex);
            allPieceLists[rookPiece]->MovePiece(castlingRookFromIndex, castlingRookToIndex);
            Square[castlingRookFromIndex] = Piece::None;
            Square[castlingRookToIndex] = Piece::Rook | MoveColour();

            newZobristKey ^= Zobrist::piecesArray[rookPiece][castlingRookFromIndex];
            newZobristKey ^= Zobrist::piecesArray[rookPiece][castlingRookToIndex];
        }
    }

    if (isPromotion) {
        TotalPieceCountWithoutPawnsAndKings++;
        int promotionPieceType = 0;
        switch (moveFlag) {
            case Move::PromoteToQueenFlag: promotionPieceType = Piece::Queen; break;
            case Move::PromoteToRookFlag: promotionPieceType = Piece::Rook; break;
            case Move::PromoteToKnightFlag: promotionPieceType = Piece::Knight; break;
            case Move::PromoteToBishopFlag: promotionPieceType = Piece::Bishop; break;
            default: break;
        }
        int promotionPiece = Piece::MakePiece(promotionPieceType, MoveColour());
        BitBoardUtility::ToggleSquare(PieceBitboards[movedPiece], targetSquare);
        BitBoardUtility::ToggleSquare(PieceBitboards[promotionPiece], targetSquare);
        allPieceLists[movedPiece]->RemovePieceAtSquare(targetSquare);
        allPieceLists[promotionPiece]->AddPieceAtSquare(targetSquare);
        Square[targetSquare] = promotionPiece;
    }

    if (moveFlag == Move::PawnTwoUpFlag) {
        // Arena-compat mode: en passant is intentionally disabled.
        // Do not create an en-passant file after a double pawn push.
    }

    if (prevCastleState != 0) {
        if (targetSquare == BoardHelper::h1 || startSquare == BoardHelper::h1) newCastlingRights &= GameState::ClearWhiteKingsideMask;
        else if (targetSquare == BoardHelper::a1 || startSquare == BoardHelper::a1) newCastlingRights &= GameState::ClearWhiteQueensideMask;
        if (targetSquare == BoardHelper::h8 || startSquare == BoardHelper::h8) newCastlingRights &= GameState::ClearBlackKingsideMask;
        else if (targetSquare == BoardHelper::a8 || startSquare == BoardHelper::a8) newCastlingRights &= GameState::ClearBlackQueensideMask;
    }

    newZobristKey ^= Zobrist::sideToMove;
    newZobristKey ^= Zobrist::piecesArray[movedPiece][startSquare];
    newZobristKey ^= Zobrist::piecesArray[Square[targetSquare]][targetSquare];
    newZobristKey ^= Zobrist::enPassantFile[prevEnPassantFile];

    if (newCastlingRights != prevCastleState) {
        newZobristKey ^= Zobrist::castlingRights[prevCastleState];
        newZobristKey ^= Zobrist::castlingRights[newCastlingRights];
    }

    IsWhiteToMove = !IsWhiteToMove;
    PlyCount++;
    int newFiftyMoveCounter = CurrentGameState.fiftyMoveCounter + 1;

    AllPiecesBitboard = ColourBitboards[WhiteIndex] | ColourBitboards[BlackIndex];
    UpdateSliderBitboards();

    if (movedPieceType == Piece::Pawn || capturedPieceType != Piece::None) {
        if (!inSearch) {
            RepetitionPositionHistory.clear();
        }
        newFiftyMoveCounter = 0;
    }

    GameState newState(capturedPieceType, newEnPassantFile, newCastlingRights, newFiftyMoveCounter, newZobristKey);
    gameStateHistory.push_back(newState);
    CurrentGameState = newState;
    hasCachedInCheckValue = false;

    if (!inSearch) {
        RepetitionPositionHistory.push_back(newState.zobristKey);
        AllGameMoves.push_back(move);
    }
}

void Board::UnmakeMove(const Move& move, bool inSearch) {
    IsWhiteToMove = !IsWhiteToMove;
    bool undoingWhiteMove = IsWhiteToMove;

    int movedFrom = move.StartSquare();
    int movedTo = move.TargetSquare();
    int moveFlag = move.MoveFlag();

    bool undoingEnPassant = moveFlag == Move::EnPassantCaptureFlag;
    bool undoingPromotion = move.IsPromotion();
    bool undoingCapture = CurrentGameState.capturedPieceType != Piece::None;

    int movedPiece = undoingPromotion ? Piece::MakePiece(Piece::Pawn, MoveColour()) : Square[movedTo];
    int movedPieceType = Piece::PieceType(movedPiece);
    int capturedPieceType = CurrentGameState.capturedPieceType;

    if (undoingPromotion) {
        int promotedPiece = Square[movedTo];
        int pawnPiece = Piece::MakePiece(Piece::Pawn, MoveColour());
        TotalPieceCountWithoutPawnsAndKings--;

        allPieceLists[promotedPiece]->RemovePieceAtSquare(movedTo);
        allPieceLists[movedPiece]->AddPieceAtSquare(movedTo);
        BitBoardUtility::ToggleSquare(PieceBitboards[promotedPiece], movedTo);
        BitBoardUtility::ToggleSquare(PieceBitboards[pawnPiece], movedTo);
    }

    MovePiece(movedPiece, movedTo, movedFrom);

    if (undoingCapture) {
        int captureSquare = movedTo;
        int capturedPiece = Piece::MakePiece(capturedPieceType, OpponentColour());
        if (undoingEnPassant) {
            captureSquare = movedTo + (undoingWhiteMove ? -8 : 8);
        }
        if (capturedPieceType != Piece::Pawn) {
            TotalPieceCountWithoutPawnsAndKings++;
        }
        BitBoardUtility::ToggleSquare(PieceBitboards[capturedPiece], captureSquare);
        BitBoardUtility::ToggleSquare(ColourBitboards[OpponentColourIndex()], captureSquare);
        allPieceLists[capturedPiece]->AddPieceAtSquare(captureSquare);
        Square[captureSquare] = capturedPiece;
    }

    if (movedPieceType == Piece::King) {
        KingSquare[MoveColourIndex()] = movedFrom;
        if (moveFlag == Move::CastleFlag) {
            int rookPiece = Piece::MakePiece(Piece::Rook, MoveColour());
            bool kingside = movedTo == BoardHelper::g1 || movedTo == BoardHelper::g8;
            int rookSquareBeforeCastling = kingside ? movedTo + 1 : movedTo - 2;
            int rookSquareAfterCastling = kingside ? movedTo - 1 : movedTo + 1;
            BitBoardUtility::ToggleSquares(PieceBitboards[rookPiece], rookSquareAfterCastling, rookSquareBeforeCastling);
            BitBoardUtility::ToggleSquares(ColourBitboards[MoveColourIndex()], rookSquareAfterCastling, rookSquareBeforeCastling);
            Square[rookSquareAfterCastling] = Piece::None;
            Square[rookSquareBeforeCastling] = rookPiece;
            allPieceLists[rookPiece]->MovePiece(rookSquareAfterCastling, rookSquareBeforeCastling);
        }
    }

    AllPiecesBitboard = ColourBitboards[WhiteIndex] | ColourBitboards[BlackIndex];
    UpdateSliderBitboards();

    if (!inSearch && !RepetitionPositionHistory.empty()) {
        RepetitionPositionHistory.pop_back();
    }
    if (!inSearch && !AllGameMoves.empty()) {
        AllGameMoves.pop_back();
    }

    if (!gameStateHistory.empty()) gameStateHistory.pop_back();
    CurrentGameState = gameStateHistory.back();
    PlyCount--;
    hasCachedInCheckValue = false;
}

void Board::MakeNullMove() {
    IsWhiteToMove = !IsWhiteToMove;
    PlyCount++;

    std::uint64_t newZobristKey = CurrentGameState.zobristKey;
    newZobristKey ^= Zobrist::sideToMove;
    newZobristKey ^= Zobrist::enPassantFile[CurrentGameState.enPassantFile];

    GameState newState(Piece::None, 0, CurrentGameState.castlingRights, CurrentGameState.fiftyMoveCounter + 1, newZobristKey);
    CurrentGameState = newState;
    gameStateHistory.push_back(CurrentGameState);
    UpdateSliderBitboards();
    hasCachedInCheckValue = true;
    cachedInCheckValue = false;
}

void Board::UnmakeNullMove() {
    IsWhiteToMove = !IsWhiteToMove;
    PlyCount--;
    if (!gameStateHistory.empty()) gameStateHistory.pop_back();
    CurrentGameState = gameStateHistory.back();
    UpdateSliderBitboards();
    hasCachedInCheckValue = true;
    cachedInCheckValue = false;
}

bool Board::IsInCheck() {
    if (hasCachedInCheckValue) return cachedInCheckValue;
    cachedInCheckValue = CalculateInCheckState();
    hasCachedInCheckValue = true;
    return cachedInCheckValue;
}

bool Board::CalculateInCheckState() {
    int kingSquare = KingSquare[MoveColourIndex()];
    std::uint64_t blockers = AllPiecesBitboard;

    if (EnemyOrthogonalSliders != 0) {
        std::uint64_t rookAttacks = Magic::GetRookAttacks(kingSquare, blockers);
        if ((rookAttacks & EnemyOrthogonalSliders) != 0) return true;
    }
    if (EnemyDiagonalSliders != 0) {
        std::uint64_t bishopAttacks = Magic::GetBishopAttacks(kingSquare, blockers);
        if ((bishopAttacks & EnemyDiagonalSliders) != 0) return true;
    }

    std::uint64_t enemyKnights = PieceBitboards[Piece::MakePiece(Piece::Knight, OpponentColour())];
    if ((BitBoardUtility::KnightAttacks[kingSquare] & enemyKnights) != 0) return true;

    std::uint64_t enemyPawns = PieceBitboards[Piece::MakePiece(Piece::Pawn, OpponentColour())];
    std::uint64_t pawnAttackMask = IsWhiteToMove ? BitBoardUtility::WhitePawnAttacks[kingSquare] : BitBoardUtility::BlackPawnAttacks[kingSquare];
    if ((pawnAttackMask & enemyPawns) != 0) return true;

    return false;
}

void Board::LoadStartPosition() { LoadPosition(FenUtility::StartPositionFEN); }

void Board::LoadPosition(const std::string& fen) {
    PositionInfo posInfo = FenUtility::PositionFromFen(fen);
    LoadPosition(posInfo);
}

void Board::LoadPosition(const PositionInfo& posInfo) {
    StartPositionInfo = posInfo;
    Initialize();

    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        int piece = posInfo.squares[squareIndex];
        int pieceType = Piece::PieceType(piece);
        int colourIndex = Piece::IsWhite(piece) ? WhiteIndex : BlackIndex;
        Square[squareIndex] = piece;

        if (piece != Piece::None) {
            BitBoardUtility::SetSquare(PieceBitboards[piece], squareIndex);
            BitBoardUtility::SetSquare(ColourBitboards[colourIndex], squareIndex);
            if (pieceType == Piece::King) {
                KingSquare[colourIndex] = squareIndex;
            } else {
                allPieceLists[piece]->AddPieceAtSquare(squareIndex);
            }
            TotalPieceCountWithoutPawnsAndKings += (pieceType == Piece::Pawn || pieceType == Piece::King) ? 0 : 1;
        }
    }

    IsWhiteToMove = posInfo.whiteToMove;
    AllPiecesBitboard = ColourBitboards[WhiteIndex] | ColourBitboards[BlackIndex];
    UpdateSliderBitboards();

    int whiteCastle = (posInfo.whiteCastleKingside ? 1 << 0 : 0) | (posInfo.whiteCastleQueenside ? 1 << 1 : 0);
    int blackCastle = (posInfo.blackCastleKingside ? 1 << 2 : 0) | (posInfo.blackCastleQueenside ? 1 << 3 : 0);
    int castlingRights = whiteCastle | blackCastle;

    PlyCount = (posInfo.moveCount - 1) * 2 + (IsWhiteToMove ? 0 : 1);
    CurrentGameState = GameState(Piece::None, posInfo.epFile, castlingRights, posInfo.fiftyMovePlyCount, 0);
    std::uint64_t zobristKey = Zobrist::CalculateZobristKey(*this);
    CurrentGameState = GameState(Piece::None, posInfo.epFile, castlingRights, posInfo.fiftyMovePlyCount, zobristKey);

    RepetitionPositionHistory.push_back(zobristKey);
    gameStateHistory.push_back(CurrentGameState);
}

std::string Board::ToString() const { return BoardHelper::CreateDiagram(*this, IsWhiteToMove); }

Board Board::CreateBoard(const std::string& fen) {
    Board board;
    board.LoadPosition(fen.empty() ? FenUtility::StartPositionFEN : fen);
    return board;
}

Board Board::CreateBoard(const Board& source) {
    Board board;
    board.LoadPosition(source.StartPositionInfo);
    for (const Move& m : source.AllGameMoves) board.MakeMove(m);
    return board;
}

void Board::MovePiece(int piece, int startSquare, int targetSquare) {
    BitBoardUtility::ToggleSquares(PieceBitboards[piece], startSquare, targetSquare);
    BitBoardUtility::ToggleSquares(ColourBitboards[MoveColourIndex()], startSquare, targetSquare);
    allPieceLists[piece]->MovePiece(startSquare, targetSquare);
    Square[startSquare] = Piece::None;
    Square[targetSquare] = piece;
}

void Board::UpdateSliderBitboards() {
    int friendlyRook = Piece::MakePiece(Piece::Rook, MoveColour());
    int friendlyQueen = Piece::MakePiece(Piece::Queen, MoveColour());
    int friendlyBishop = Piece::MakePiece(Piece::Bishop, MoveColour());
    FriendlyOrthogonalSliders = PieceBitboards[friendlyRook] | PieceBitboards[friendlyQueen];
    FriendlyDiagonalSliders = PieceBitboards[friendlyBishop] | PieceBitboards[friendlyQueen];

    int enemyRook = Piece::MakePiece(Piece::Rook, OpponentColour());
    int enemyQueen = Piece::MakePiece(Piece::Queen, OpponentColour());
    int enemyBishop = Piece::MakePiece(Piece::Bishop, OpponentColour());
    EnemyOrthogonalSliders = PieceBitboards[enemyRook] | PieceBitboards[enemyQueen];
    EnemyDiagonalSliders = PieceBitboards[enemyBishop] | PieceBitboards[enemyQueen];
}

void Board::Initialize() {
    AllGameMoves.clear();
    KingSquare = {0, 0};
    Square.fill(0);

    RepetitionPositionHistory.clear();
    gameStateHistory.clear();

    CurrentGameState = GameState();
    PlyCount = 0;

    Knights = {PieceList(10), PieceList(10)};
    Pawns = {PieceList(8), PieceList(8)};
    Rooks = {PieceList(10), PieceList(10)};
    Bishops = {PieceList(10), PieceList(10)};
    Queens = {PieceList(9), PieceList(9)};

    allPieceLists.fill(nullptr);
    allPieceLists[Piece::WhitePawn] = &Pawns[WhiteIndex];
    allPieceLists[Piece::WhiteKnight] = &Knights[WhiteIndex];
    allPieceLists[Piece::WhiteBishop] = &Bishops[WhiteIndex];
    allPieceLists[Piece::WhiteRook] = &Rooks[WhiteIndex];
    allPieceLists[Piece::WhiteQueen] = &Queens[WhiteIndex];
    static PieceList whiteKingList(1), blackKingList(1);
    whiteKingList = PieceList(1);
    blackKingList = PieceList(1);
    allPieceLists[Piece::WhiteKing] = &whiteKingList;

    allPieceLists[Piece::BlackPawn] = &Pawns[BlackIndex];
    allPieceLists[Piece::BlackKnight] = &Knights[BlackIndex];
    allPieceLists[Piece::BlackBishop] = &Bishops[BlackIndex];
    allPieceLists[Piece::BlackRook] = &Rooks[BlackIndex];
    allPieceLists[Piece::BlackQueen] = &Queens[BlackIndex];
    allPieceLists[Piece::BlackKing] = &blackKingList;

    TotalPieceCountWithoutPawnsAndKings = 0;
    PieceBitboards.fill(0);
    ColourBitboards.fill(0);
    AllPiecesBitboard = 0;
    FriendlyOrthogonalSliders = FriendlyDiagonalSliders = EnemyOrthogonalSliders = EnemyDiagonalSliders = 0;
    cachedInCheckValue = false;
    hasCachedInCheckValue = false;
}
} // namespace Chess::Core
