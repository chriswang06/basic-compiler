#pragma once

#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    exit,
    if_,
    elif,
    else_,
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
    less,
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

inline std::optional<int> bin_prec(const TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::star:
        case TokenType::fslash:
            return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    std::optional<std::string> value{};
};


class Tokenizer {
public:
    explicit Tokenizer(std::string src)
        : m_src(std::move(src)) {
    }

    std::vector<Token> tokenize() {
        std::string buffer;
        std::vector<Token> tokens;
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buffer.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buffer.push_back(consume());
                }
                if (buffer == "exit") {
                    tokens.push_back({.type = TokenType::exit});
                    buffer.clear();
                } else if (buffer == "let") {
                    tokens.push_back({.type = TokenType::let});
                    buffer.clear();
                } else if (buffer == "elif") {
                    tokens.push_back({.type = TokenType::elif});
                    buffer.clear();
                } else if (buffer == "if") {
                    tokens.push_back({.type = TokenType::if_});
                    buffer.clear();
                } else if (buffer == "else") {
                    tokens.push_back({.type = TokenType::else_});
                    buffer.clear();
                } else {
                    tokens.push_back({.type = TokenType::ident, .value = buffer});
                    buffer.clear();
                }
            } else if (std::isdigit(peek().value())) {
                buffer.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buffer.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_lit, .value = buffer});
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
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::open_paren});
            } else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::closed_paren});
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
            } else if (peek().value() == '+') {
                consume();
                tokens.push_back({.type = TokenType::plus});
            } else if (peek().value() == '*') {
                consume();
                tokens.push_back({.type = TokenType::star});
            } else if (peek().value() == '-') {
                consume();
                tokens.push_back({.type = TokenType::minus});
            } else if (peek().value() == '/') {
                consume();
                tokens.push_back({.type = TokenType::fslash});
            } else if (peek().value() == '{') {
                consume();
                tokens.push_back({.type = TokenType::open_curly});
            } else if (peek().value() == '}') {
                consume();
                tokens.push_back({.type = TokenType::closed_curly});
            } else if (peek().value() == '>') {
                consume();
                tokens.push_back({.type = TokenType::greater});
            } else if (peek().value() == '<') {
                consume();
                tokens.push_back({.type = TokenType::less});
            } else if (std::isspace(peek().value())) {
                consume();
            } else {
                std::cerr << "Mess up" << std::endl;
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
