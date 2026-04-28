#include "ChessEngine.hpp"
#include "BookResource.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

namespace CodingAdventureBot {
using namespace Chess::Core;

bool Bot::IsThinking() const { return isThinking.load(); }

Bot::Bot() : board(Board::CreateBoard()), searcher(board), book(kEmbeddedBook) {
    searcher.OnSearchComplete = [this](Move move) { OnSearchComplete(move); };
    worker = std::thread([this]() { SearchThread(); });
}

Bot::~Bot() {
    Quit();
    if (timerThread.joinable()) {
        timerThread.request_stop();
        timerThread.join();
    }
    cv.notify_all();
    if (worker.joinable()) worker.join();
}

void Bot::NotifyNewGame() { searcher.ClearForNewPosition(); }
void Bot::SetPosition(const std::string& fen) { board.LoadPosition(fen); }
void Bot::MakeMove(const std::string& moveString) { Move move = MoveUtility::GetMoveFromUCIName(moveString, board); board.MakeMove(move); }

int Bot::ChooseThinkTime(int timeRemainingWhiteMs, int timeRemainingBlackMs, int incrementWhiteMs, int incrementBlackMs) {
    int myTimeRemainingMs = board.IsWhiteToMove ? timeRemainingWhiteMs : timeRemainingBlackMs;
    int myIncrementMs = board.IsWhiteToMove ? incrementWhiteMs : incrementBlackMs;
    double thinkTimeMs = myTimeRemainingMs / 40.0;
    if (useMaxThinkTime) thinkTimeMs = std::min<double>(maxThinkTimeMs, thinkTimeMs);
    if (myTimeRemainingMs > myIncrementMs * 2) thinkTimeMs += myIncrementMs * 0.8;
    double minThinkTime = std::min<double>(50, myTimeRemainingMs * 0.25);
    return static_cast<int>(std::ceil(std::max(minThinkTime, thinkTimeMs)));
}

void Bot::ThinkTimed(int timeMs) {
    LatestMoveIsBookMove = false;
    isThinking = true;
    timeMs = std::max(0, timeMs);
    if (timerThread.joinable()) {
        timerThread.request_stop();
        timerThread.join();
    }

    Move bookMove;
    if (TryGetOpeningBookMove(bookMove)) {
        LatestMoveIsBookMove = true;
        OnSearchComplete(bookMove);
    } else {
        StartSearch(timeMs);
    }
}

void Bot::StartSearch(int timeMs) {
    int searchID = ++currentSearchID;
    {
        std::lock_guard lock(mtx);
        pendingSearch = true;
    }
    cv.notify_one();
    timerThread = std::jthread([this, timeMs, searchID](std::stop_token st) {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeMs);
        while (!st.stop_requested() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (!st.stop_requested() && currentSearchID.load() == searchID && IsThinking()) {
            searcher.EndSearch();
        }
    });
}

void Bot::SearchThread() {
    std::unique_lock lock(mtx);
    while (true) {
        cv.wait(lock, [this]() { return pendingSearch || isQuitting.load(); });
        if (isQuitting.load()) break;
        pendingSearch = false;
        lock.unlock();
        searcher.StartSearch();
        lock.lock();
    }
}

void Bot::StopThinking() { EndSearch(); }

void Bot::Quit() {
    isQuitting = true;
    EndSearch();
    cv.notify_all();
}

std::string Bot::GetBoardDiagram() const { return board.ToString(); }

void Bot::EndSearch() {
    if (timerThread.joinable()) {
        timerThread.request_stop();
        if (timerThread.get_id() != std::this_thread::get_id()) {
            timerThread.join();
        }
    }
    if (IsThinking()) searcher.EndSearch();
}

void Bot::EndSearch(int searchID) {
    if (currentSearchID.load() == searchID) EndSearch();
}

void Bot::OnSearchComplete(Move move) {
    isThinking = false;
    std::string moveName = MoveUtility::GetMoveNameUCI(move);
    moveName.erase(std::remove(moveName.begin(), moveName.end(), '='), moveName.end());
    if (OnMoveChosen) OnMoveChosen(moveName);
}

bool Bot::TryGetOpeningBookMove(Move& bookMove) {
    if (useOpeningBook && board.PlyCount <= maxBookPly) {
        std::string moveString;
        if (book.TryGetBookMove(board, moveString)) {
            bookMove = MoveUtility::GetMoveFromUCIName(moveString, board);
            return true;
        }
    }
    bookMove = Move::NullMove();
    return false;
}
} // namespace CodingAdventureBot
