#pragma once
#include <algorithm>
#include <cctype>
#include <sstream>
#include <span>
#include <string>
#include <vector>

namespace Chess::Core::Detail {
inline std::string trim_copy(const std::string& s) {
    auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return {};
    auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

inline std::string lower_copy(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

inline std::vector<std::string> split_ws(const std::string& s) {
    std::istringstream ss(s);
    std::vector<std::string> out;
    std::string part;
    while (ss >> part) out.push_back(part);
    return out;
}

template <typename T>
inline int index_of(const std::vector<T>& vec, const T& value) {
    auto it = std::find(vec.begin(), vec.end(), value);
    return it == vec.end() ? -1 : static_cast<int>(std::distance(vec.begin(), it));
}

inline void sort_moves_desc(std::span<Move> values, std::span<int> scores, int low, int high) {
    if (low >= high) return;
    int pivotScore = scores[high];
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
        if (scores[j] > pivotScore) {
            ++i;
            std::swap(values[static_cast<std::size_t>(i)], values[static_cast<std::size_t>(j)]);
            std::swap(scores[static_cast<std::size_t>(i)], scores[static_cast<std::size_t>(j)]);
        }
    }
    std::swap(values[static_cast<std::size_t>(i + 1)], values[static_cast<std::size_t>(high)]);
    std::swap(scores[static_cast<std::size_t>(i + 1)], scores[static_cast<std::size_t>(high)]);
    int pivot = i + 1;
    sort_moves_desc(values, scores, low, pivot - 1);
    sort_moves_desc(values, scores, pivot + 1, high);
}
} // namespace Chess::Core::Detail
