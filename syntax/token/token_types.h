//
// Created by bibib on 2026/2/20.
//

#ifndef BSHARP_TOKEN_TYPES_H
#define BSHARP_TOKEN_TYPES_H

enum class token_type {
    NUMBER,
    PLUS,
    SUBTRACT,
    MULTIPLY,
    DIVISION,
    MODULO,
    POWER,
    FACTORIAL,
    LPARN,
    RPARN,
    NONE,
    EOF_,
    IDENTIFIER,
    EQUALS,
    EQ,
    GREATER,
    LESS,
    GE,
    LE,
    NEQ,
    TRUE,
    FALSE,
    AND,
    OR,
    NOT,
    XOR,
    NEWLINE,
    IF,
    ELSE,
    DO,
    INDENT,
    DEDENT,
    NULL_LIT,
    FOR,
    WHILE,
    UNTIL,
    TILDE,
    COLON,
    PLUS_EQ,
    SUB_EQ,
    MUL_EQ,
    DIV_EQ,
    MOD_EQ,
    POW_EQ,
    COMMA,
    FN,
    STRING,
    RETURN,
    BREAK,
    CONTINUE
};

#endif //BSHARP_TOKEN_TYPES_H