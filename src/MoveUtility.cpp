#include "ChessEngine.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace Chess::Core {
Move MoveUtility::GetMoveFromUCIName(const std::string& moveName, Board& board) {
    int startSquare = BoardHelper::SquareIndexFromName(moveName.substr(0, 2));
    int targetSquare = BoardHelper::SquareIndexFromName(moveName.substr(2, 2));
    int movedPieceType = Piece::PieceType(board.Square[startSquare]);
    Coord startCoord(startSquare);
    Coord targetCoord(targetSquare);
    int flag = Move::NoFlag;

    if (movedPieceType == Piece::Pawn) {
        if (moveName.size() > 4) {
            char last = static_cast<char>(std::tolower(static_cast<unsigned char>(moveName.back())));
            switch (last) {
                case 'q': flag = Move::PromoteToQueenFlag; break;
                case 'r': flag = Move::PromoteToRookFlag; break;
                case 'n': flag = Move::PromoteToKnightFlag; break;
                case 'b': flag = Move::PromoteToBishopFlag; break;
                default: flag = Move::NoFlag; break;
            }
        } else if (std::abs(targetCoord.rankIndex - startCoord.rankIndex) == 2) {
            flag = Move::PawnTwoUpFlag;
        }
    } else if (movedPieceType == Piece::King) {
        if (std::abs(startCoord.fileIndex - targetCoord.fileIndex) > 1) flag = Move::CastleFlag;
    }

    return Move(startSquare, targetSquare, flag);
}

std::string MoveUtility::GetMoveNameUCI(const Move& move) {
    std::string startSquareName = BoardHelper::SquareNameFromIndex(move.StartSquare());
    std::string endSquareName = BoardHelper::SquareNameFromIndex(move.TargetSquare());
    std::string moveName = startSquareName + endSquareName;
    if (move.IsPromotion()) {
        switch (move.MoveFlag()) {
            case Move::PromoteToRookFlag: moveName += 'r'; break;
            case Move::PromoteToKnightFlag: moveName += 'n'; break;
            case Move::PromoteToBishopFlag: moveName += 'b'; break;
            case Move::PromoteToQueenFlag: moveName += 'q'; break;
        }
    }
    return moveName;
}

std::string MoveUtility::GetMoveNameSAN(const Move& move, Board& board) {
    if (move.IsNull()) return "Null";
    int movePieceType = Piece::PieceType(board.Square[move.StartSquare()]);
    int capturedPieceType = Piece::PieceType(board.Square[move.TargetSquare()]);

    if (move.MoveFlag() == Move::CastleFlag) {
        int delta = move.TargetSquare() - move.StartSquare();
        if (delta == 2) return "O-O";
        if (delta == -2) return "O-O-O";
    }

    MoveGenerator moveGen;
    std::string moveNotation(1, static_cast<char>(std::toupper(static_cast<unsigned char>(Piece::GetSymbol(movePieceType)))));

    if (movePieceType != Piece::Pawn && movePieceType != Piece::King) {
        auto allMoves = moveGen.GenerateMoves(board);
        for (const Move& altMove : allMoves) {
            if (altMove.StartSquare() != move.StartSquare() && altMove.TargetSquare() == move.TargetSquare()) {
                if (Piece::PieceType(board.Square[altMove.StartSquare()]) == movePieceType) {
                    int fromFileIndex = BoardHelper::FileIndex(move.StartSquare());
                    int alternateFromFileIndex = BoardHelper::FileIndex(altMove.TargetSquare()); // faithful to source
                    int fromRankIndex = BoardHelper::RankIndex(move.StartSquare());
                    int alternateFromRankIndex = BoardHelper::RankIndex(altMove.StartSquare());
                    if (fromFileIndex != alternateFromFileIndex) {
                        moveNotation += BoardHelper::fileNames[fromFileIndex];
                        break;
                    } else if (fromRankIndex != alternateFromRankIndex) {
                        moveNotation += BoardHelper::rankNames[fromRankIndex];
                        break;
                    }
                }
            }
        }
    }

    if (capturedPieceType != 0) {
        if (movePieceType == Piece::Pawn) moveNotation += BoardHelper::fileNames[BoardHelper::FileIndex(move.StartSquare())];
        moveNotation += 'x';
    } else if (move.MoveFlag() == Move::EnPassantCaptureFlag) {
        moveNotation += BoardHelper::fileNames[BoardHelper::FileIndex(move.StartSquare())];
        moveNotation += 'x';
    }

    moveNotation += BoardHelper::fileNames[BoardHelper::FileIndex(move.TargetSquare())];
    moveNotation += BoardHelper::rankNames[BoardHelper::RankIndex(move.TargetSquare())];

    if (move.IsPromotion()) {
        int promotionPieceType = move.PromotionPieceType();
        moveNotation += "=";
        moveNotation += static_cast<char>(std::toupper(static_cast<unsigned char>(Piece::GetSymbol(promotionPieceType))));
    }

    board.MakeMove(move, true);
    auto legalResponses = moveGen.GenerateMoves(board);
    if (moveGen.InCheck()) {
        if (legalResponses.empty()) moveNotation += '#';
        else moveNotation += '+';
    }
    board.UnmakeMove(move, true);
    return moveNotation;
}

Move MoveUtility::GetMoveFromSAN(Board& board, std::string algebraicMove) {
    MoveGenerator moveGenerator;
    auto erase_all = [&](const std::string& what) {
        std::size_t pos = 0;
        while ((pos = algebraicMove.find(what, pos)) != std::string::npos) algebraicMove.erase(pos, what.size());
    };
    erase_all("+"); erase_all("#"); erase_all("x"); erase_all("-");
    auto allMoves = moveGenerator.GenerateMoves(board);

    Move move;
    for (const Move& moveToTest : allMoves) {
        move = moveToTest;
        int moveFromIndex = move.StartSquare();
        int moveToIndex = move.TargetSquare();
        int movePieceType = Piece::PieceType(board.Square[moveFromIndex]);
        Coord fromCoord(moveFromIndex);
        Coord toCoord(moveToIndex);
        if (algebraicMove == "OO") {
            if (movePieceType == Piece::King && moveToIndex - moveFromIndex == 2) return move;
        } else if (algebraicMove == "OOO") {
            if (movePieceType == Piece::King && moveToIndex - moveFromIndex == -2) return move;
        } else if (std::string(BoardHelper::fileNames).find(algebraicMove[0]) != std::string::npos) {
            if (movePieceType != Piece::Pawn) continue;
            if (static_cast<int>(std::string(BoardHelper::fileNames).find(algebraicMove[0])) == fromCoord.fileIndex) {
                if (algebraicMove.find('=') != std::string::npos) {
                    if (toCoord.rankIndex == 0 || toCoord.rankIndex == 7) {
                        if (algebraicMove.size() == 5) {
                            char targetFile = algebraicMove[1];
                            if (static_cast<int>(std::string(BoardHelper::fileNames).find(targetFile)) != toCoord.fileIndex) continue;
                        }
                        char promotionChar = algebraicMove[algebraicMove.size() - 1];
                        if (move.PromotionPieceType() != Piece::GetPieceTypeFromSymbol(promotionChar)) continue;
                        return move;
                    }
                } else {
                    char targetFile = algebraicMove[algebraicMove.size() - 2];
                    char targetRank = algebraicMove[algebraicMove.size() - 1];
                    if (static_cast<int>(std::string(BoardHelper::fileNames).find(targetFile)) == toCoord.fileIndex) {
                        if (std::to_string(toCoord.rankIndex + 1) == std::string(1, targetRank)) break;
                    }
                }
            }
        } else {
            char movePieceChar = algebraicMove[0];
            if (Piece::GetPieceTypeFromSymbol(movePieceChar) != movePieceType) continue;
            char targetFile = algebraicMove[algebraicMove.size() - 2];
            char targetRank = algebraicMove[algebraicMove.size() - 1];
            if (static_cast<int>(std::string(BoardHelper::fileNames).find(targetFile)) == toCoord.fileIndex) {
                if (std::to_string(toCoord.rankIndex + 1) == std::string(1, targetRank)) {
                    if (algebraicMove.size() == 4) {
                        char disambiguationChar = algebraicMove[1];
                        if (std::string(BoardHelper::fileNames).find(disambiguationChar) != std::string::npos) {
                            if (static_cast<int>(std::string(BoardHelper::fileNames).find(disambiguationChar)) != fromCoord.fileIndex) continue;
                        } else {
                            if (std::to_string(fromCoord.rankIndex + 1) != std::string(1, disambiguationChar)) continue;
                        }
                    }
                    break;
                }
            }
        }
    }
    return move;
}

std::string PGNCreator::CreatePGN(const std::vector<Move>& moves) {
    return CreatePGN(moves, static_cast<int>(GameResult::InProgress), FenUtility::StartPositionFEN);
}

std::string PGNCreator::CreatePGN(Board& board, int result, const std::string& whiteName, const std::string& blackName) {
    return CreatePGN(board.AllGameMoves, result, board.GameStartFEN(), whiteName, blackName);
}

std::string PGNCreator::CreatePGN(const std::vector<Move>& moves, int result, const std::string& startFen_, const std::string& whiteName, const std::string& blackName) {
    std::string startFen = startFen_;
    startFen.erase(std::remove(startFen.begin(), startFen.end(), '\n'), startFen.end());
    startFen.erase(std::remove(startFen.begin(), startFen.end(), '\r'), startFen.end());

    std::ostringstream pgn;
    Board board;
    board.LoadPosition(startFen);
    if (!whiteName.empty()) pgn << "[White \"" << whiteName << "\"]\n";
    if (!blackName.empty()) pgn << "[Black \"" << blackName << "\"]\n";
    if (startFen != FenUtility::StartPositionFEN) pgn << "[FEN \"" << startFen << "\"]\n";
    if (result != static_cast<int>(GameResult::NotStarted) && result != static_cast<int>(GameResult::InProgress)) {
        pgn << "[Result \"" << result << "\"]\n";
    }
    for (std::size_t plyCount = 0; plyCount < moves.size(); plyCount++) {
        std::string moveString = MoveUtility::GetMoveNameSAN(moves[plyCount], board);
        board.MakeMove(moves[plyCount]);
        if (plyCount % 2 == 0) pgn << (plyCount / 2 + 1) << ". ";
        pgn << moveString << ' ';
    }
    return pgn.str();
}
} // namespace Chess::Core
