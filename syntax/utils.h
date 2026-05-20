//
// Created by bibib on 2026/3/1.
//

#ifndef BSHARP_UTILS_H
#define BSHARP_UTILS_H
#include <sstream>
#include <string>
#include <cmath>

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

#endif //BSHARP_UTILS_H
