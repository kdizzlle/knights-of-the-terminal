#ifndef REPETITIONTABLE_H
#define REPETITIONTABLE_H

#include <vector>
#include <cstdint>

class RepetitionTable {
public:
    void init(uint64_t initialKey);
    void push(uint64_t key, bool reset);
    void pop();
    bool contains(uint64_t key) const;

private:
    struct Entry {
        uint64_t key;
        int previousResetIndex;
    };

    std::vector<Entry> history;
    int resetIndex = 0;
};

#endif
