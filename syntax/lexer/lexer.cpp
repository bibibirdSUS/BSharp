//
// Created by bibib on 2026/2/20.
//

#include "lexer.h"

#include "../bs_objs/exception/bs_illegal_character_exception.h"
#include "../position/position.h"
#include "../utils.h"
#include <sstream>

#include "../bs_objs/exception/bs_invalid_syntax_exception.h"

std::string_view operators = "+-*/%^!()=<>:~,";

lexer::lexer(std::string input, std::string &&file_name) : input_(std::move(input)), file_name_(std::move(file_name)),
                                                           index_(0), position_(file_name_, 0, 0),
                                                           is_line_start_(true) {
    indent_stack_.push(0);
    current_char_ = !input_.empty() ? input_[index_] : 0;
}

token_type get_operator_type(const std::string &s) {
    if (s == "+") return token_type::PLUS;
    if (s == "-") return token_type::SUBTRACT;
    if (s == "*") return token_type::MULTIPLY;
    if (s == "/") return token_type::DIVISION;
    if (s == "%") return token_type::MODULO;
    if (s == "^") return token_type::POWER;
    if (s == "!") return token_type::FACTORIAL;
    if (s == "(") return token_type::LPARN;
    if (s == ")") return token_type::RPARN;
    if (s == "=") return token_type::EQUALS;
    if (s == ",") return token_type::COMMA;
    if (s == "==") return token_type::EQ;
    if (s == ">") return token_type::GREATER;
    if (s == "<") return token_type::LESS;
    if (s == ">=") return token_type::GE;
    if (s == "<=") return token_type::LE;
    if (s == "!=") return token_type::NEQ;
    if (s == "~") return token_type::TILDE;
    if (s == ":") return token_type::COLON;
    if (s == "+=") return token_type::PLUS_EQ;
    if (s == "-=") return token_type::SUB_EQ;
    if (s == "*=") return token_type::MUL_EQ;
    if (s == "/=") return token_type::DIV_EQ;
    if (s == "%=") return token_type::MOD_EQ;
    if (s == "^=") return token_type::POW_EQ;

    return token_type::NONE;
}

token_type get_identifier_type(const std::string &identifier) {
    if (identifier == "true") return token_type::TRUE;
    if (identifier == "false") return token_type::FALSE;
    if (identifier == "and") return token_type::AND;
    if (identifier == "or") return token_type::OR;
    if (identifier == "xor") return token_type::XOR;
    if (identifier == "not") return token_type::NOT;
    if (identifier == "if") return token_type::IF;
    if (identifier == "else") return token_type::ELSE;
    if (identifier == "do") return token_type::DO;
    if (identifier == "null") return token_type::NULL_LIT;
    if (identifier == "for") return token_type::FOR;
    if (identifier == "until") return token_type::UNTIL;
    if (identifier == "while") return token_type::WHILE;
    if (identifier == "fn") return token_type::FN;
    if (identifier == "break") return token_type::BREAK;
    if (identifier == "continue") return token_type::CONTINUE;
    if (identifier == "return") return token_type::RETURN;

    return token_type::IDENTIFIER;
}

std::vector<token> lexer::tokenize() {
    std::vector<token> tokens;

    while (current_char_ != 0) {
        if (is_line_start_) {
            int indent = 0;
            while (current_char_ == ' ' || current_char_ == '\t') {
                indent += current_char_ == ' ' ? 1 : 4;
                advance();
            }
            if (current_char_ == '\n' || current_char_ == '\r' || current_char_ == 0 || current_char_ == '#') {
                if (current_char_ == '#') {
                    while (current_char_ != 0 && current_char_ != '\n') advance();
                }
                if (current_char_ == '\r') advance();
                if (current_char_ == '\n') advance();
                continue;
            }

            if (indent > indent_stack_.top()) {
                indent_stack_.push(indent);
                tokens.emplace_back("INDENT", token_type::INDENT, position_, position_);
            } else if (indent < indent_stack_.top()) {
                while (!indent_stack_.empty() && indent < indent_stack_.top()) {
                    indent_stack_.pop();
                    tokens.emplace_back("DEDENT", token_type::DEDENT, position_, position_);
                }
            }

            is_line_start_ = false;
        }
        if (current_char_ == ' ' || current_char_ == '\r' || current_char_ == '\t') advance();
        else if (current_char_ == '\n') {
            const position start_pos = position_;
            advance();
            tokens.emplace_back("\n", token_type::NEWLINE, start_pos, position_);
            is_line_start_ = true;
        } else if (current_char_ == '"') {
            const position start_pos = position_;
            advance();
            const std::string str = make_string();
            if (current_char_ == 0)
                throw bs_invalid_syntax_exception{"Unterminated string literal", input_, position_, position_};
            if (current_char_ == '\r' || current_char_ == '\n')
                throw bs_invalid_syntax_exception{"Unterminated string literal", input_, position_, position_};
            advance();
            tokens.emplace_back(str, token_type::STRING, start_pos, position_);
        } else if (current_char_ == '_' || isalpha(static_cast<unsigned char>(current_char_))) {
            const position start_pos = position_;
            const std::string identifier = make_identifier();
            tokens.emplace_back(identifier, get_identifier_type(identifier), start_pos,
                                position_);
        } else if (operators.find(current_char_) != std::string_view::npos) {
            const position start_pos = position_;
            const std::string op = make_operator();
            tokens.emplace_back(op, get_operator_type(op),
                                start_pos, position_);
        } else if (isdigit(static_cast<unsigned char>(current_char_))) {
            const position start_pos = position_;
            std::string val = make_number();
            tokens.emplace_back(val, token_type::NUMBER, start_pos,
                                position_);
        } else if (current_char_ == '#')
            while (current_char_ != 0 && current_char_ != '\n') advance();
        else {
            const position start_pos = position_;
            const char c = current_char_;
            advance();
            throw bs_illegal_character_exception{c, input_, start_pos, position_};
        }
    }

    while (indent_stack_.size() > 1) {
        indent_stack_.pop();
        tokens.emplace_back("DEDENT", token_type::DEDENT, position_, position_);
    }

    const position start_pos = position_;
    advance();
    tokens.emplace_back(" ", token_type::EOF_, start_pos, position_);
    return tokens;
}

void lexer::advance() {
    if (current_char_ == 0) return;
    position_.advance(current_char_);

    index_++;
    current_char_ = index_ >= input_.length() ? 0 : input_[index_];
}

std::string lexer::make_number() {
    std::stringstream number;
    bool is_float = false;
    while (current_char_ != 0 && (std::isdigit(current_char_) || current_char_ == '.')) {
        if (is_float && current_char_ == '.') break;
        if (current_char_ == '.') is_float = true;
        number << current_char_;
        advance();
    }
    return number.str();
}

std::string lexer::make_identifier() {
    std::stringstream identifier;
    while (current_char_ != 0 && (isalpha(current_char_) || isdigit(current_char_) || current_char_ == '_')) {
        identifier << current_char_;
        advance();
    }
    return identifier.str();
}

std::string lexer::make_operator() {
    std::string op;
    const char first = current_char_;
    op += current_char_;
    advance();
    if (current_char_ != 0 &&
        (
            (first == '=' && current_char_ == '=') ||
            (first == '>' && current_char_ == '=') ||
            (first == '<' && current_char_ == '=') ||
            (first == '!' && current_char_ == '=') ||
            (first == '+' && current_char_ == '=') ||
            (first == '-' && current_char_ == '=') ||
            (first == '*' && current_char_ == '=') ||
            (first == '/' && current_char_ == '=') ||
            (first == '%' && current_char_ == '=') ||
            (first == '^' && current_char_ == '=')
        )
    ) {
        op += current_char_;
        advance();
    }

    return op;
}

std::string lexer::make_string() {
    std::stringstream str;

    while (current_char_ != 0 && current_char_ != '"') {
        if (current_char_ == '\\') {
            advance();

            if (current_char_ == 0) break;

            switch (current_char_) {
                case 'n': str << '\n';
                    break;
                case 't': str << '\t';
                    break;
                case 'r': str << '\r';
                    break;
                case '\"': str << '\"';
                    break;
                case '\\': str << '\\';
                    break;
                case 'b': str << '\b';
                    break;
                case '0': str << '\0';
                    break;
                default:
                    str << '\\';
                    str << current_char_;
                    break;
            }
        } else if (current_char_ == '\n' || current_char_ == '\r') {
            return "";
        } else
            str << current_char_;
        advance();
    }
    return str.str();
}
