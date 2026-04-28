#include "ChessEngine.hpp"

#include <bit>
#include <cctype>
#include <sstream>

using std::uint64_t;

namespace Chess::Core {
Coord::Coord(int f, int r) : fileIndex(f), rankIndex(r) {}
Coord::Coord(int squareIndex) : fileIndex(BoardHelper::FileIndex(squareIndex)), rankIndex(BoardHelper::RankIndex(squareIndex)) {}
bool Coord::IsLightSquare() const { return (fileIndex + rankIndex) % 2 != 0; }
int Coord::CompareTo(const Coord& other) const { return (fileIndex == other.fileIndex && rankIndex == other.rankIndex) ? 0 : 1; }
bool Coord::IsValidSquare() const { return fileIndex >= 0 && fileIndex < 8 && rankIndex >= 0 && rankIndex < 8; }
int Coord::SquareIndex() const { return BoardHelper::IndexFromCoord(*this); }
Coord operator+(const Coord& a, const Coord& b) { return Coord(a.fileIndex + b.fileIndex, a.rankIndex + b.rankIndex); }
Coord operator-(const Coord& a, const Coord& b) { return Coord(a.fileIndex - b.fileIndex, a.rankIndex - b.rankIndex); }
Coord operator*(const Coord& a, int m) { return Coord(a.fileIndex * m, a.rankIndex * m); }
Coord operator*(int m, const Coord& a) { return a * m; }

int Piece::MakePiece(int pieceType, int pieceColour) { return pieceType | pieceColour; }
int Piece::MakePiece(int pieceType, bool pieceIsWhite) { return MakePiece(pieceType, pieceIsWhite ? White : Black); }
bool Piece::IsColour(int piece, int colour) { return (piece & colourMask) == colour && piece != 0; }
bool Piece::IsWhite(int piece) { return IsColour(piece, White); }
int Piece::PieceColour(int piece) { return piece & colourMask; }
int Piece::PieceType(int piece) { return piece & typeMask; }
bool Piece::IsOrthogonalSlider(int piece) { int t = PieceType(piece); return t == Queen || t == Rook; }
bool Piece::IsDiagonalSlider(int piece) { int t = PieceType(piece); return t == Queen || t == Bishop; }
bool Piece::IsSlidingPiece(int piece) { int t = PieceType(piece); return t == Queen || t == Bishop || t == Rook; }
char Piece::GetSymbol(int piece) {
    int pieceType = PieceType(piece);
    char symbol = ' ';
    switch (pieceType) {
        case Rook: symbol='R'; break; case Knight: symbol='N'; break; case Bishop: symbol='B'; break; case Queen: symbol='Q'; break; case King: symbol='K'; break; case Pawn: symbol='P'; break;
        default: break;
    }
    symbol = IsWhite(piece) ? symbol : static_cast<char>(std::tolower(symbol));
    return symbol;
}
int Piece::GetPieceTypeFromSymbol(char symbol) {
    symbol = static_cast<char>(std::toupper(symbol));
    switch (symbol) { case 'R': return Rook; case 'N': return Knight; case 'B': return Bishop; case 'Q': return Queen; case 'K': return King; case 'P': return Pawn; default: return None; }
}

Move::Move(std::uint16_t v) : moveValue(v) {}
Move::Move(int s, int t) : moveValue(static_cast<std::uint16_t>(s | (t << 6))) {}
Move::Move(int s, int t, int f) : moveValue(static_cast<std::uint16_t>(s | (t << 6) | (f << 12))) {}
std::uint16_t Move::Value() const { return moveValue; }
bool Move::IsNull() const { return moveValue == 0; }
int Move::StartSquare() const { return moveValue & startSquareMask; }
int Move::TargetSquare() const { return (moveValue & targetSquareMask) >> 6; }
bool Move::IsPromotion() const { return MoveFlag() >= PromoteToQueenFlag; }
int Move::MoveFlag() const { return moveValue >> 12; }
int Move::PromotionPieceType() const {
    switch (MoveFlag()) { case PromoteToRookFlag: return Piece::Rook; case PromoteToKnightFlag: return Piece::Knight; case PromoteToBishopFlag: return Piece::Bishop; case PromoteToQueenFlag: return Piece::Queen; default: return Piece::None; }
}
Move Move::NullMove() { return Move(0); }
bool Move::SameMove(const Move& a, const Move& b) { return a.moveValue == b.moveValue; }

GameState::GameState(int c,int e,int cr,int f,uint64_t z):capturedPieceType(c),enPassantFile(e),castlingRights(cr),fiftyMoveCounter(f),zobristKey(z){}
bool GameState::HasKingsideCastleRight(bool white) const { int mask = white ? 1 : 4; return (castlingRights & mask) != 0; }
bool GameState::HasQueensideCastleRight(bool white) const { int mask = white ? 2 : 8; return (castlingRights & mask) != 0; }

PieceList::PieceList(int maxPieceCount) : occupiedSquares(maxPieceCount, 0) {}
int PieceList::Count() const { return numPieces; }
void PieceList::AddPieceAtSquare(int square) { occupiedSquares[numPieces] = square; map[square] = numPieces; numPieces++; }
void PieceList::RemovePieceAtSquare(int square) { int pieceIndex = map[square]; occupiedSquares[pieceIndex] = occupiedSquares[numPieces - 1]; map[occupiedSquares[pieceIndex]] = pieceIndex; numPieces--; }
void PieceList::MovePiece(int startSquare, int targetSquare) { int pieceIndex = map[startSquare]; occupiedSquares[pieceIndex] = targetSquare; map[targetSquare] = pieceIndex; }
int PieceList::operator[](int index) const { return occupiedSquares[index]; }

int BoardHelper::RankIndex(int squareIndex) { return squareIndex >> 3; }
int BoardHelper::FileIndex(int squareIndex) { return squareIndex & 0b000111; }
int BoardHelper::IndexFromCoord(int fileIndex, int rankIndex) { return rankIndex * 8 + fileIndex; }
int BoardHelper::IndexFromCoord(const Coord& coord) { return IndexFromCoord(coord.fileIndex, coord.rankIndex); }
Coord BoardHelper::CoordFromIndex(int squareIndex) { return Coord(FileIndex(squareIndex), RankIndex(squareIndex)); }
bool BoardHelper::LightSquare(int fileIndex, int rankIndex) { return (fileIndex + rankIndex) % 2 != 0; }
bool BoardHelper::LightSquare(int squareIndex) { return LightSquare(FileIndex(squareIndex), RankIndex(squareIndex)); }
std::string BoardHelper::SquareNameFromCoordinate(int fileIndex, int rankIndex) { return std::string(1,fileNames[fileIndex]) + std::to_string(rankIndex+1); }
std::string BoardHelper::SquareNameFromIndex(int squareIndex) { return SquareNameFromCoordinate(CoordFromIndex(squareIndex)); }
std::string BoardHelper::SquareNameFromCoordinate(const Coord& coord) { return SquareNameFromCoordinate(coord.fileIndex, coord.rankIndex); }
int BoardHelper::SquareIndexFromName(const std::string& name) { return IndexFromCoord(std::string(fileNames).find(name[0]), std::string(rankNames).find(name[1])); }
bool BoardHelper::IsValidCoordinate(int x, int y) { return x>=0&&x<8&&y>=0&&y<8; }
bool BoardHelper::IsValidCoordinate(const Coord& coord) { return IsValidCoordinate(coord.fileIndex, coord.rankIndex); }
std::string BoardHelper::CreateDiagram(const Board& board, bool blackAtTop, bool includeFen, bool includeZobristKey) {
    std::ostringstream out;
    auto printRanks = [&](int start, int end, int step) {
        for (int rank = start; rank != end; rank += step) {
            out << "+---+---+---+---+---+---+---+---+\n";
            out << "|";
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                char c = board.Square[sq] == Piece::None ? ' ' : Piece::GetSymbol(board.Square[sq]);
                out << ' ' << c << " |";
            }
            out << ' ' << (rank + 1) << "\n";
        }
        out << "+---+---+---+---+---+---+---+---+\n";
        out << "  a   b   c   d   e   f   g   h\n";
    };
    if (blackAtTop) printRanks(7,-1,-1); else printRanks(0,8,1);
    if (includeFen) out << "Fen         : " << FenUtility::CurrentFen(board) << "\n";
    if (includeZobristKey) out << "Zobrist Key : " << board.ZobristKey() << "\n";
    return out.str();
}
} // namespace Chess::Core
