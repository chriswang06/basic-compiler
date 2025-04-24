#pragma once
#include <variant>

#include "./tokenization.hpp"
#include "./arena.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMult {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    // std::variant<NodeBinExprAdd *, NodeBinExprMult *> var;
    NodeBinExprAdd* add;
};


struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *> var;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};


struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    NodeExpr *expr;
    Token ident;
};


struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *> var;
};

struct NodeProgram {
    std::vector<NodeStmt *> stmts;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)),
          m_allocator(1024 * 1024 * 4) // 4 MB
    {
    }

    std::optional<NodeTerm *> parse_term() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = consume();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        } else if (peek().has_value() && peek().value().type == TokenType::ident) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = consume();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        } else {
            return {};
        }
    }

    std::optional<NodeExpr *> parse_expr() {
        if (auto term = parse_term()) {
            if (peek().has_value() && peek().value().type == TokenType::plus) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                auto lhs_expr = m_allocator.alloc<NodeExpr>();
                lhs_expr->var = term.value();
                bin_expr_add->lhs = lhs_expr;
                consume();
                if (auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->add = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;
                } else {
                    std::cerr << "Error parsing binary expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr->var = term.value();
                return expr;
            }
        } else {
            return {};
        }
    }

    std::optional<NodeStmt *> parse_stmt() {
        if (peek().value().type == TokenType::_exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::closed_paren) {
                consume();
            } else {
                std::cerr << "Expected ')' " << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                std::cerr << "Expected ';' " << std::endl;
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        } else if (peek().has_value() && peek().value().type == TokenType::let &&
                   peek(1).has_value() && peek(1).value().type == TokenType::ident &&
                   peek(2).has_value() && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                std::cerr << "expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        } else {
            return {};
        }
    }

    std::optional<NodeProgram> parse_prog() {
        NodeProgram prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    [[nodiscard]] inline std::optional<Token> peek(const int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume() {
        return m_tokens.at(m_index++);
    }

    const std::vector<Token> m_tokens;
    int m_index = 0;
    ArenaAllocator m_allocator;
};
