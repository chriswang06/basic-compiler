#pragma once

#include <string>
#include <vector>
#include <optional>

enum class TokenType {
    _exit,
    int_lit,
    semi,
    open_paren,
    closed_paren,
    ident,
    let,
    eq,
    plus,
    star,
    sub,
    slash,
};

bool is_bin_op(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::star:
        case TokenType::sub:
        case TokenType::slash:
            return true;
        default:
            return false;
    }
}

std::optional<int> bin_prec(TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::sub:
            return 0;
        case TokenType::star:
        case TokenType::slash:
            return 1;
        default:
            return {};
    }
}
struct Token {
    TokenType type;
    std::optional<std::string> value {};
};


class Tokenizer {
public:
    inline explicit Tokenizer(std::string src)
        : m_src(std::move(src))
    {

    }
    inline std::vector<Token> tokenize() {
        std::string buffer;
        std::vector<Token> tokens;
        while (peek().has_value()) {
            char c = peek().value();
            if (std::isalpha(peek().value())){
                buffer.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buffer.push_back(consume());
                }
                if (buffer=="exit") {
                    tokens.push_back({.type = TokenType::_exit});
                    buffer.clear();
                }
                else if (buffer=="let") {
                    tokens.push_back({.type = TokenType::let});
                    buffer.clear();
                }
                else {
                    tokens.push_back({.type = TokenType::ident, .value= buffer});
                    buffer.clear();
                }

            }
            else if (std::isdigit(peek().value())) {
                buffer.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buffer.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_lit, .value = buffer});
                buffer.clear();
            }
            else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::open_paren});
            }
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
            }
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::closed_paren});
            }
            else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({.type = TokenType::plus});
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({.type = TokenType::star});
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({.type = TokenType::sub});
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({.type = TokenType::slash});
            }
            else if (std::isspace(peek().value())) {
                consume();
            }

            else {
                std::cerr << "messup" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return tokens;
    }
private:
    [[nodiscard]] inline std::optional<char> peek(const int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index + offset);
        }
    }

    inline char consume() {
        return m_src.at(m_index++);

    }
    const std::string m_src;
    int m_index{};
};