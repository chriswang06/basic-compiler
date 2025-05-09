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
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMult {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMult*, NodeBinExprSub*, NodeBinExprDiv*> var;
};


struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};
struct NodeCondExprGreater {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeCondExprGreaterEq {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeCondExprLess {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeCondExprLessEq {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeCondExprEq {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeCondExprNotEq {
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeCondExpr {
    std::variant<NodeCondExprGreater*, NodeCondExprGreaterEq*, NodeCondExprLess*, NodeCondExprLessEq*, NodeCondExprNotEq*, NodeCondExprEq*> var;
};
struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*, NodeCondExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};


struct NodeStmtLet {
    NodeExpr* expr{};
    Token ident;
};

struct NodeStmt;
struct NodeUnaryAdd {
    NodeTermIdent* term_ident;
};
struct NodeUnarySub{
    NodeTermIdent* term_ident;
};

struct NodeUnary {
    std::variant<NodeUnarySub*,NodeUnaryAdd*> var;
};
struct NodeScope {
    std::vector<NodeStmt*> stmts;
};
struct NodeStmtWhile {
    NodeExpr* expr;
    NodeScope* scope;
};
struct NodeIfPred;

struct NodeStmtIf {
    NodeExpr* expr;
    NodeScope* scope;
    std::optional<NodeIfPred* > pred;
};


struct NodeIfPredElif {
    NodeExpr* expr{};
    NodeScope* scope{};
    std::optional<NodeIfPred*> pred;
};

struct NodeIfPredElse {
    NodeScope* scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredElif*, NodeIfPredElse*> var;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr* expr{};
};
struct NodeCompoundPlus{
    NodeTermIdent* term_ident;
    NodeTerm* term;
};
struct NodeCompoundSub{
    NodeTermIdent* term_ident;
    NodeTerm* term;
};
struct NodeCompoundMult{
    NodeTermIdent* term_ident;
    NodeTerm* term;
};
struct NodeCompoundDiv{
    NodeTermIdent* term_ident;
    NodeTerm* term;
};
struct NodeCompound {
    std::variant<NodeCompoundPlus*, NodeCompoundSub*, NodeCompoundDiv*, NodeCompoundMult*> var;
};
struct NodeVarReassign {
    std::variant<NodeUnary*, NodeCompound*> var;
};
struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtLet*, NodeScope*, NodeStmtIf*, NodeStmtAssign*, NodeStmtWhile*, NodeVarReassign*> var;
};

struct NodeProgram {
    std::vector<NodeStmt* > stmts;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)),
          m_allocator(1024 * 1024 * 4) // 4 MB
    {
    }
    void error_expected(const std::string& msg) const{
        std::cerr << "[Parsing Error] Expected "<< msg <<" on line " << peek(-1).value().line << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<NodeTerm*> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.emplace<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.emplace<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        if (auto ident = try_consume(TokenType::ident)) {
            auto term_ident = m_allocator.emplace<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.emplace<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        if (const auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("'expression'");
            }
            try_consume_err(TokenType::closed_paren);
            auto term_paren = m_allocator.emplace<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.emplace<NodeTerm>();
            term->var = term_paren;
            return term;
        }
        return {};
    }

    std::optional<NodeExpr*> parse_expr(const int min_prec = 0) {
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }

        auto expr_lhs = m_allocator.emplace<NodeExpr>();
        expr_lhs->var = term_lhs.value();

        while (true) {
            std::optional<Token> cur_tok = peek();
            std::optional<int> prec;
            if (cur_tok.has_value()) {
                prec = bin_prec(cur_tok->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            } else {
                break;
            }
            const auto [type, line, value] = consume();
            const int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                error_expected("'expression'");

            }

            auto expr_lhs2 = m_allocator.emplace<NodeExpr>();
            expr_lhs2->var = expr_lhs->var;
            if (type == TokenType::plus) {
                auto add = m_allocator.emplace<NodeBinExprAdd>();
                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                auto bin_expr = m_allocator.emplace<NodeBinExpr>();
                bin_expr->var = add;
                expr_lhs->var = bin_expr;
            } else if (type == TokenType::star) {
                auto mult = m_allocator.emplace<NodeBinExprMult>();
                mult->lhs = expr_lhs2;
                mult->rhs = expr_rhs.value();
                auto bin_expr = m_allocator.emplace<NodeBinExpr>();
                bin_expr->var = mult;
                expr_lhs->var = bin_expr;

            } else if (type == TokenType::minus) {
                auto sub = m_allocator.emplace<NodeBinExprSub>();
                sub->lhs = expr_lhs2;
                sub->rhs = expr_rhs.value();
                auto bin_expr = m_allocator.emplace<NodeBinExpr>();
                bin_expr->var = sub;
                expr_lhs->var = bin_expr;

            } else if (type == TokenType::fslash) {
                auto div = m_allocator.emplace<NodeBinExprDiv>();
                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                auto bin_expr = m_allocator.emplace<NodeBinExpr>();
                bin_expr->var = div;
                expr_lhs->var = bin_expr;

            } else if (type == TokenType::greater) {
                auto greater = m_allocator.emplace<NodeCondExprGreater>();
                greater->lhs = expr_lhs2;
                greater->rhs = expr_rhs.value();
                auto cond_expr = m_allocator.emplace<NodeCondExpr>();
                cond_expr->var = greater;
                expr_lhs->var = cond_expr;

            }
            else if (type == TokenType::less) {
                auto less = m_allocator.emplace<NodeCondExprLess>();
                less->lhs = expr_lhs2;
                less->rhs = expr_rhs.value();
                auto cond_expr = m_allocator.emplace<NodeCondExpr>();
                cond_expr->var = less;
                expr_lhs->var = cond_expr;

            }
            else if (type == TokenType::greaterequal) {
                auto greaterequal = m_allocator.emplace<NodeCondExprGreaterEq>();
                greaterequal->lhs = expr_lhs2;
                greaterequal->rhs = expr_rhs.value();
                auto cond_expr = m_allocator.emplace<NodeCondExpr>();
                cond_expr->var = greaterequal;
                expr_lhs->var = cond_expr;

            }
            else if (type == TokenType::lessequal) {
                auto lessequal = m_allocator.emplace<NodeCondExprLessEq>();
                lessequal->lhs = expr_lhs2;
                lessequal->rhs = expr_rhs.value();
                auto cond_expr = m_allocator.emplace<NodeCondExpr>();
                cond_expr->var = lessequal;
                expr_lhs->var = cond_expr;

            }
            else if (type == TokenType::equiv) {
                auto equiv = m_allocator.emplace<NodeCondExprEq>();
                equiv->lhs = expr_lhs2;
                equiv->rhs = expr_rhs.value();
                auto cond_expr = m_allocator.emplace<NodeCondExpr>();
                cond_expr->var = equiv;
                expr_lhs->var = cond_expr;

            }
            else if (type == TokenType::notequiv) {
                auto notequiv = m_allocator.emplace<NodeCondExprNotEq>();
                notequiv->lhs = expr_lhs2;
                notequiv->rhs = expr_rhs.value();
                auto cond_expr = m_allocator.emplace<NodeCondExpr>();
                cond_expr->var = notequiv;
                expr_lhs->var = cond_expr;

            }

            else {
                assert(false && "unreachable");
            }

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
        try_consume_err(TokenType::closed_curly);
        return scope;
    }
    std::optional<NodeIfPred*> parse_if_pred() {
        if (try_consume(TokenType::elif)) {
            try_consume_err(TokenType::open_paren);
            const auto elif = m_allocator.emplace<NodeIfPredElif>();

            if (const auto expr = parse_expr()) {
                elif->expr = expr.value();
            } else {
                error_expected("'expression'");
            }
            try_consume_err(TokenType::closed_paren);
            if (const auto scope = parse_scope()) {
                elif->scope = scope.value();
            } else {
                error_expected("'Scope'");
            }
            elif->pred = parse_if_pred();
            auto pred = m_allocator.emplace<NodeIfPred>(elif);
            return pred;
        }
        if (try_consume(TokenType::else_)) {
            const auto else_ = m_allocator.emplace<NodeIfPredElse>();

            if (const auto scope = parse_scope()) {
                else_->scope = scope.value();
            } else {
                error_expected("'Scope'");
            }
            auto pred = m_allocator.emplace<NodeIfPred>(else_);
            return pred;
        }
        return {};
    }
    std::optional<NodeVarReassign*> parse_var_reassign() {
        //parsing unary

        if (peek().has_value() && peek().value().type == TokenType::ident &&
            peek(1).has_value() && (peek(1).value().type == TokenType::unary_plus || peek(1).value().type == TokenType::unary_minus)) {

            const auto term_ident = m_allocator.emplace<NodeTermIdent>();
            term_ident->ident = consume();

            if (try_consume(TokenType::unary_minus).has_value()) {
                auto minus = m_allocator.emplace<NodeUnarySub>();
                minus->term_ident = term_ident;
                auto unary = m_allocator.emplace<NodeUnary>();
                unary->var = minus;
                auto var_reassign = m_allocator.emplace<NodeVarReassign>();
                var_reassign->var = unary;
                try_consume_err(TokenType::semi);

                return var_reassign;
            }
            if (try_consume(TokenType::unary_plus).has_value()) {
                auto plus = m_allocator.emplace<NodeUnaryAdd>();
                plus->term_ident = term_ident;
                auto unary = m_allocator.emplace<NodeUnary>();
                unary->var = plus;
                auto var_reassign = m_allocator.emplace<NodeVarReassign>();
                var_reassign->var = unary;
                try_consume_err(TokenType::semi);
                return var_reassign;
            }


            }

        //compound
        if (peek().has_value() && peek().value().type == TokenType::ident &&
            peek(1).has_value() && (peek(1).value().type == TokenType::compound_add ||
            peek(1).value().type == TokenType::compound_sub ||
            peek(1).value().type == TokenType::compound_mul ||
            peek(1).value().type == TokenType::compound_div)) {

            const auto term_ident = m_allocator.emplace<NodeTermIdent>();
            term_ident->ident = consume();

            if (try_consume(TokenType::compound_add).has_value()) {
                auto add = m_allocator.emplace<NodeCompoundPlus>();
                add->term_ident = term_ident;
                if (const auto term = parse_term()) {
                    add->term = term.value();
                }
                else {
                    error_expected("'term'");
                }
                auto compound = m_allocator.emplace<NodeCompound>();
                compound->var = add;
                auto var_reassign = m_allocator.emplace<NodeVarReassign>();
                var_reassign->var = compound;
                try_consume_err(TokenType::semi);
                return var_reassign;
            }
            if (try_consume(TokenType::compound_sub).has_value()) {
                auto sub = m_allocator.emplace<NodeCompoundSub>();
                sub->term_ident = term_ident;
                if (const auto term = parse_term()) {
                    sub->term = term.value();
                }
                else {
                    error_expected("'term'");
                }
                auto compound = m_allocator.emplace<NodeCompound>();
                compound->var = sub;
                auto var_reassign = m_allocator.emplace<NodeVarReassign>();
                var_reassign->var = compound;
                try_consume_err(TokenType::semi);
                return var_reassign;
            }
            if (try_consume(TokenType::compound_div).has_value()) {
                auto div = m_allocator.emplace<NodeCompoundDiv>();
                div->term_ident = term_ident;
                if (const auto term = parse_term()) {
                    div->term = term.value();
                }
                else {
                    error_expected("'term'");
                }
                auto compound = m_allocator.emplace<NodeCompound>();
                compound->var = div;
                auto var_reassign = m_allocator.emplace<NodeVarReassign>();
                var_reassign->var = compound;
                try_consume_err(TokenType::semi);
                return var_reassign;
            }
            if (try_consume(TokenType::compound_mul).has_value()) {
                auto mul = m_allocator.emplace<NodeCompoundMult>();
                mul->term_ident = term_ident;
                if (const auto term = parse_term()) {
                    mul->term = term.value();
                }
                else {
                    error_expected("'term'");
                }
                auto compound = m_allocator.emplace<NodeCompound>();
                compound->var = mul;
                auto var_reassign = m_allocator.emplace<NodeVarReassign>();
                var_reassign->var = compound;
                try_consume_err(TokenType::semi);
                return var_reassign;
            }

            }
        return {};
    }
    std::optional<NodeStmt*> parse_stmt() {
        //parsing exit
        if (peek().value().type == TokenType::exit && peek(1).has_value()
            && peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.emplace<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                error_expected("'expression'");
            }
            try_consume_err(TokenType::closed_paren);
            try_consume_err(TokenType::semi);

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        }
        //parsing variable declaration
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
                error_expected("'expression'");

            }

            try_consume_err(TokenType::semi);

            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        }
        //parsing assignment
        if (peek().has_value() && peek().value().type == TokenType::ident &&
            peek(1).has_value() && peek(1).value().type == TokenType::eq) {
            const auto assign = m_allocator.emplace<NodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                assign->expr = expr.value();
            } else {
                error_expected("'Expression'");

            }
            try_consume_err(TokenType::semi);
            auto stmt = m_allocator.emplace<NodeStmt>(assign);
            return stmt;
        }
        //parsing scopes
        if (peek().has_value() && peek().value().type == TokenType::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.emplace<NodeStmt>();
                stmt->var = scope.value();
                return stmt;
            }
            error_expected("'scope'");

        }
        //parsing if statements
        if (auto if_ = try_consume(TokenType::if_)) {
            try_consume_err(TokenType::open_paren);
            auto stmt_if = m_allocator.emplace<NodeStmtIf>();
            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                error_expected("'expression'");
            }
            try_consume_err(TokenType::closed_paren);

            if (auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                error_expected("'scope'");

            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_if;
            return stmt;
        }
        //parsing while
        if (auto while_ = try_consume(TokenType::while_)) {
            try_consume_err(TokenType::open_paren);
            auto stmt_while = m_allocator.emplace<NodeStmtWhile>();
            if (auto expr = parse_expr()) {
                stmt_while->expr = expr.value();
            } else {
                error_expected("'expression'");
            }
            try_consume_err(TokenType::closed_paren);
            if (auto scope = parse_scope()) {
                stmt_while->scope = scope.value();
            } else {
                error_expected("'scope'");
            }
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = stmt_while;
            return stmt;


        }
        //parsing reassign operators
        if (auto var_reassign = parse_var_reassign()) {
            auto stmt = m_allocator.emplace<NodeStmt>();
            stmt->var = var_reassign.value();
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
                error_expected("statement");

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


    Token try_consume_err(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(to_string(type));
        return {};
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
