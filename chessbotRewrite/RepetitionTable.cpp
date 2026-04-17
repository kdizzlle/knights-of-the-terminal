#include "RepetitionTable.h"

void RepetitionTable::init(uint64_t initialKey) {
    history.clear();
    resetIndex = 0;
    history.push_back({initialKey, 0});
}

void RepetitionTable::push(uint64_t key, bool reset) {
    int previous = resetIndex;
    if (reset) {
        resetIndex = static_cast<int>(history.size());
    }
    history.push_back({key, previous});
}

void RepetitionTable::pop() {
    if (history.size() <= 1) return;
    Entry e = history.back();
    history.pop_back();
    resetIndex = e.previousResetIndex;
}

bool RepetitionTable::contains(uint64_t key) const {
    for (int i = static_cast<int>(history.size()) - 2; i >= resetIndex; --i) {
        if (history[i].key == key) return true;
    }
    return false;
}
