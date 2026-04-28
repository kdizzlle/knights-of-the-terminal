#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <thread>
#include <vector>

namespace Chess::Core {

struct Coord {
    int fileIndex{};
    int rankIndex{};
    Coord() = default;
    Coord(int fileIndex, int rankIndex);
    explicit Coord(int squareIndex);
    bool IsLightSquare() const;
    int CompareTo(const Coord& other) const;
    bool IsValidSquare() const;
    int SquareIndex() const;
};
Coord operator+(const Coord& a, const Coord& b);
Coord operator-(const Coord& a, const Coord& b);
Coord operator*(const Coord& a, int m);
Coord operator*(int m, const Coord& a);

struct Piece {
    static constexpr int None = 0;
    static constexpr int Pawn = 1;
    static constexpr int Knight = 2;
    static constexpr int Bishop = 3;
    static constexpr int Rook = 4;
    static constexpr int Queen = 5;
    static constexpr int King = 6;
    static constexpr int White = 0;
    static constexpr int Black = 8;
    static constexpr int WhitePawn = Pawn | White;
    static constexpr int WhiteKnight = Knight | White;
    static constexpr int WhiteBishop = Bishop | White;
    static constexpr int WhiteRook = Rook | White;
    static constexpr int WhiteQueen = Queen | White;
    static constexpr int WhiteKing = King | White;
    static constexpr int BlackPawn = Pawn | Black;
    static constexpr int BlackKnight = Knight | Black;
    static constexpr int BlackBishop = Bishop | Black;
    static constexpr int BlackRook = Rook | Black;
    static constexpr int BlackQueen = Queen | Black;
    static constexpr int BlackKing = King | Black;
    static constexpr int MaxPieceIndex = BlackKing;
    static constexpr int typeMask = 0b0111;
    static constexpr int colourMask = 0b1000;
    static inline constexpr std::array<int, 12> PieceIndices = {
        WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing,
        BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing
    };
    static int MakePiece(int pieceType, int pieceColour);
    static int MakePiece(int pieceType, bool pieceIsWhite);
    static bool IsColour(int piece, int colour);
    static bool IsWhite(int piece);
    static int PieceColour(int piece);
    static int PieceType(int piece);
    static bool IsOrthogonalSlider(int piece);
    static bool IsDiagonalSlider(int piece);
    static bool IsSlidingPiece(int piece);
    static char GetSymbol(int piece);
    static int GetPieceTypeFromSymbol(char symbol);
};

struct Move {
    std::uint16_t moveValue{};
    static constexpr int NoFlag = 0b0000;
    static constexpr int EnPassantCaptureFlag = 0b0001;
    static constexpr int CastleFlag = 0b0010;
    static constexpr int PawnTwoUpFlag = 0b0011;
    static constexpr int PromoteToQueenFlag = 0b0100;
    static constexpr int PromoteToKnightFlag = 0b0101;
    static constexpr int PromoteToRookFlag = 0b0110;
    static constexpr int PromoteToBishopFlag = 0b0111;
    static constexpr std::uint16_t startSquareMask = 0b0000000000111111;
    static constexpr std::uint16_t targetSquareMask = 0b0000111111000000;
    static constexpr std::uint16_t flagMask = 0b1111000000000000;
    Move() = default;
    explicit Move(std::uint16_t moveValue);
    Move(int startSquare, int targetSquare);
    Move(int startSquare, int targetSquare, int flag);
    std::uint16_t Value() const;
    bool IsNull() const;
    int StartSquare() const;
    int TargetSquare() const;
    bool IsPromotion() const;
    int MoveFlag() const;
    int PromotionPieceType() const;
    static Move NullMove();
    static bool SameMove(const Move& a, const Move& b);
};

struct GameState {
    int capturedPieceType{};
    int enPassantFile{};
    int castlingRights{};
    int fiftyMoveCounter{};
    std::uint64_t zobristKey{};
    static constexpr int ClearWhiteKingsideMask = 0b1110;
    static constexpr int ClearWhiteQueensideMask = 0b1101;
    static constexpr int ClearBlackKingsideMask = 0b1011;
    static constexpr int ClearBlackQueensideMask = 0b0111;
    GameState() = default;
    GameState(int capturedPieceType, int enPassantFile, int castlingRights, int fiftyMoveCounter, std::uint64_t zobristKey);
    bool HasKingsideCastleRight(bool white) const;
    bool HasQueensideCastleRight(bool white) const;
};

class PieceList {
public:
    explicit PieceList(int maxPieceCount = 16);
    int Count() const;
    void AddPieceAtSquare(int square);
    void RemovePieceAtSquare(int square);
    void MovePiece(int startSquare, int targetSquare);
    int operator[](int index) const;
    std::vector<int> occupiedSquares;
private:
    std::array<int, 64> map{};
    int numPieces{};
};

struct BoardHelper {
    static inline constexpr const char* fileNames = "abcdefgh";
    static inline constexpr const char* rankNames = "12345678";
    static constexpr int a1=0,b1=1,c1=2,d1=3,e1=4,f1=5,g1=6,h1=7;
    static constexpr int a8=56,b8=57,c8=58,d8=59,e8=60,f8=61,g8=62,h8=63;
    static int RankIndex(int squareIndex);
    static int FileIndex(int squareIndex);
    static int IndexFromCoord(int fileIndex, int rankIndex);
    static int IndexFromCoord(const Coord& coord);
    static Coord CoordFromIndex(int squareIndex);
    static bool LightSquare(int fileIndex, int rankIndex);
    static bool LightSquare(int squareIndex);
    static std::string SquareNameFromCoordinate(int fileIndex, int rankIndex);
    static std::string SquareNameFromIndex(int squareIndex);
    static std::string SquareNameFromCoordinate(const Coord& coord);
    static int SquareIndexFromName(const std::string& name);
    static bool IsValidCoordinate(int x, int y);
    static bool IsValidCoordinate(const Coord& coord);
    static std::string CreateDiagram(const class Board& board, bool blackAtTop = true, bool includeFen = true, bool includeZobristKey = true);
    static inline const std::array<Coord,4> RookDirections = {Coord(-1,0), Coord(1,0), Coord(0,1), Coord(0,-1)};
    static inline const std::array<Coord,4> BishopDirections = {Coord(-1,1), Coord(1,1), Coord(1,-1), Coord(-1,-1)};
};

struct BitBoardUtility {
    static constexpr std::uint64_t FileA = 0x101010101010101ULL;
    static constexpr std::uint64_t Rank1 = 0b11111111ULL;
    static constexpr std::uint64_t Rank2 = Rank1 << 8;
    static constexpr std::uint64_t Rank3 = Rank2 << 8;
    static constexpr std::uint64_t Rank4 = Rank3 << 8;
    static constexpr std::uint64_t Rank5 = Rank4 << 8;
    static constexpr std::uint64_t Rank6 = Rank5 << 8;
    static constexpr std::uint64_t Rank7 = Rank6 << 8;
    static constexpr std::uint64_t Rank8 = Rank7 << 8;
    static constexpr std::uint64_t notAFile = ~FileA;
    static constexpr std::uint64_t notHFile = ~(FileA << 7);
    static std::array<std::uint64_t,64> KnightAttacks;
    static std::array<std::uint64_t,64> KingMoves;
    static std::array<std::uint64_t,64> WhitePawnAttacks;
    static std::array<std::uint64_t,64> BlackPawnAttacks;
    static void Init();
    static int PopLSB(std::uint64_t& b);
    static void SetSquare(std::uint64_t& bitboard, int squareIndex);
    static void ClearSquare(std::uint64_t& bitboard, int squareIndex);
    static void ToggleSquare(std::uint64_t& bitboard, int squareIndex);
    static void ToggleSquares(std::uint64_t& bitboard, int squareA, int squareB);
    static bool ContainsSquare(std::uint64_t bitboard, int square);
    static std::uint64_t PawnAttacks(std::uint64_t pawnBitboard, bool isWhite);
    static std::uint64_t Shift(std::uint64_t bitboard, int numSquaresToShift);
};

struct Bits {
    static constexpr std::uint64_t FileA = 0x101010101010101ULL;
    static constexpr std::uint64_t WhiteKingsideMask = (1ULL<<BoardHelper::f1) | (1ULL<<BoardHelper::g1);
    static constexpr std::uint64_t BlackKingsideMask = (1ULL<<BoardHelper::f8) | (1ULL<<BoardHelper::g8);
    static constexpr std::uint64_t WhiteQueensideMask2 = (1ULL<<BoardHelper::d1) | (1ULL<<BoardHelper::c1);
    static constexpr std::uint64_t BlackQueensideMask2 = (1ULL<<BoardHelper::d8) | (1ULL<<BoardHelper::c8);
    static constexpr std::uint64_t WhiteQueensideMask = WhiteQueensideMask2 | (1ULL<<BoardHelper::b1);
    static constexpr std::uint64_t BlackQueensideMask = BlackQueensideMask2 | (1ULL<<BoardHelper::b8);
    static std::array<std::uint64_t,64> WhitePassedPawnMask;
    static std::array<std::uint64_t,64> BlackPassedPawnMask;
    static std::array<std::uint64_t,64> WhitePawnSupportMask;
    static std::array<std::uint64_t,64> BlackPawnSupportMask;
    static std::array<std::uint64_t,8> FileMask;
    static std::array<std::uint64_t,8> AdjacentFileMasks;
    static std::array<std::uint64_t,64> KingSafetyMask;
    static std::array<std::uint64_t,64> WhiteForwardFileMask;
    static std::array<std::uint64_t,64> BlackForwardFileMask;
    static std::array<std::uint64_t,8> TripleFileMask;
    static void Init();
};

struct PrecomputedMoveData {
    static std::array<std::array<std::uint64_t,64>,64> alignMask;
    static std::array<std::array<std::uint64_t,64>,8> dirRayMask;
    static inline constexpr std::array<int,8> directionOffsets = {8,-8,-1,1,7,-7,9,-9};
    static inline const std::array<Coord,8> dirOffsets2D = {Coord(0,1),Coord(0,-1),Coord(-1,0),Coord(1,0),Coord(-1,1),Coord(1,-1),Coord(1,1),Coord(-1,-1)};
    static std::array<std::array<int,8>,64> numSquaresToEdge;
    static std::array<std::vector<std::uint8_t>,64> knightMoves;
    static std::array<std::vector<std::uint8_t>,64> kingMoves;
    static inline constexpr std::array<std::array<std::uint8_t,2>,2> pawnAttackDirections = {{{4,6},{7,5}}};
    static std::array<std::vector<int>,64> pawnAttacksWhite;
    static std::array<std::vector<int>,64> pawnAttacksBlack;
    static std::array<int,127> directionLookup;
    static std::array<std::uint64_t,64> kingAttackBitboards;
    static std::array<std::uint64_t,64> knightAttackBitboards;
    static std::array<std::array<std::uint64_t,2>,64> pawnAttackBitboards;
    static std::array<std::uint64_t,64> rookMoves;
    static std::array<std::uint64_t,64> bishopMoves;
    static std::array<std::uint64_t,64> queenMoves;
    static std::array<std::array<int,64>,64> OrthogonalDistance;
    static std::array<std::array<int,64>,64> kingDistance;
    static std::array<int,64> CentreManhattanDistance;
    static void Init();
    static int NumRookMovesToReachSquare(int startSquare, int targetSquare);
    static int NumKingMovesToReachSquare(int startSquare, int targetSquare);
};

struct MagicHelper {
    static std::vector<std::uint64_t> CreateAllBlockerBitboards(std::uint64_t movementMask);
    static std::uint64_t CreateMovementMask(int squareIndex, bool ortho);
    static std::uint64_t LegalMoveBitboardFromBlockers(int startSquare, std::uint64_t blockerBitboard, bool ortho);
};

struct PrecomputedMagics {
    static inline constexpr std::array<int,64> RookShifts = {52,52,52,52,52,52,52,52,53,53,53,54,53,53,54,53,53,54,54,54,53,53,54,53,53,54,53,53,54,54,54,53,52,54,53,53,53,53,54,53,52,53,54,54,53,53,54,53,53,54,54,54,53,53,54,53,52,53,53,53,53,53,53,52};
    static inline constexpr std::array<int,64> BishopShifts = {58,60,59,59,59,59,60,58,60,59,59,59,59,59,59,60,59,59,57,57,57,57,59,59,59,59,57,55,55,57,59,59,59,59,57,55,55,57,59,59,59,59,57,57,57,57,59,59,60,60,59,59,59,59,60,60,58,60,59,59,59,59,59,58};
    static inline constexpr std::array<std::uint64_t,64> RookMagics = {468374916371625120ULL,18428729537625841661ULL,2531023729696186408ULL,6093370314119450896ULL,13830552789156493815ULL,16134110446239088507ULL,12677615322350354425ULL,5404321144167858432ULL,2111097758984580ULL,18428720740584907710ULL,17293734603602787839ULL,4938760079889530922ULL,7699325603589095390ULL,9078693890218258431ULL,578149610753690728ULL,9496543503900033792ULL,1155209038552629657ULL,9224076274589515780ULL,1835781998207181184ULL,509120063316431138ULL,16634043024132535807ULL,18446673631917146111ULL,9623686630121410312ULL,4648737361302392899ULL,738591182849868645ULL,1732936432546219272ULL,2400543327507449856ULL,5188164365601475096ULL,10414575345181196316ULL,1162492212166789136ULL,9396848738060210946ULL,622413200109881612ULL,7998357718131801918ULL,7719627227008073923ULL,16181433497662382080ULL,18441958655457754079ULL,1267153596645440ULL,18446726464209379263ULL,1214021438038606600ULL,4650128814733526084ULL,9656144899867951104ULL,18444421868610287615ULL,3695311799139303489ULL,10597006226145476632ULL,18436046904206950398ULL,18446726472933277663ULL,3458977943764860944ULL,39125045590687766ULL,9227453435446560384ULL,6476955465732358656ULL,1270314852531077632ULL,2882448553461416064ULL,11547238928203796481ULL,1856618300822323264ULL,2573991788166144ULL,4936544992551831040ULL,13690941749405253631ULL,15852669863439351807ULL,18302628748190527413ULL,12682135449552027479ULL,13830554446930287982ULL,18302628782487371519ULL,7924083509981736956ULL,4734295326018586370ULL};
    static inline constexpr std::array<std::uint64_t,64> BishopMagics = {16509839532542417919ULL,14391803910955204223ULL,1848771770702627364ULL,347925068195328958ULL,5189277761285652493ULL,3750937732777063343ULL,18429848470517967340ULL,17870072066711748607ULL,16715520087474960373ULL,2459353627279607168ULL,7061705824611107232ULL,8089129053103260512ULL,7414579821471224013ULL,9520647030890121554ULL,17142940634164625405ULL,9187037984654475102ULL,4933695867036173873ULL,3035992416931960321ULL,15052160563071165696ULL,5876081268917084809ULL,1153484746652717320ULL,6365855841584713735ULL,2463646859659644933ULL,1453259901463176960ULL,9808859429721908488ULL,2829141021535244552ULL,576619101540319252ULL,5804014844877275314ULL,4774660099383771136ULL,328785038479458864ULL,2360590652863023124ULL,569550314443282ULL,17563974527758635567ULL,11698101887533589556ULL,5764964460729992192ULL,6953579832080335136ULL,1318441160687747328ULL,8090717009753444376ULL,16751172641200572929ULL,5558033503209157252ULL,17100156536247493656ULL,7899286223048400564ULL,4845135427956654145ULL,2368485888099072ULL,2399033289953272320ULL,6976678428284034058ULL,3134241565013966284ULL,8661609558376259840ULL,17275805361393991679ULL,15391050065516657151ULL,11529206229534274423ULL,9876416274250600448ULL,16432792402597134585ULL,11975705497012863580ULL,11457135419348969979ULL,9763749252098620046ULL,16960553411078512574ULL,15563877356819111679ULL,14994736884583272463ULL,9441297368950544394ULL,14537646123432199168ULL,9888547162215157388ULL,18140215579194907366ULL,18374682062228545019ULL};
};

struct Magic {
    static std::array<std::uint64_t,64> RookMask;
    static std::array<std::uint64_t,64> BishopMask;
    static std::array<std::vector<std::uint64_t>,64> RookAttacks;
    static std::array<std::vector<std::uint64_t>,64> BishopAttacks;
    static void Init();
    static std::uint64_t GetSliderAttacks(int square, std::uint64_t blockers, bool ortho);
    static std::uint64_t GetRookAttacks(int square, std::uint64_t blockers);
    static std::uint64_t GetBishopAttacks(int square, std::uint64_t blockers);
};

class Board;

struct Zobrist {
    static std::array<std::array<std::uint64_t,64>, Piece::MaxPieceIndex+1> piecesArray;
    static std::array<std::uint64_t,16> castlingRights;
    static std::array<std::uint64_t,9> enPassantFile;
    static std::uint64_t sideToMove;
    static void Init();
    static std::uint64_t CalculateZobristKey(const Board& board);
};

struct PieceSquareTable {
    static int Read(const std::array<int,64>& table, int square, bool isWhite);
    static int Read(int piece, int square);
    static std::array<int,64> GetFlippedTable(const std::array<int,64>& table);
    static void Init();
    static std::array<std::array<int,64>, Piece::MaxPieceIndex+1> Tables;
    static std::array<int,64> Pawns, PawnsEnd, Rooks, Knights, Bishops, Queens, KingStart, KingEnd;
};

struct PrecomputedEvaluationData {
    static std::array<std::vector<int>,64> PawnShieldSquaresWhite;
    static std::array<std::vector<int>,64> PawnShieldSquaresBlack;
    static void Init();
};

class Board {
public:
    static constexpr int WhiteIndex = 0;
    static constexpr int BlackIndex = 1;
    std::array<int,64> Square{};
    std::array<int,2> KingSquare{};
    std::array<std::uint64_t, Piece::MaxPieceIndex+1> PieceBitboards{};
    std::array<std::uint64_t,2> ColourBitboards{};
    std::uint64_t AllPiecesBitboard{};
    std::uint64_t FriendlyOrthogonalSliders{};
    std::uint64_t FriendlyDiagonalSliders{};
    std::uint64_t EnemyOrthogonalSliders{};
    std::uint64_t EnemyDiagonalSliders{};
    int TotalPieceCountWithoutPawnsAndKings{};
    std::array<PieceList,2> Rooks{PieceList(10),PieceList(10)};
    std::array<PieceList,2> Bishops{PieceList(10),PieceList(10)};
    std::array<PieceList,2> Queens{PieceList(9),PieceList(9)};
    std::array<PieceList,2> Knights{PieceList(10),PieceList(10)};
    std::array<PieceList,2> Pawns{PieceList(8),PieceList(8)};
    bool IsWhiteToMove{};
    std::vector<std::uint64_t> RepetitionPositionHistory;
    int PlyCount{};
    GameState CurrentGameState{};
    std::vector<Move> AllGameMoves;

    struct PositionInfo {
        std::array<int,64> squares{};
        bool whiteToMove{};
        bool whiteCastleKingside{};
        bool whiteCastleQueenside{};
        bool blackCastleKingside{};
        bool blackCastleQueenside{};
        int epFile{};
        int fiftyMovePlyCount{};
        int moveCount{1};
        std::string fen;
    };
    PositionInfo StartPositionInfo;

    Board();
    int MoveColour() const;
    int OpponentColour() const;
    int MoveColourIndex() const;
    int OpponentColourIndex() const;
    int FiftyMoveCounter() const;
    std::uint64_t ZobristKey() const;
    std::string CurrentFEN() const;
    std::string GameStartFEN() const;

    void MakeMove(const Move& move, bool inSearch=false);
    void UnmakeMove(const Move& move, bool inSearch=false);
    void MakeNullMove();
    void UnmakeNullMove();
    bool IsInCheck();
    bool CalculateInCheckState();
    void LoadStartPosition();
    void LoadPosition(const std::string& fen);
    void LoadPosition(const PositionInfo& posInfo);
    std::string ToString() const;
    static Board CreateBoard(const std::string& fen = "");
    static Board CreateBoard(const Board& source);
private:
    std::array<PieceList*, Piece::MaxPieceIndex+1> allPieceLists{};
    std::vector<GameState> gameStateHistory;
    bool cachedInCheckValue{};
    bool hasCachedInCheckValue{};
    void MovePiece(int piece, int startSquare, int targetSquare);
    void UpdateSliderBitboards();
    void Initialize();
};

struct FenUtility {
    static inline const std::string StartPositionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    static Board::PositionInfo PositionFromFen(const std::string& fen);
    static std::string CurrentFen(const Board& board, bool alwaysIncludeEPSquare=true);
    static bool EnPassantCanBeCaptured(int epFileIndex, int epRankIndex, const Board& board);
    static std::string FlipFen(const std::string& fen);
};

class Evaluation {
public:
    static constexpr int PawnValue = 100;
    static constexpr int KnightValue = 300;
    static constexpr int BishopValue = 320;
    static constexpr int RookValue = 500;
    static constexpr int QueenValue = 900;
    struct EvaluationData { int materialScore{}, mopUpScore{}, pieceSquareScore{}, pawnScore{}, pawnShieldScore{}; int Sum() const; };
    struct MaterialInfo { int materialScore{}, numPawns{}, numMajors{}, numMinors{}, numBishops{}, numQueens{}, numRooks{}; std::uint64_t pawns{}, enemyPawns{}; float endgameT{}; MaterialInfo()=default; MaterialInfo(int numPawns,int numKnights,int numBishops,int numQueens,int numRooks,std::uint64_t myPawns,std::uint64_t enemyPawns); };
    Evaluation()=default;
    int Evaluate(Board& board);
    int KingPawnShield(int colourIndex, const MaterialInfo& enemyMaterial, float enemyPieceSquareScore);
    int EvaluatePawns(int colourIndex);
    EvaluationData whiteEval{}, blackEval{};
private:
    Board* board{};
    static inline constexpr std::array<int,7> passedPawnBonuses = {0,120,80,50,30,15,15};
    static inline constexpr std::array<int,9> isolatedPawnPenaltyByCount = {0,-10,-25,-50,-75,-75,-75,-75,-75};
    static inline constexpr std::array<int,6> kingPawnShieldScores = {4,7,4,3,6,3};
    static constexpr float endgameMaterialStart = RookValue * 2 + BishopValue + KnightValue;
    int MopUpEval(bool isWhite, const MaterialInfo& myMaterial, const MaterialInfo& enemyMaterial);
    int CountMaterial(int colourIndex) const;
    int EvaluatePieceSquareTables(bool isWhite, float endgameT) const;
    static int EvaluatePieceSquareTable(const std::array<int,64>& table, const PieceList& pieceList, bool isWhite);
    MaterialInfo GetMaterialInfo(int colourIndex) const;
};

struct MoveUtility {
    static Move GetMoveFromUCIName(const std::string& moveName, Board& board);
    static std::string GetMoveNameUCI(const Move& move);
    static std::string GetMoveNameSAN(const Move& move, Board& board);
    static Move GetMoveFromSAN(Board& board, std::string algebraicMove);
};

struct OpeningBook {
    struct BookMove { std::string moveString; int numTimesPlayed{}; BookMove()=default; BookMove(std::string s,int n):moveString(std::move(s)),numTimesPlayed(n){} }; 
    std::unordered_map<std::string, std::vector<BookMove>> movesByPosition;
    std::mt19937 rng;
    explicit OpeningBook(const std::string& file);
    bool HasBookMove(const std::string& positionFen) const;
    bool TryGetBookMove(Board& board, std::string& moveString, double weightPow = 0.5);
    static std::string RemoveMoveCountersFromFEN(const std::string& fen);
};

struct PGNCreator {
    static std::string CreatePGN(const std::vector<Move>& moves);
    static std::string CreatePGN(Board& board, int result, const std::string& whiteName="", const std::string& blackName="");
    static std::string CreatePGN(const std::vector<Move>& moves, int result, const std::string& startFen, const std::string& whiteName="", const std::string& blackName="");
};

enum class GameResult { NotStarted, InProgress, WhiteIsMated, BlackIsMated, Stalemate, Repetition, FiftyMoveRule, InsufficientMaterial, DrawByArbiter, WhiteTimeout, BlackTimeout, WhiteIllegalMove, BlackIllegalMove };

struct Arbiter {
    static bool IsDrawResult(GameResult result);
    static bool IsWinResult(GameResult result);
    static bool IsWhiteWinsResult(GameResult result);
    static bool IsBlackWinsResult(GameResult result);
    static GameResult GetGameState(Board& board);
    static bool InsufficentMaterial(Board& board);
};

class RepetitionTable {
public:
    RepetitionTable();
    void Init(Board& board);
    void Push(std::uint64_t hash, bool reset);
    void TryPop();
    bool Contains(std::uint64_t h) const;
private:
    std::array<std::uint64_t,256> hashes{};
    std::array<int,257> startIndices{};
    int count{};
};

class TranspositionTable {
public:
    static constexpr int LookupFailed = -1;
    static constexpr int Exact = 0;
    static constexpr int LowerBound = 1;
    static constexpr int UpperBound = 2;
    struct Entry { std::uint64_t key{}; int value{}; Move move{}; std::uint8_t depth{}; std::uint8_t nodeType{}; Entry()=default; Entry(std::uint64_t key,int value,std::uint8_t depth,std::uint8_t nodeType,Move move):key(key),value(value),move(move),depth(depth),nodeType(nodeType){} };
    std::vector<Entry> entries;
    std::uint64_t count{};
    bool enabled{true};
    Board* board{};
    explicit TranspositionTable(Board& board, int sizeMB);
    void Clear();
    std::uint64_t Index() const;
    Move TryGetStoredMove() const;
    int LookupEvaluation(int depth, int plyFromRoot, int alpha, int beta);
    void StoreEvaluation(int depth, int numPlySearched, int eval, int evalType, Move move);
    Entry GetEntry(std::uint64_t zobristKey) const;
private:
    int CorrectMateScoreForStorage(int score, int numPlySearched) const;
    int CorrectRetrievedMateScore(int score, int numPlySearched) const;
};

class MoveGenerator {
public:
    static constexpr int MaxMoves = 218;
    enum class PromotionMode { All, QueenOnly, QueenAndKnight };
    PromotionMode promotionsToGenerate{PromotionMode::All};
    std::uint64_t opponentAttackMap{};
    std::uint64_t opponentPawnAttackMap{};
    std::span<Move> GenerateMoves(Board& board, bool capturesOnly=false);
    int GenerateMoves(Board& board, std::span<Move>& moves, bool capturesOnly=false);
    bool InCheck() const;
private:
    bool isWhiteToMove{};
    int friendlyColour{};
    int opponentColour{};
    int friendlyKingSquare{};
    int friendlyIndex{};
    int enemyIndex{};
    bool inCheck{};
    bool inDoubleCheck{};
    std::uint64_t checkRayBitmask{};
    std::uint64_t pinRays{};
    std::uint64_t notPinRays{};
    std::uint64_t opponentAttackMapNoPawns{};
    std::uint64_t opponentSlidingAttackMap{};
    bool generateQuietMoves{};
    Board* board{};
    int currMoveIndex{};
    std::uint64_t enemyPieces{};
    std::uint64_t friendlyPieces{};
    std::uint64_t allPieces{};
    std::uint64_t emptySquares{};
    std::uint64_t emptyOrEnemySquares{};
    std::uint64_t moveTypeMask{};
    void Init();
    void GenerateKingMoves(std::span<Move> moves);
    void GenerateSlidingMoves(std::span<Move> moves);
    void GenerateKnightMoves(std::span<Move> moves);
    void GeneratePawnMoves(std::span<Move> moves);
    void GeneratePromotions(int startSquare, int targetSquare, std::span<Move> moves);
    bool IsPinned(int square) const;
    void GenSlidingAttackMap();
    void CalculateAttackData();
    bool InCheckAfterEnPassant(int startSquare, int targetSquare, int epCaptureSquare);
};

class MoveOrdering {
public:
    struct Killers { Move a{}, b{}; bool Match(const Move& m) const { return Move::SameMove(a,m) || Move::SameMove(b,m); } void Add(const Move& m){ if(!Move::SameMove(a,m)){ b=a; a=m; } } };
    static constexpr int maxKillerMovePly = 32;
    MoveOrdering(MoveGenerator& m, TranspositionTable& tt);
    void ClearHistory();
    void ClearKillers();
    void Clear();
    void OrderMoves(Move hashMove, Board& board, std::span<Move> moves, std::uint64_t oppAttacks, std::uint64_t oppPawnAttacks, bool inQSearch, int ply);
    std::array<Killers,maxKillerMovePly> killerMoves{};
    std::array<std::array<std::array<int,64>,64>,2> History{};
private:
    static constexpr int maxMoveCount = 218;
    static constexpr int hashMoveScore = 100 * 1000000;
    static constexpr int winningCaptureBias = 8 * 1000000;
    static constexpr int promoteBias = 6 * 1000000;
    static constexpr int killerBias = 4 * 1000000;
    static constexpr int losingCaptureBias = 2 * 1000000;
    static constexpr int regularBias = 0;
    std::array<int,maxMoveCount> moveScores{};
    TranspositionTable* transpositionTable{};
    Move invalidMove{};
    static int GetPieceValue(int pieceType);
};

class Searcher {
public:
    static constexpr int immediateMateScore = 100000;
    struct SearchDiagnostics { int numCompletedIterations{}; int numPositionsEvaluated{}; std::uint64_t numCutOffs{}; std::string moveVal; std::string move; int eval{}; bool moveIsFromPartialSearch{}; int NumQChecks{}; int numQMates{}; bool isBook{}; int maxExtentionReachedInSearch{}; };
    explicit Searcher(Board& board);
    std::function<void(Move)> OnSearchComplete;
    int CurrentDepth{};
    Move BestMoveSoFar() const;
    int BestEvalSoFar() const;
    void StartSearch();
    std::pair<Move,int> GetSearchResult() const;
    void EndSearch();
    static bool IsMateScore(int score);
    static int NumPlyToMateFromScore(int score);
    std::string AnnounceMate();
    void ClearForNewPosition();
    TranspositionTable& GetTranspositionTable();
    SearchDiagnostics searchDiagnostics;
    std::string debugInfo;
private:
    static constexpr int transpositionTableSizeMB = 64;
    static constexpr int maxExtentions = 16;
    static constexpr int positiveInfinity = 9999999;
    static constexpr int negativeInfinity = -positiveInfinity;
    bool isPlayingWhite{};
    Move bestMoveThisIteration{};
    int bestEvalThisIteration{};
    Move bestMove{};
    int bestEval{};
    bool hasSearchedAtLeastOneMove{};
    std::atomic<bool> searchCancelled{false};
    int currentIterationDepth{};
    std::chrono::steady_clock::time_point searchIterationTimer{};
    std::chrono::steady_clock::time_point searchTotalTimer{};
    TranspositionTable transpositionTable;
    RepetitionTable repetitionTable;
    MoveGenerator moveGenerator;
    MoveOrdering moveOrderer;
    Evaluation evaluation;
    Board& board;
    void RunIterativeDeepeningSearch();
    int Search(int plyRemaining, int plyFromRoot, int alpha, int beta, int numExtensions = 0, Move prevMove = Move{}, bool prevWasCapture = false);
    int QuiescenceSearch(int alpha, int beta);
};

} // namespace Chess::Core

namespace CodingAdventureBot {
using namespace Chess::Core;
class Bot {
public:
    static constexpr bool useOpeningBook = true;
    static constexpr int maxBookPly = 16;
    static constexpr bool useMaxThinkTime = false;
    static constexpr int maxThinkTimeMs = 2500;
    std::function<void(const std::string&)> OnMoveChosen;
    bool IsThinking() const;
    bool LatestMoveIsBookMove{};
    Bot();
    ~Bot();
    void NotifyNewGame();
    void SetPosition(const std::string& fen);
    void MakeMove(const std::string& moveString);
    int ChooseThinkTime(int timeRemainingWhiteMs, int timeRemainingBlackMs, int incrementWhiteMs, int incrementBlackMs);
    void ThinkTimed(int timeMs);
    void StopThinking();
    void Quit();
    std::string GetBoardDiagram() const;
private:
    Board board;
    Searcher searcher;
    OpeningBook book;
    std::thread worker;
    mutable std::mutex mtx;
    std::condition_variable cv;
    bool pendingSearch{};
    std::atomic<bool> isThinking{false};
    std::atomic<bool> isQuitting{false};
    std::atomic<int> currentSearchID{0};
    std::jthread timerThread;
    void StartSearch(int timeMs);
    void SearchThread();
    void EndSearch();
    void EndSearch(int searchID);
    void OnSearchComplete(Move move);
    bool TryGetOpeningBookMove(Move& bookMove);
};

class EngineUCI {
public:
    EngineUCI();
    void ReceiveCommand(const std::string& message);
    static std::string AppDataPath();
private:
    std::unique_ptr<Bot> player;
    void EnsurePlayer();
    static constexpr bool logToFile = false;
    static inline const std::array<std::string,3> positionLabels = {"position","fen","moves"};
    static inline const std::array<std::string,7> goLabels = {"go","movetime","wtime","btime","winc","binc","movestogo"};
    void OnMoveChosen(const std::string& move);
    void ProcessGoCommand(const std::string& message);
    void ProcessPositionCommand(const std::string& message);
    void Respond(const std::string& response);
    static int TryGetLabelledValueInt(const std::string& text, const std::string& label, const std::vector<std::string>& allLabels, int defaultValue = 0);
    static std::string TryGetLabelledValue(const std::string& text, const std::string& label, const std::vector<std::string>& allLabels, const std::string& defaultValue = "");
    void LogToFile(const std::string& text);
};
}
