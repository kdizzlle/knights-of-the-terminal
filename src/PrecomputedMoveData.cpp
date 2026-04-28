#include "ChessEngine.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <numeric>
#include <random>
#include <utility>

using std::uint64_t;

namespace Chess::Core {
std::array<uint64_t,64> BitBoardUtility::KnightAttacks{};
std::array<uint64_t,64> BitBoardUtility::KingMoves{};
std::array<uint64_t,64> BitBoardUtility::WhitePawnAttacks{};
std::array<uint64_t,64> BitBoardUtility::BlackPawnAttacks{};
std::array<uint64_t,64> Bits::WhitePassedPawnMask{};
std::array<uint64_t,64> Bits::BlackPassedPawnMask{};
std::array<uint64_t,64> Bits::WhitePawnSupportMask{};
std::array<uint64_t,64> Bits::BlackPawnSupportMask{};
std::array<uint64_t,8> Bits::FileMask{};
std::array<uint64_t,8> Bits::AdjacentFileMasks{};
std::array<uint64_t,64> Bits::KingSafetyMask{};
std::array<uint64_t,64> Bits::WhiteForwardFileMask{};
std::array<uint64_t,64> Bits::BlackForwardFileMask{};
std::array<uint64_t,8> Bits::TripleFileMask{};
std::array<std::array<uint64_t,64>,64> PrecomputedMoveData::alignMask{};
std::array<std::array<uint64_t,64>,8> PrecomputedMoveData::dirRayMask{};
std::array<std::array<int,8>,64> PrecomputedMoveData::numSquaresToEdge{};
std::array<std::vector<std::uint8_t>,64> PrecomputedMoveData::knightMoves{};
std::array<std::vector<std::uint8_t>,64> PrecomputedMoveData::kingMoves{};
std::array<std::vector<int>,64> PrecomputedMoveData::pawnAttacksWhite{};
std::array<std::vector<int>,64> PrecomputedMoveData::pawnAttacksBlack{};
std::array<int,127> PrecomputedMoveData::directionLookup{};
std::array<uint64_t,64> PrecomputedMoveData::kingAttackBitboards{};
std::array<uint64_t,64> PrecomputedMoveData::knightAttackBitboards{};
std::array<std::array<uint64_t,2>,64> PrecomputedMoveData::pawnAttackBitboards{};
std::array<uint64_t,64> PrecomputedMoveData::rookMoves{};
std::array<uint64_t,64> PrecomputedMoveData::bishopMoves{};
std::array<uint64_t,64> PrecomputedMoveData::queenMoves{};
std::array<std::array<int,64>,64> PrecomputedMoveData::OrthogonalDistance{};
std::array<std::array<int,64>,64> PrecomputedMoveData::kingDistance{};
std::array<int,64> PrecomputedMoveData::CentreManhattanDistance{};
std::array<uint64_t,64> Magic::RookMask{};
std::array<uint64_t,64> Magic::BishopMask{};
std::array<std::vector<uint64_t>,64> Magic::RookAttacks{};
std::array<std::vector<uint64_t>,64> Magic::BishopAttacks{};
std::array<std::array<uint64_t,64>, Piece::MaxPieceIndex+1> Zobrist::piecesArray{};
std::array<uint64_t,16> Zobrist::castlingRights{};
std::array<uint64_t,9> Zobrist::enPassantFile{};
uint64_t Zobrist::sideToMove{};
std::array<std::array<int,64>, Piece::MaxPieceIndex+1> PieceSquareTable::Tables{};
std::array<int,64> PieceSquareTable::Pawns{};
std::array<int,64> PieceSquareTable::PawnsEnd{};
std::array<int,64> PieceSquareTable::Rooks{};
std::array<int,64> PieceSquareTable::Knights{};
std::array<int,64> PieceSquareTable::Bishops{};
std::array<int,64> PieceSquareTable::Queens{};
std::array<int,64> PieceSquareTable::KingStart{};
std::array<int,64> PieceSquareTable::KingEnd{};
std::array<std::vector<int>,64> PrecomputedEvaluationData::PawnShieldSquaresWhite{};
std::array<std::vector<int>,64> PrecomputedEvaluationData::PawnShieldSquaresBlack{};


namespace {
struct StaticInit {
    StaticInit() {
        BitBoardUtility::Init();
        Bits::Init();
        PrecomputedMoveData::Init();
        Magic::Init();
        Zobrist::Init();
        PieceSquareTable::Init();
        PrecomputedEvaluationData::Init();
    }
} staticInit;
}

int BitBoardUtility::PopLSB(uint64_t& b) { int i = std::countr_zero(b); b &= (b - 1); return i; }
void BitBoardUtility::SetSquare(uint64_t& bitboard, int squareIndex) { bitboard |= 1ULL << squareIndex; }
void BitBoardUtility::ClearSquare(uint64_t& bitboard, int squareIndex) { bitboard &= ~(1ULL << squareIndex); }
void BitBoardUtility::ToggleSquare(uint64_t& bitboard, int squareIndex) { bitboard ^= 1ULL << squareIndex; }
void BitBoardUtility::ToggleSquares(uint64_t& bitboard, int a, int b) { bitboard ^= ((1ULL << a) | (1ULL << b)); }
bool BitBoardUtility::ContainsSquare(uint64_t bitboard, int square) { return ((bitboard >> square) & 1ULL) != 0; }
uint64_t BitBoardUtility::PawnAttacks(uint64_t pawnBitboard, bool isWhite) { return isWhite ? (((pawnBitboard << 9) & notAFile) | ((pawnBitboard << 7) & notHFile)) : (((pawnBitboard >> 7) & notAFile) | ((pawnBitboard >> 9) & notHFile)); }
uint64_t BitBoardUtility::Shift(uint64_t bitboard, int n) { return n > 0 ? bitboard << n : bitboard >> (-n); }
void BitBoardUtility::Init() {
    static bool init = false; if (init) return; init = true;
    const std::array<std::pair<int,int>,4> orthoDir{{{-1,0},{0,1},{1,0},{0,-1}}};
    const std::array<std::pair<int,int>,4> diagDir{{{-1,-1},{-1,1},{1,1},{1,-1}}};
    const std::array<std::pair<int,int>,8> knightJumps{{{-2,-1},{-2,1},{-1,2},{1,2},{2,1},{2,-1},{1,-2},{-1,-2}}};
    auto valid = [](int x, int y, int& target) { if (x>=0&&x<8&&y>=0&&y<8) { target = y*8+x; return true; } return false; };
    for (int y=0;y<8;y++) for (int x=0;x<8;x++) {
        int squareIndex = y * 8 + x;
        for (int dirIndex = 0; dirIndex < 4; dirIndex++) {
            for (int dst = 1; dst < 8; dst++) {
                int orthoX = x + orthoDir[dirIndex].first * dst; int orthoY = y + orthoDir[dirIndex].second * dst; int diagX = x + diagDir[dirIndex].first * dst; int diagY = y + diagDir[dirIndex].second * dst; int idx;
                if (valid(orthoX, orthoY, idx) && dst == 1) KingMoves[squareIndex] |= 1ULL << idx;
                if (valid(diagX, diagY, idx) && dst == 1) KingMoves[squareIndex] |= 1ULL << idx;
            }
            for (auto [dx,dy] : knightJumps) { int idx; if (valid(x+dx, y+dy, idx)) KnightAttacks[squareIndex] |= 1ULL << idx; }
        }
        uint64_t bit = 1ULL << squareIndex;
        WhitePawnAttacks[squareIndex] = PawnAttacks(bit, true);
        BlackPawnAttacks[squareIndex] = PawnAttacks(bit, false);
    }
}

void Bits::Init() {
    static bool init = false; if (init) return; init = true;
    for (int i = 0; i < 8; i++) {
        FileMask[i] = FileA << i; uint64_t left = i > 0 ? FileA << (i - 1) : 0; uint64_t right = i < 7 ? FileA << (i + 1) : 0; AdjacentFileMasks[i] = left | right;
    }
    for (int i = 0; i < 8; i++) { int clampedFile = std::clamp(i,1,6); TripleFileMask[i] = FileMask[clampedFile] | AdjacentFileMasks[clampedFile]; }
    for (int square = 0; square < 64; square++) {
        int file = BoardHelper::FileIndex(square); int rank = BoardHelper::RankIndex(square); uint64_t adjacentFiles = (FileA << std::max(0, file - 1)) | (FileA << std::min(7, file + 1));
        uint64_t whiteForwardMask = ~(~0ULL >> (64 - 8 * (rank + 1))); uint64_t blackForwardMask = ((1ULL << (8 * rank)) - 1ULL);
        WhitePassedPawnMask[square] = ((FileA << file) | adjacentFiles) & whiteForwardMask; BlackPassedPawnMask[square] = ((FileA << file) | adjacentFiles) & blackForwardMask;
        uint64_t adjacent = (((square>0?1ULL << (square - 1):0) | (square<63?1ULL << (square + 1):0)) & adjacentFiles);
        WhitePawnSupportMask[square] = adjacent | BitBoardUtility::Shift(adjacent, -8); BlackPawnSupportMask[square] = adjacent | BitBoardUtility::Shift(adjacent, 8);
        WhiteForwardFileMask[square] = whiteForwardMask & FileMask[file]; BlackForwardFileMask[square] = blackForwardMask & FileMask[file]; KingSafetyMask[square] = BitBoardUtility::KingMoves[square] | (1ULL << square);
    }
}




// Static storage

int PrecomputedMoveData::NumRookMovesToReachSquare(int startSquare, int targetSquare) {
    return OrthogonalDistance[startSquare][targetSquare];
}

int PrecomputedMoveData::NumKingMovesToReachSquare(int startSquare, int targetSquare) {
    return kingDistance[startSquare][targetSquare];
}

void PrecomputedMoveData::Init() {
    static bool init = false; if (init) return; init = true;

    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        int y = squareIndex / 8;
        int x = squareIndex - y * 8;

        int north = 7 - y;
        int south = y;
        int west = x;
        int east = 7 - x;
        numSquaresToEdge[squareIndex] = {north, south, west, east,
            std::min(north, west), std::min(south, east), std::min(north, east), std::min(south, west)};

        static constexpr std::array<int,8> allKnightJumps = {15,17,-17,-15,10,-6,6,-10};
        for (int delta : allKnightJumps) {
            int target = squareIndex + delta;
            if (target >= 0 && target < 64) {
                int ty = target / 8;
                int tx = target - ty * 8;
                int maxCoordMoveDst = std::max(std::abs(x - tx), std::abs(y - ty));
                if (maxCoordMoveDst == 2) {
                    knightMoves[squareIndex].push_back(static_cast<std::uint8_t>(target));
                    knightAttackBitboards[squareIndex] |= 1ULL << target;
                }
            }
        }

        for (int delta : directionOffsets) {
            int target = squareIndex + delta;
            if (target >= 0 && target < 64) {
                int ty = target / 8;
                int tx = target - ty * 8;
                int maxCoordMoveDst = std::max(std::abs(x - tx), std::abs(y - ty));
                if (maxCoordMoveDst == 1) {
                    kingMoves[squareIndex].push_back(static_cast<std::uint8_t>(target));
                    kingAttackBitboards[squareIndex] |= 1ULL << target;
                }
            }
        }

        if (x > 0) {
            if (y < 7) {
                pawnAttacksWhite[squareIndex].push_back(squareIndex + 7);
                pawnAttackBitboards[squareIndex][Board::WhiteIndex] |= 1ULL << (squareIndex + 7);
            }
            if (y > 0) {
                pawnAttacksBlack[squareIndex].push_back(squareIndex - 9);
                pawnAttackBitboards[squareIndex][Board::BlackIndex] |= 1ULL << (squareIndex - 9);
            }
        }
        if (x < 7) {
            if (y < 7) {
                pawnAttacksWhite[squareIndex].push_back(squareIndex + 9);
                pawnAttackBitboards[squareIndex][Board::WhiteIndex] |= 1ULL << (squareIndex + 9);
            }
            if (y > 0) {
                pawnAttacksBlack[squareIndex].push_back(squareIndex - 7);
                pawnAttackBitboards[squareIndex][Board::BlackIndex] |= 1ULL << (squareIndex - 7);
            }
        }

        for (int directionIndex = 0; directionIndex < 4; directionIndex++) {
            int currentDirOffset = directionOffsets[directionIndex];
            for (int n = 0; n < numSquaresToEdge[squareIndex][directionIndex]; n++) {
                int targetSquare = squareIndex + currentDirOffset * (n + 1);
                rookMoves[squareIndex] |= 1ULL << targetSquare;
            }
        }
        for (int directionIndex = 4; directionIndex < 8; directionIndex++) {
            int currentDirOffset = directionOffsets[directionIndex];
            for (int n = 0; n < numSquaresToEdge[squareIndex][directionIndex]; n++) {
                int targetSquare = squareIndex + currentDirOffset * (n + 1);
                bishopMoves[squareIndex] |= 1ULL << targetSquare;
            }
        }
        queenMoves[squareIndex] = rookMoves[squareIndex] | bishopMoves[squareIndex];
    }

    for (int i = 0; i < 127; i++) {
        int offset = i - 63;
        int absOffset = std::abs(offset);
        int absDir = 1;
        if (absOffset % 9 == 0) absDir = 9;
        else if (absOffset % 8 == 0) absDir = 8;
        else if (absOffset % 7 == 0) absDir = 7;
        directionLookup[i] = absDir * ((offset > 0) - (offset < 0));
    }

    for (int squareA = 0; squareA < 64; squareA++) {
        Coord coordA = BoardHelper::CoordFromIndex(squareA);
        int fileDstFromCentre = std::max(3 - coordA.fileIndex, coordA.fileIndex - 4);
        int rankDstFromCentre = std::max(3 - coordA.rankIndex, coordA.rankIndex - 4);
        CentreManhattanDistance[squareA] = fileDstFromCentre + rankDstFromCentre;

        for (int squareB = 0; squareB < 64; squareB++) {
            Coord coordB = BoardHelper::CoordFromIndex(squareB);
            int rankDistance = std::abs(coordA.rankIndex - coordB.rankIndex);
            int fileDistance = std::abs(coordA.fileIndex - coordB.fileIndex);
            OrthogonalDistance[squareA][squareB] = fileDistance + rankDistance;
            kingDistance[squareA][squareB] = std::max(fileDistance, rankDistance);
        }
    }

    for (int squareA = 0; squareA < 64; squareA++) {
        for (int squareB = 0; squareB < 64; squareB++) {
            Coord cA = BoardHelper::CoordFromIndex(squareA);
            Coord cB = BoardHelper::CoordFromIndex(squareB);
            Coord delta = cB - cA;
            Coord dir((delta.fileIndex > 0) - (delta.fileIndex < 0), (delta.rankIndex > 0) - (delta.rankIndex < 0));
            for (int i = -8; i < 8; i++) {
                Coord coord = BoardHelper::CoordFromIndex(squareA) + dir * i;
                if (coord.IsValidSquare()) {
                    alignMask[squareA][squareB] |= 1ULL << BoardHelper::IndexFromCoord(coord);
                }
            }
        }
    }

    for (int dirIndex = 0; dirIndex < 8; ++dirIndex) {
        for (int squareIndex = 0; squareIndex < 64; ++squareIndex) {
            for (int n = 0; n < numSquaresToEdge[squareIndex][dirIndex]; ++n) {
                int targetSquare = squareIndex + directionOffsets[dirIndex] * (n + 1);
                dirRayMask[dirIndex][squareIndex] |= 1ULL << targetSquare;
            }
        }
    }
}

std::vector<std::uint64_t> MagicHelper::CreateAllBlockerBitboards(std::uint64_t movementMask) {
    std::vector<int> moveSquareIndices;
    for (int i = 0; i < 64; i++) {
        if (((movementMask >> i) & 1ULL) == 1ULL) moveSquareIndices.push_back(i);
    }
    int numPatterns = 1 << static_cast<int>(moveSquareIndices.size());
    std::vector<std::uint64_t> blockerBitboards(numPatterns, 0ULL);
    for (int patternIndex = 0; patternIndex < numPatterns; patternIndex++) {
        for (int bitIndex = 0; bitIndex < static_cast<int>(moveSquareIndices.size()); bitIndex++) {
            int bit = (patternIndex >> bitIndex) & 1;
            blockerBitboards[patternIndex] |= static_cast<std::uint64_t>(bit) << moveSquareIndices[bitIndex];
        }
    }
    return blockerBitboards;
}

std::uint64_t MagicHelper::CreateMovementMask(int squareIndex, bool ortho) {
    std::uint64_t mask = 0;
    const auto& directions = ortho ? BoardHelper::RookDirections : BoardHelper::BishopDirections;
    Coord startCoord(squareIndex);
    for (const Coord& dir : directions) {
        for (int dst = 1; dst < 8; dst++) {
            Coord coord = startCoord + dir * dst;
            Coord nextCoord = startCoord + dir * (dst + 1);
            if (nextCoord.IsValidSquare()) BitBoardUtility::SetSquare(mask, coord.SquareIndex());
            else break;
        }
    }
    return mask;
}

std::uint64_t MagicHelper::LegalMoveBitboardFromBlockers(int startSquare, std::uint64_t blockerBitboard, bool ortho) {
    std::uint64_t bitboard = 0;
    const auto& directions = ortho ? BoardHelper::RookDirections : BoardHelper::BishopDirections;
    Coord startCoord(startSquare);
    for (const Coord& dir : directions) {
        for (int dst = 1; dst < 8; dst++) {
            Coord coord = startCoord + dir * dst;
            if (coord.IsValidSquare()) {
                BitBoardUtility::SetSquare(bitboard, coord.SquareIndex());
                if (BitBoardUtility::ContainsSquare(blockerBitboard, coord.SquareIndex())) break;
            } else break;
        }
    }
    return bitboard;
}

void Magic::Init() {
    static bool init = false; if (init) return; init = true;
    
    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        RookMask[squareIndex] = MagicHelper::CreateMovementMask(squareIndex, true);
        BishopMask[squareIndex] = MagicHelper::CreateMovementMask(squareIndex, false);
    }

    for (int i = 0; i < 64; i++) {
        auto createTable = [&](int square, bool rook, std::uint64_t magic, int leftShift) {
            int numBits = 64 - leftShift;
            int lookupSize = 1 << numBits;
            std::vector<std::uint64_t> table(lookupSize, 0ULL);
            std::uint64_t movementMask = MagicHelper::CreateMovementMask(square, rook);
            auto blockerPatterns = MagicHelper::CreateAllBlockerBitboards(movementMask);
            for (std::uint64_t pattern : blockerPatterns) {
                std::uint64_t index = (pattern * magic) >> leftShift;
                std::uint64_t moves = MagicHelper::LegalMoveBitboardFromBlockers(square, pattern, rook);
                table[static_cast<std::size_t>(index)] = moves;
            }
            return table;
        };
        RookAttacks[i] = createTable(i, true, PrecomputedMagics::RookMagics[i], PrecomputedMagics::RookShifts[i]);
        BishopAttacks[i] = createTable(i, false, PrecomputedMagics::BishopMagics[i], PrecomputedMagics::BishopShifts[i]);
    }
}

std::uint64_t Magic::GetSliderAttacks(int square, std::uint64_t blockers, bool ortho) {
    return ortho ? GetRookAttacks(square, blockers) : GetBishopAttacks(square, blockers);
}

std::uint64_t Magic::GetRookAttacks(int square, std::uint64_t blockers) {
    std::uint64_t key = ((blockers & RookMask[square]) * PrecomputedMagics::RookMagics[square]) >> PrecomputedMagics::RookShifts[square];
    return RookAttacks[square][static_cast<std::size_t>(key)];
}

std::uint64_t Magic::GetBishopAttacks(int square, std::uint64_t blockers) {
    std::uint64_t key = ((blockers & BishopMask[square]) * PrecomputedMagics::BishopMagics[square]) >> PrecomputedMagics::BishopShifts[square];
    return BishopAttacks[square][static_cast<std::size_t>(key)];
}

static std::uint64_t RandomUnsigned64BitNumber(std::mt19937& rng) {
    std::array<unsigned char, 8> buffer{};
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& b : buffer) b = static_cast<unsigned char>(dist(rng));
    std::uint64_t value = 0;
    std::memcpy(&value, buffer.data(), 8);
    return value;
}

void Zobrist::Init() {
    static bool init = false; if (init) return; init = true;
    constexpr int seed = 29426028;
    std::mt19937 rng(seed);
    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        for (int piece : Piece::PieceIndices) {
            piecesArray[piece][squareIndex] = RandomUnsigned64BitNumber(rng);
        }
    }
    for (auto& v : castlingRights) v = RandomUnsigned64BitNumber(rng);
    for (std::size_t i = 0; i < enPassantFile.size(); i++) {
        enPassantFile[i] = (i == 0) ? 0ULL : RandomUnsigned64BitNumber(rng);
    }
    sideToMove = RandomUnsigned64BitNumber(rng);
}

std::uint64_t Zobrist::CalculateZobristKey(const Board& board) {
    Init();
    std::uint64_t zobristKey = 0;
    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        int piece = board.Square[squareIndex];
        if (Piece::PieceType(piece) != Piece::None) {
            zobristKey ^= piecesArray[piece][squareIndex];
        }
    }
    zobristKey ^= enPassantFile[board.CurrentGameState.enPassantFile];
    if (board.MoveColour() == Piece::Black) zobristKey ^= sideToMove;
    zobristKey ^= castlingRights[board.CurrentGameState.castlingRights];
    return zobristKey;
}

int PieceSquareTable::Read(const std::array<int,64>& table, int square, bool isWhite) {
    if (isWhite) {
        int file = BoardHelper::FileIndex(square);
        int rank = BoardHelper::RankIndex(square);
        rank = 7 - rank;
        square = BoardHelper::IndexFromCoord(file, rank);
    }
    return table[square];
}

int PieceSquareTable::Read(int piece, int square) {
    return Tables[piece][square];
}

std::array<int,64> PieceSquareTable::GetFlippedTable(const std::array<int,64>& table) {
    std::array<int,64> flipped{};
    for (int i = 0; i < 64; ++i) {
        Coord coord(i);
        Coord flippedCoord(coord.fileIndex, 7 - coord.rankIndex);
        flipped[flippedCoord.SquareIndex()] = table[i];
    }
    return flipped;
}

void PieceSquareTable::Init() {
    static bool init = false; if (init) return; init = true;

    Pawns = {0,0,0,0,0,0,0,0, 50,50,50,50,50,50,50,50, 10,10,20,30,30,20,10,10, 5,5,10,25,25,10,5,5, 0,0,0,20,20,0,0,0, 5,-5,-10,0,0,-10,-5,5, 5,10,10,-20,-20,10,10,5, 0,0,0,0,0,0,0,0};
    PawnsEnd = {0,0,0,0,0,0,0,0, 80,80,80,80,80,80,80,80, 50,50,50,50,50,50,50,50, 30,30,30,30,30,30,30,30, 20,20,20,20,20,20,20,20, 10,10,10,10,10,10,10,10, 10,10,10,10,10,10,10,10, 0,0,0,0,0,0,0,0};
    Rooks = {0,0,0,0,0,0,0,0, 5,10,10,10,10,10,10,5, -5,0,0,0,0,0,0,-5, -5,0,0,0,0,0,0,-5, -5,0,0,0,0,0,0,-5, -5,0,0,0,0,0,0,-5, -5,0,0,0,0,0,0,-5, 0,0,0,5,5,0,0,0};
    Knights = {-50,-40,-30,-30,-30,-30,-40,-50, -40,-20,0,0,0,0,-20,-40, -30,0,10,15,15,10,0,-30, -30,5,15,20,20,15,5,-30, -30,0,15,20,20,15,0,-30, -30,5,10,15,15,10,5,-30, -40,-20,0,5,5,0,-20,-40, -50,-40,-30,-30,-30,-30,-40,-50};
    Bishops = {-20,-10,-10,-10,-10,-10,-10,-20, -10,0,0,0,0,0,0,-10, -10,0,5,10,10,5,0,-10, -10,5,5,10,10,5,5,-10, -10,0,10,10,10,10,0,-10, -10,10,10,10,10,10,10,-10, -10,5,0,0,0,0,5,-10, -20,-10,-10,-10,-10,-10,-10,-20};
    Queens = {-20,-10,-10,-5,-5,-10,-10,-20, -10,0,0,0,0,0,0,-10, -10,0,5,5,5,5,0,-10, -5,0,5,5,5,5,0,-5, 0,0,5,5,5,5,0,-5, -10,5,5,5,5,5,0,-10, -10,0,5,0,0,0,0,-10, -20,-10,-10,-5,-5,-10,-10,-20};
    KingStart = {-80,-70,-70,-70,-70,-70,-70,-80, -60,-60,-60,-60,-60,-60,-60,-60, -40,-50,-50,-60,-60,-50,-50,-40, -30,-40,-40,-50,-50,-40,-40,-30, -20,-30,-30,-40,-40,-30,-30,-20, -10,-20,-20,-20,-20,-20,-20,-10, 20,20,-5,-5,-5,-5,20,20, 20,30,10,0,0,10,30,20};
    KingEnd = {-20,-10,-10,-10,-10,-10,-10,-20, -5,0,5,5,5,5,0,-5, -10,-5,20,30,30,20,-5,-10, -15,-10,35,45,45,35,-10,-15, -20,-15,30,40,40,30,-15,-20, -25,-20,20,25,25,20,-20,-25, -30,-25,0,0,0,0,-25,-30, -50,-30,-30,-30,-30,-30,-30,-50};

    Tables.fill({});
    Tables[Piece::MakePiece(Piece::Pawn, Piece::White)] = Pawns;
    Tables[Piece::MakePiece(Piece::Rook, Piece::White)] = Rooks;
    Tables[Piece::MakePiece(Piece::Knight, Piece::White)] = Knights;
    Tables[Piece::MakePiece(Piece::Bishop, Piece::White)] = Bishops;
    Tables[Piece::MakePiece(Piece::Queen, Piece::White)] = Queens;

    Tables[Piece::MakePiece(Piece::Pawn, Piece::Black)] = GetFlippedTable(Pawns);
    Tables[Piece::MakePiece(Piece::Rook, Piece::Black)] = GetFlippedTable(Rooks);
    Tables[Piece::MakePiece(Piece::Knight, Piece::Black)] = GetFlippedTable(Knights);
    Tables[Piece::MakePiece(Piece::Bishop, Piece::Black)] = GetFlippedTable(Bishops);
    Tables[Piece::MakePiece(Piece::Queen, Piece::Black)] = GetFlippedTable(Queens);
}


void PrecomputedEvaluationData::Init() {
    static bool init = false; if (init) return; init = true;
    for (int squareIndex = 0; squareIndex < 64; ++squareIndex) {
        std::vector<int> shieldIndicesWhite;
        std::vector<int> shieldIndicesBlack;
        Coord coord(squareIndex);
        int rank = coord.rankIndex;
        int file = std::clamp(coord.fileIndex, 1, 6);
        auto addIfValid = [](const Coord& c, std::vector<int>& list) {
            if (c.IsValidSquare()) list.push_back(c.SquareIndex());
        };
        for (int fileOffset = -1; fileOffset <= 1; ++fileOffset) {
            addIfValid(Coord(file + fileOffset, rank + 1), shieldIndicesWhite);
            addIfValid(Coord(file + fileOffset, rank - 1), shieldIndicesBlack);
        }
        for (int fileOffset = -1; fileOffset <= 1; ++fileOffset) {
            addIfValid(Coord(file + fileOffset, rank + 2), shieldIndicesWhite);
            addIfValid(Coord(file + fileOffset, rank - 2), shieldIndicesBlack);
        }
        PawnShieldSquaresWhite[squareIndex] = std::move(shieldIndicesWhite);
        PawnShieldSquaresBlack[squareIndex] = std::move(shieldIndicesBlack);
    }
}
} // namespace Chess::Core
