#include "ChessEngine.hpp"
#include "InternalUtils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace Chess::Core {
Board::PositionInfo FenUtility::PositionFromFen(const std::string& fen) {
    Board::PositionInfo info;
    info.fen = fen;
    info.squares.fill(Piece::None);

    auto sections = Detail::split_ws(fen);
    if (sections.empty()) return info;

    int file = 0;
    int rank = 7;
    for (char symbol : sections[0]) {
        if (symbol == '/') {
            file = 0;
            rank--;
        } else if (std::isdigit(static_cast<unsigned char>(symbol))) {
            file += symbol - '0';
        } else {
            int pieceColour = std::isupper(static_cast<unsigned char>(symbol)) ? Piece::White : Piece::Black;
            int pieceType = Piece::None;
            switch (static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)))) {
                case 'k': pieceType = Piece::King; break;
                case 'p': pieceType = Piece::Pawn; break;
                case 'n': pieceType = Piece::Knight; break;
                case 'b': pieceType = Piece::Bishop; break;
                case 'r': pieceType = Piece::Rook; break;
                case 'q': pieceType = Piece::Queen; break;
            }
            info.squares[rank * 8 + file] = pieceType | pieceColour;
            file++;
        }
    }

    if (sections.size() > 1) info.whiteToMove = sections[1] == "w";
    if (sections.size() > 2) {
        const std::string& castlingRights = sections[2];
        info.whiteCastleKingside = castlingRights.find('K') != std::string::npos;
        info.whiteCastleQueenside = castlingRights.find('Q') != std::string::npos;
        info.blackCastleKingside = castlingRights.find('k') != std::string::npos;
        info.blackCastleQueenside = castlingRights.find('q') != std::string::npos;
    }
    info.epFile = 0;
    info.fiftyMovePlyCount = 0;
    info.moveCount = 0;
    if (sections.size() > 3) {
        // Arena-compat mode: ignore any en-passant square encoded in the FEN.
        info.epFile = 0;
    }
    if (sections.size() > 4) {
        try { info.fiftyMovePlyCount = std::stoi(sections[4]); } catch (...) {}
    }
    if (sections.size() > 5) {
        try { info.moveCount = std::stoi(sections[5]); } catch (...) {}
    }
    return info;
}

std::string FenUtility::CurrentFen(const Board& board, bool alwaysIncludeEPSquare) {
    std::string fen;
    for (int rank = 7; rank >= 0; rank--) {
        int numEmptyFiles = 0;
        for (int file = 0; file < 8; file++) {
            int i = rank * 8 + file;
            int piece = board.Square[i];
            if (piece != 0) {
                if (numEmptyFiles != 0) {
                    fen += std::to_string(numEmptyFiles);
                    numEmptyFiles = 0;
                }
                bool isBlack = Piece::IsColour(piece, Piece::Black);
                int pieceType = Piece::PieceType(piece);
                char pieceChar = ' ';
                switch (pieceType) {
                    case Piece::Rook: pieceChar = 'R'; break;
                    case Piece::Knight: pieceChar = 'N'; break;
                    case Piece::Bishop: pieceChar = 'B'; break;
                    case Piece::Queen: pieceChar = 'Q'; break;
                    case Piece::King: pieceChar = 'K'; break;
                    case Piece::Pawn: pieceChar = 'P'; break;
                }
                fen += isBlack ? static_cast<char>(std::tolower(static_cast<unsigned char>(pieceChar))) : pieceChar;
            } else {
                numEmptyFiles++;
            }
        }
        if (numEmptyFiles != 0) fen += std::to_string(numEmptyFiles);
        if (rank != 0) fen += '/';
    }

    fen += ' ';
    fen += board.IsWhiteToMove ? 'w' : 'b';

    bool whiteKingside = (board.CurrentGameState.castlingRights & 1) == 1;
    bool whiteQueenside = ((board.CurrentGameState.castlingRights >> 1) & 1) == 1;
    bool blackKingside = ((board.CurrentGameState.castlingRights >> 2) & 1) == 1;
    bool blackQueenside = ((board.CurrentGameState.castlingRights >> 3) & 1) == 1;
    fen += ' ';
    fen += whiteKingside ? "K" : "";
    fen += whiteQueenside ? "Q" : "";
    fen += blackKingside ? "k" : "";
    fen += blackQueenside ? "q" : "";
    fen += (board.CurrentGameState.castlingRights == 0) ? "-" : "";

    fen += ' ';
    // Arena-compat mode: en passant is not part of the rule set, so always emit '-'.
    (void)alwaysIncludeEPSquare;
    fen += '-';

    fen += ' ';
    fen += std::to_string(board.CurrentGameState.fiftyMoveCounter);
    fen += ' ';
    fen += std::to_string((board.PlyCount / 2) + 1);
    return fen;
}

bool FenUtility::EnPassantCanBeCaptured(int epFileIndex, int epRankIndex, const Board& boardConst) {
    Board& board = const_cast<Board&>(boardConst);
    Coord captureFromA(epFileIndex - 1, epRankIndex + (board.IsWhiteToMove ? -1 : 1));
    Coord captureFromB(epFileIndex + 1, epRankIndex + (board.IsWhiteToMove ? -1 : 1));
    int epCaptureSquare = Coord(epFileIndex, epRankIndex).SquareIndex();
    int friendlyPawn = Piece::MakePiece(Piece::Pawn, board.MoveColour());

    auto canCapture = [&](const Coord& from) {
        if (!from.IsValidSquare()) return false;
        bool isPawnOnSquare = board.Square[from.SquareIndex()] == friendlyPawn;
        if (isPawnOnSquare) {
            Move move(from.SquareIndex(), epCaptureSquare, Move::EnPassantCaptureFlag);
            board.MakeMove(move);
            board.MakeNullMove();
            bool wasLegalMove = !board.CalculateInCheckState();
            board.UnmakeNullMove();
            board.UnmakeMove(move);
            return wasLegalMove;
        }
        return false;
    };

    return canCapture(captureFromA) || canCapture(captureFromB);
}

std::string FenUtility::FlipFen(const std::string& fen) {
    std::string flippedFen;
    auto sections = Detail::split_ws(fen);
    if (sections.size() < 6) return fen;

    std::vector<std::string> fenRanks;
    std::stringstream ss(sections[0]);
    std::string part;
    while (std::getline(ss, part, '/')) fenRanks.push_back(part);

    auto invertCase = [](char c) {
        if (std::islower(static_cast<unsigned char>(c))) return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    };

    for (int i = static_cast<int>(fenRanks.size()) - 1; i >= 0; i--) {
        for (char c : fenRanks[static_cast<std::size_t>(i)]) flippedFen += invertCase(c);
        if (i != 0) flippedFen += '/';
    }

    flippedFen += ' ';
    flippedFen += sections[1][0] == 'w' ? 'b' : 'w';

    std::string castlingRights = sections[2];
    std::string flippedRights;
    for (char c : std::string("kqKQ")) {
        if (castlingRights.find(c) != std::string::npos) flippedRights += invertCase(c);
    }
    flippedFen += ' ' + (flippedRights.empty() ? std::string("-") : flippedRights);

    std::string ep = sections[3];
    std::string flippedEp = ep.empty() ? std::string("-") : std::string(1, ep[0]);
    if (ep.size() > 1) flippedEp += (ep[1] == '6' ? '3' : '6');
    flippedFen += ' ' + flippedEp;
    flippedFen += ' ' + sections[4] + ' ' + sections[5];
    return flippedFen;
}
} // namespace Chess::Core
