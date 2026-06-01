
/// =====================================================================
/// BSharp - Simple Wide Character Width Utility (UTF-8 Decoder)
/// =====================================================================
/// Original Author : sulfur ethan
/// Adapted & Modified by : bibibird (2026/05/29)
///
/// Description:
///   A lightweight C++ implementation of wcwidth for the BSharp Interpreter.
///   Handles UTF-8 multi-byte decoding, boundary safety defenses, and
///   calculates visual column widths (combining/wide/emoji characters)
///   to ensure flawless terminal error diagnostics alignment.
/// =====================================================================

#ifndef BSHARP_SIMPLE_WCWIDTH_H
#define BSHARP_SIMPLE_WCWIDTH_H

#include <cstdint>
#include <string>
#include <vector>

using u32 = uint32_t;

struct glyph {
    u32 codepoint;
    int width;
};

inline bool is_combining(const u32 cp) {
    return
            (cp >= 0x0300 && cp <= 0x036F) ||
            (cp >= 0x1AB0 && cp <= 0x1AFF) ||
            (cp >= 0x1DC0 && cp <= 0x1DFF) ||
            (cp >= 0x20D0 && cp <= 0x20FF) ||
            (cp >= 0xFE20 && cp <= 0xFE2F);
}

inline bool is_wide(const u32 cp) {
    return
            // CJK Unified Ideographs
            (cp >= 0x4E00 && cp <= 0x9FFF) ||

            // CJK Extension A
            (cp >= 0x3400 && cp <= 0x4DBF) ||

            // Hangul Syllables
            (cp >= 0xAC00 && cp <= 0xD7AF) ||

            // Hiragana + Katakana
            (cp >= 0x3040 && cp <= 0x30FF) ||

            // Fullwidth forms
            (cp >= 0xFF01 && cp <= 0xFF60) ||
            (cp >= 0xFFE0 && cp <= 0xFFE6) ||

            // Common emoji blocks
            (cp >= 0x1F300 && cp <= 0x1FAFF);
}

inline int wcwidth_simple(const u32 cp) {
    // NUL
    if (cp == 0)
        return 0;

    // Control characters
    if (cp < 32 || (cp >= 0x7F && cp < 0xA0))
        return 0;

    // Combining marks
    if (is_combining(cp))
        return 0;

    // Wide characters
    if (is_wide(cp))
        return 2;

    // Default narrow width
    return 1;
}

inline bool is_continuation_byte(const unsigned char c) {
    return c >> 6 == 0b10;
}

inline std::vector<glyph> utf8_decode(const std::string &s) {
    std::vector<glyph> result;
    result.reserve(s.size());

    size_t i = 0;

    while (i < s.size()) {
        u32 cp = 0;

        //
        // ASCII
        //
        if (const auto c = static_cast<unsigned char>(s[i]); c < 0x80) {
            cp = c;
            i += 1;
        }

        //
        // 2-byte sequence
        //
        else if (c >> 5 == 0x6) {
            if (i + 1 >= s.size()) {
                cp = 0xFFFD;
                i += 1;
            } else {
                if (const auto b1 = static_cast<unsigned char>(s[i + 1]); !is_continuation_byte(b1)) {
                    cp = 0xFFFD;
                    i += 1;
                } else {
                    cp =
                            (c & 0x1F) << 6 |
                            b1 & 0x3F;

                    // Reject overlong encoding
                    if (cp < 0x80)
                        cp = 0xFFFD;

                    i += 2;
                }
            }
        }

        //
        // 3-byte sequence
        //
        else if (c >> 4 == 0xE) {
            if (i + 2 >= s.size()) {
                cp = 0xFFFD;
                i += 1;
            } else {
                const auto b1 = static_cast<unsigned char>(s[i + 1]);

                if (const auto b2 = static_cast<unsigned char>(s[i + 2]); !is_continuation_byte(b1) ||
                                                                          !is_continuation_byte(b2)) {
                    cp = 0xFFFD;
                    i += 1;
                } else {
                    cp =
                            (c & 0x0F) << 12 |
                            (b1 & 0x3F) << 6 |
                            b2 & 0x3F;

                    // Reject overlong encoding
                    // Reject UTF-16 surrogate range
                    if (cp < 0x800 ||
                        (cp >= 0xD800 && cp <= 0xDFFF))
                        cp = 0xFFFD;


                    i += 3;
                }
            }
        }

        //
        // 4-byte sequence
        //
        else if (c >> 3 == 0x1E) {
            if (i + 3 >= s.size()) {
                cp = 0xFFFD;
                i += 1;
            } else {
                const auto b1 = static_cast<unsigned char>(s[i + 1]);

                const auto b2 = static_cast<unsigned char>(s[i + 2]);

                if (const auto b3 = static_cast<unsigned char>(s[i + 3]); !is_continuation_byte(b1) ||
                                                                          !is_continuation_byte(b2) ||
                                                                          !is_continuation_byte(b3)) {
                    cp = 0xFFFD;
                    i += 1;
                } else {
                    cp =
                            (c & 0x07) << 18 |
                            (b1 & 0x3F) << 12 |
                            (b2 & 0x3F) << 6 |
                            b3 & 0x3F;

                    // Reject overlong encoding
                    // Reject values beyond Unicode range
                    if (cp < 0x10000 ||
                        cp > 0x10FFFF)
                        cp = 0xFFFD;


                    i += 4;
                }
            }
        }

        //
        // Invalid UTF-8 leading byte
        //
        else {
            cp = 0xFFFD;
            i += 1;
        }

        result.push_back({
            cp,
            wcwidth_simple(cp)
        });
    }

    return result;
}

#endif // BSHARP_SIMPLE_WCWIDTH_H
