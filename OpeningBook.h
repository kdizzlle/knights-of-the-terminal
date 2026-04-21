#ifndef OPENINGBOOK_H
#define OPENINGBOOK_H

#include <string>
#include <vector>
#include "Move.h"

namespace OpeningBook {
    void init();
    Move probe(const std::vector<std::string>& movesUci);
}

#endif
