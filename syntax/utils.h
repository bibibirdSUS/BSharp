//
// Created by bibibird on 2026/3/1.
//

#ifndef BSHARP_UTILS_H
#define BSHARP_UTILS_H
#include <sstream>
#include <string>
#include <cmath>
#include <cstddef>

inline std::string get_line(const std::string &input, const size_t line) {
    std::istringstream stream{input};
    std::string result;

    for (size_t i = 0; i <= line; ++i)
        if (!std::getline(stream, result))
            return "";

    if (!result.empty() && result.back() == '\r')
        result.pop_back();

    return result;
}

inline double factorial(const double n) {
    return std::tgamma(n + 1);
}

inline std::string argument_word(const size_t count) {
    return count == 1 ? "argument" : "arguments";
}

inline size_t hash_combine(const size_t seed, const size_t value) {
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

#endif //BSHARP_UTILS_H
