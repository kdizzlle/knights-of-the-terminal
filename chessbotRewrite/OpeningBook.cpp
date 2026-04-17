#include "OpeningBook.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
    std::vector<std::vector<std::string>> lines;
    bool loaded = false;
}

namespace OpeningBook {
    void init() {
        if (loaded) return;
        loaded = true;
        std::ifstream in("book.txt");
        if (!in) return;
        std::string line;
        while (std::getline(in, line)) {
            std::istringstream ss(line);
            std::vector<std::string> seq;
            std::string mv;
            while (ss >> mv) seq.push_back(mv);
            if (!seq.empty()) lines.push_back(seq);
        }
    }

    Move probe(const std::vector<std::string>& movesUci) {
        init();
        for (const auto& seq : lines) {
            if (seq.size() <= movesUci.size()) continue;
            bool match = true;
            for (size_t i = 0; i < movesUci.size(); ++i) {
                if (seq[i] != movesUci[i]) { match = false; break; }
            }
            if (match) return Move::fromUci(seq[movesUci.size()].c_str());
        }
        return Move();
    }
}
