#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <vector>
#include "Board.h"

namespace MoveGenerator {
    void generateLegalMoves(Board& b, std::vector<Move>& out);
    void generateTacticalMoves(Board& b, std::vector<Move>& out);
}

#endif
