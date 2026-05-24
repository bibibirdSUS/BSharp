#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include "syntax/interpreter/context.h"
#include "syntax/interpreter/interpreter.h"
#include "syntax/lexer/lexer.h"
#include "syntax/parser/parser.h"
#include "syntax/bs_objs/builtin/builtins.h"

std::string token_type_to_string(const token_type t) {
    switch (t) {
        case token_type::NUMBER: return "NUMBER";
        case token_type::PLUS: return "PLUS";
        case token_type::SUBTRACT: return "SUBTRACT";
        case token_type::MULTIPLY: return "MULTIPLY";
        case token_type::DIVISION: return "DIVISION";
        case token_type::MODULO: return "MODULO";
        case token_type::POWER: return "POWER";
        case token_type::FACTORIAL: return "FACTORIAL";
        case token_type::LPARN: return "LPARN";
        case token_type::RPARN: return "RPARN";
        case token_type::IDENTIFIER: return "IDENTIFIER";
        case token_type::EQUALS: return "EQUALS";
        case token_type::EQ: return "EQ";
        case token_type::NEQ: return "NEQ";
        case token_type::LESS: return "LESS";
        case token_type::GREATER: return "GREATER";
        case token_type::LE: return "LE";
        case token_type::GE: return "GE";
        case token_type::AND: return "AND";
        case token_type::OR: return "OR";
        case token_type::XOR: return "XOR";
        case token_type::NOT: return "NOT";
        case token_type::TRUE: return "TRUE";
        case token_type::FALSE: return "FALSE";
        case token_type::NONE: return "NONE";
        case token_type::NULL_LIT: return "NONE_LIT";
        case token_type::IF: return "IF";
        case token_type::ELSE: return "ELSE";
        case token_type::DO: return "DO";
        case token_type::WHILE: return "WHILE";
        case token_type::UNTIL: return "UNTIL";
        case token_type::FOR: return "FOR";
        case token_type::INDENT: return "INDENT";
        case token_type::DEDENT: return "DEDENT";
        case token_type::TILDE: return "TILDE";
        case token_type::COLON: return "COLON";
        case token_type::PLUS_EQ: return "PLUS_EQ";
        case token_type::SUB_EQ: return "SUB_EQ";
        case token_type::MUL_EQ: return "MUL_EQ";
        case token_type::DIV_EQ: return "DIV_EQ";
        case token_type::MOD_EQ: return "MOD_EQ";
        case token_type::POW_EQ: return "POW_EQ";
        case token_type::EOF_: return "EOF";
        case token_type::NEWLINE: return "NEWLINE";
        default: return "UNKNOWN";
    }
}

void print_node(const node *n, const int indent = 0) {
    if (n == nullptr) return;
    for (int i = 0; i < indent; ++i) std::cout << "  ";

    if (const auto stmts = dynamic_cast<const statements_node *>(n)) {
        std::cout << "Statements: " << std::endl;
        for (const auto &stmt: stmts->statements) {
            print_node(stmt.get(), indent + 1);
        }
    } else if (const auto num = dynamic_cast<const number_node *>(n)) {
        std::cout << "Number: " << num->value << std::endl;
    } else if (const auto boolean = dynamic_cast<const boolean_node *>(n)) {
        std::cout << "Boolean: " << (boolean->value ? "true" : "false") << std::endl;
    } else if (const auto comp = dynamic_cast<const comparison_node *>(n)) {
        std::cout << "Comparison Chain: " << std::endl;
        for (size_t i = 0; i < comp->operands.size(); ++i) {
            print_node(comp->operands[i].get(), indent + 1);
            if (i < comp->operators.size()) {
                for (int j = 0; j < indent + 1; ++j) std::cout << "  ";
                std::cout << "Op: " << token_type_to_string(comp->operators[i]) << std::endl;
            }
        }
    } else if (const auto bin = dynamic_cast<const bin_op_node *>(n)) {
        std::cout << "Binary Op: " << token_type_to_string(bin->op) << std::endl;
        print_node(bin->left.get(), indent + 1);
        print_node(bin->right.get(), indent + 1);
    } else if (const auto uni = dynamic_cast<const unary_op_node *>(n)) {
        std::cout << "Unary Op: " << token_type_to_string(uni->op) << std::endl;
        print_node(uni->operand.get(), indent + 1);
    } else if (const auto fac = dynamic_cast<const factorial_node *>(n)) {
        std::cout << "Factorial" << std::endl;
        print_node(fac->operand.get(), indent + 1);
    } else if (const auto assign = dynamic_cast<const var_assign_node *>(n)) {
        std::cout << "Var Assign: " << assign->name << std::endl;
        print_node(assign->value.get(), indent + 1);
    } else if (const auto access = dynamic_cast<const var_access_node *>(n)) {
        std::cout << "Var Access: " << access->name << std::endl;
    } else if (const auto if_n = dynamic_cast<const if_node *>(n)) {
        std::cout << "If Statement: " << std::endl;
        for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
        std::cout << "Condition: " << std::endl;
        print_node(if_n->condition.get(), indent + 2);
        for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
        std::cout << "Then: " << std::endl;
        print_node(if_n->then_branch.get(), indent + 2);
        if (if_n->else_branch) {
            for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
            std::cout << "Else: " << std::endl;
            print_node(if_n->else_branch.get(), indent + 2);
        }
    } else if (const auto while_n = dynamic_cast<const while_node *>(n)) {
        std::cout << (while_n->is_until ? "Until Statement:" : "While Statement:") << std::endl;
        for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
        std::cout << "Condition:" << std::endl;
        print_node(while_n->condition.get(), indent + 2);
        for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
        std::cout << "Body:" << std::endl;
        print_node(while_n->body.get(), indent + 2);
    } else if (const auto for_n = dynamic_cast<const for_node *>(n)) {
        std::cout << "For Statement (" << for_n->name << "):" << std::endl;
        for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
        std::cout << "Start:" << std::endl;
        print_node(for_n->start.get(), indent + 2);
        for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
        std::cout << "End:" << std::endl;
        print_node(for_n->end.get(), indent + 2);
        if (for_n->step) {
            for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
            std::cout << "Step:" << std::endl;
            print_node(for_n->step.get(), indent + 2);
        }
        for (int i = 0; i < indent + 1; ++i) std::cout << "  ";
        std::cout << "Body:" << std::endl;
        print_node(for_n->body.get(), indent + 2);
    } else if (dynamic_cast<const null_node *>(n)) {
        std::cout << "None" << std::endl;
    } else
        std::cout << "Unknown Node" << std::endl;
}

bool run(const std::string &input, const std::string &file_name, const std::shared_ptr<context> &ctx,
         const bool debug) {
    if (input.empty()) return true;

    // lexer
    lexer lexer{input, std::string(file_name)};
    std::vector<token> tokens;
    try {
        tokens = lexer.tokenize();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    if (debug) {
        for (const auto &[literal, type, start, end]: tokens)
            std::cout << "[" << literal << " (" << token_type_to_string(type) << ") " << start.file_name << " " << start
                    .line + 1 <<
                    ":" << start.column + 1 << "~" << end.line + 1 <<
                    ":" << end.column + 1 << "]";
        std::cout << std::endl;
    }

    // parser
    parser parser{std::move(tokens), input};
    node_ptr ast;
    try {
        ast = parser.parse();
        if (ast && debug) {
            std::cout << "AST structure:" << std::endl;
            print_node(ast.get());
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    // interpreter
    interpreter interpreter{std::move(ast), input, ctx};
    try {
        interpreter.eval();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

int run_repl(const bool debug) {
    const auto global_context = std::make_shared<context>();
    register_all(global_context);

    for (;;) {
        std::string input;
        std::string line;
        std::cout << ">>> ";

        while (std::getline(std::cin, line)) {
            if (line.empty()) break;
            input += line + "\n";
            std::cout << "... ";
        }

        if (std::cin.eof()) break;
        if (input.empty()) continue;
        if (input == "exit\n") break;

        run(input, "<stdin>", global_context, debug);
    }
    return 0;
}

int run_file(const std::string &path, const bool debug) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();

    const auto global_context = std::make_shared<context>();
    register_all(global_context);
    return run(input, path, global_context, debug) ? 0 : 1;
}

int main(const int argc, char *argv[]) {
    bool debug = false;
    if (argc > 2 && argv[2] == std::string("debug"))
        debug = true;

    if (argc > 1)
        return run_file(argv[1], debug);
    return run_repl(debug);
}
