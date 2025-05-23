#pragma once

#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    exit,
    if_,
    elif,
    else_,
    while_,
    int_lit,
    semi,
    open_paren,
    closed_paren,
    ident,
    let,
    eq,
    plus,
    star,
    minus,
    fslash,
    open_curly,
    closed_curly,
    greater,
    greaterequal,
    less,
    lessequal,
    equiv,
    notequiv,
    compound_add,
    compound_sub,
    compound_mul,
    compound_div,
    unary_plus,
    unary_minus,

};

inline bool is_bin_op(const TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::star:
        case TokenType::minus:
        case TokenType::fslash:
            return true;
        default:
            return false;
    }
}

inline std::string to_string(const TokenType type) {
    switch (type) {
        case TokenType::exit:
            return "'exit'";
        case TokenType::if_:
            return "'if'";
        case TokenType::elif:
            return "'elif'";
        case TokenType::else_:
            return "'else'";
        case TokenType::int_lit:
            return "'int lit'";
        case TokenType::semi:
            return "';'";
        case TokenType::open_paren:
            return "'('";
        case TokenType::closed_paren:
            return "')'";
        case TokenType::ident:
            return "'identifier'";
        case TokenType::let:
            return "'let'";
        case TokenType::eq:
            return "'='";
        case TokenType::plus:
            return "'+'";
        case TokenType::star:
            return "'*'";
        case TokenType::minus:
            return "'-'";
        case TokenType::fslash:
            return "'/'";
        case TokenType::greater:
            return "'>'";
        case TokenType::less:
            return "'<'";
        case TokenType::open_curly:
            return "'{'";
        case TokenType::closed_curly:
            return "'}'";
        case TokenType::greaterequal:
            return "'>='";
        case TokenType::lessequal:
            return "'<='";
        case TokenType::equiv:
            return "'=='";
        case TokenType::notequiv:
            return "'!='";
        case TokenType::while_:
            return "'while'";
        case TokenType::unary_minus:
            return"'--'";
        case TokenType::unary_plus:
            return "'++'";
        case TokenType::compound_add:
            return "'+'";
        case TokenType::compound_sub:
            return "'-'";
        case TokenType::compound_mul:
            return "'*'";
        case TokenType::compound_div:
            return "'/'";
        default:
            return"";
    }
    assert(false);
}

inline std::optional<int> bin_prec(const TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
            return 1;
        case TokenType::star:
        case TokenType::fslash:
            return 2;
        case TokenType::greater:
        case TokenType::greaterequal:
        case TokenType::less:
        case TokenType::lessequal:
        case TokenType::equiv:
        case TokenType::notequiv:
            return 0;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    int line;
    std::optional<std::string> value{};
};


class Tokenizer {
public:
    explicit Tokenizer(std::string src)
        : m_src(std::move(src)) {
    }

    std::vector<Token> tokenize() {
        int line_count = 1;
        std::string buffer;
        std::vector<Token> tokens;
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buffer.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buffer.push_back(consume());
                }
                if (buffer == "exit") {
                    tokens.push_back({TokenType::exit, line_count});
                    buffer.clear();
                } else if (buffer == "let") {
                    tokens.push_back({TokenType::let, line_count});
                    buffer.clear();
                } else if (buffer == "elif") {
                    tokens.push_back({TokenType::elif, line_count});
                    buffer.clear();
                } else if (buffer == "if") {
                    tokens.push_back({TokenType::if_, line_count});
                    buffer.clear();
                } else if (buffer == "else") {
                    tokens.push_back({TokenType::else_, line_count});
                    buffer.clear();
                } else if (buffer == "while") {
                    tokens.push_back({TokenType::while_, line_count});
                    buffer.clear();
                } else {
                    tokens.push_back({TokenType::ident, line_count, buffer});
                    buffer.clear();
                }
            } else if (std::isdigit(peek().value())) {
                buffer.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buffer.push_back(consume());
                }
                tokens.push_back({TokenType::int_lit, line_count, buffer});
                buffer.clear();
            } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
            } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
                consume();
                consume();
                while (peek().has_value()) {
                    if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/') {
                        break;
                    }
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
            }
            else if (peek().value() == '=' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::equiv, line_count});
            }
            else if (peek().value() == '!' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::notequiv, line_count});
            }
            else if (peek().value() == '>' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::greaterequal, line_count});
            }
            else if (peek().value() == '<' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::lessequal, line_count});
            }
            else if (peek().value() == '+' && peek(1).has_value() && peek(1).value() == '+') {
                consume();
                consume();
                tokens.push_back({TokenType::unary_plus, line_count});
            }
            else if (peek().value() == '-' && peek(1).has_value() && peek(1).value() == '-') {
                consume();
                consume();
                tokens.push_back({TokenType::unary_minus, line_count});
            }
            else if (peek().value() == '+' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::compound_add, line_count});
            }
            else if (peek().value() == '-' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::compound_sub, line_count});
            }
            else if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::compound_mul, line_count});
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({TokenType::compound_div, line_count});
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({TokenType::open_paren, line_count});
            } else if (peek().value() == '=') {
                consume();
                tokens.push_back({TokenType::eq, line_count});
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({TokenType::closed_paren, line_count});
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({TokenType::semi, line_count});
            } else if (peek().value() == '+') {
                consume();
                tokens.push_back({TokenType::plus, line_count});
            } else if (peek().value() == '*') {
                consume();
                tokens.push_back({TokenType::star, line_count});
            } else if (peek().value() == '-') {
                consume();
                tokens.push_back({TokenType::minus, line_count});
            } else if (peek().value() == '/') {
                consume();
                tokens.push_back({TokenType::fslash, line_count});
            } else if (peek().value() == '{') {
                consume();
                tokens.push_back({TokenType::open_curly, line_count});
            } else if (peek().value() == '}') {
                consume();
                tokens.push_back({TokenType::closed_curly, line_count});
            } else if (peek().value() == '>') {
                consume();
                tokens.push_back({TokenType::greater, line_count});
            } else if (peek().value() == '<') {
                consume();
                tokens.push_back({TokenType::less, line_count});
            } else if (peek().value() == '\n') {
                consume();
                line_count++;
            }
            else if (std::isspace(peek().value())) {
                consume();
            } else {
                std::cerr << "Invalid token" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return tokens;
    }

private:
    [[nodiscard]] std::optional<char> peek(const int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        return m_src.at(m_index + offset);
    }

    char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src;
    int m_index{};
};
