#pragma once
#include <cassert>
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

struct NodeTermParen {
    NodeExpr* expr;
};
struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMult {
    NodeExpr *lhs;
    NodeExpr *rhs;
};
struct NodeBinExprSub {
    NodeExpr *lhs;
    NodeExpr *rhs;
};
struct NodeBinExprDiv {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd *, NodeBinExprMult *, NodeBinExprSub*, NodeBinExprDiv*> var;
};


struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen*> var;
};


struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};


struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    NodeExpr* expr{};
    Token ident;
};
struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeStmtIf {
    NodeExpr* expr;
    NodeScope* scope;
};
struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *, NodeScope*, NodeStmtIf*> var;
};

struct NodeProgram {
    std::vector<NodeStmt *> stmts;
};

class Parser {
public:
     explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)),
          m_allocator(1024 * 1024 * 4) // 4 MB
    {
    }

    std::optional<NodeTerm *> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.emplace<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.emplace<NodeTerm>();
            term->var = term_int_lit;
            return term;
        } if (auto ident = try_consume(TokenType::ident)){
            auto term_ident = m_allocator.emplace<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.emplace<NodeTerm>();
            term->var = term_ident;
            return term;
        } if (auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                std::cerr << "expected expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::closed_paren, "Expected ')'");
            auto term_paren = m_allocator.emplace<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.emplace<NodeTerm>();
            term->var = term_paren;
            return term;
        }
            return {};
    }

    std::optional<NodeExpr *> parse_expr(const int min_prec = 0) {
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }

        auto expr_lhs = m_allocator.emplace<NodeExpr>();
        expr_lhs->var = term_lhs.value();

        while (true) {
            std::optional<Token> cur_tok = peek();
            std::optional<int> prec;
            if (cur_tok.has_value()){
                prec = bin_prec(cur_tok->type);
                if (!prec.has_value() || prec < min_prec){
                    break;
                }
            } else {
                break;
            }
            const auto [type, value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                std::cerr<<"unable to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            auto expr = m_allocator.emplace<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.emplace<NodeExpr>();
            if (type == TokenType::plus) {
                auto add = m_allocator.emplace<NodeBinExprAdd>();
                expr_lhs2->var = expr_lhs->var;
                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                expr->var = add;
            }
            else if (type == TokenType::star) {
                auto mult = m_allocator.emplace<NodeBinExprMult>();
                expr_lhs2->var = expr_lhs->var;
                mult->lhs = expr_lhs2;
                mult->rhs = expr_rhs.value();
                expr->var = mult;
            }
            else if (type == TokenType::minus) {
                auto sub = m_allocator.emplace<NodeBinExprSub>();
                expr_lhs2->var = expr_lhs->var;
                sub->lhs = expr_lhs2;
                sub->rhs = expr_rhs.value();
                expr->var = sub;
            }
            else if (type == TokenType::fslash) {
                auto div = m_allocator.emplace<NodeBinExprDiv>();
                expr_lhs2->var = expr_lhs->var;
                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                expr->var = div;
            } else {
                assert(false && "unreachable");
            }

            expr_lhs->var = expr;
        }
        return expr_lhs;
    }

    std::optional<NodeScope*> parse_scope() {
        if (!try_consume(TokenType::open_curly).has_value()) {
            return {};
        }
            auto scope = m_allocator.emplace<NodeScope>();
            while (auto stmt = parse_stmt()) {
                scope->stmts.push_back(stmt.value());
            }
            try_consume(TokenType::closed_curly, "Expected '}'");
            return scope;
    }
    std::optional<NodeStmt *> parse_stmt() {
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.emplace<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::closed_paren, "Expected ')'");
            try_consume(TokenType::semi, "Expected ';'");

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
         if (peek().has_value() && peek().value().type == TokenType::let &&
                   peek(1).has_value() && peek(1).value().type == TokenType::ident &&
                   peek(2).has_value() && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = m_allocator.emplace<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            
            try_consume(TokenType::semi, "Expected ';'");

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }
         if (peek().has_value() && peek().value().type == TokenType::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.emplace<NodeStmt>();
                stmt->var = scope.value();
                return stmt;
            }
                std::cerr << "invalid scope" << std::endl;
                exit(EXIT_FAILURE);

        }
        if (auto if_ = try_consume(TokenType::if_)) {
            try_consume(TokenType::open_paren, "Expected '('");
            auto stmt_if = m_allocator.emplace<NodeStmtIf>();
            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::closed_paren, "Expected ')'");

            if (auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                std::cerr << "Invalid Scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_if;
            return stmt;

        }
            return {};
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
    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        }
            return m_tokens.at(m_index + offset);

    }

    Token consume() {
        return m_tokens.at(m_index++);
    }

    Token try_consume(TokenType type, const std::string& err_msg) {
        if (peek().has_value() && peek().value().type == type) {
        return consume();
        }
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
    }
    std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
            return {};
    }

    const std::vector<Token> m_tokens;
    int m_index = 0;
    ArenaAllocator m_allocator;
};
