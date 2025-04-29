#pragma once
#include <algorithm>

#include "./parser.hpp"
#include "cassert"

class Generator {
public:
    inline explicit Generator(NodeProgram prog)
        : m_prog(std::move(prog)) {
    }

    void gen_term(const NodeTerm *term) {
        struct TermVisitor {
            Generator &gen;

            void operator()(const NodeTermIntLit *term_int_lit) const {
                gen.m_output << "    mov x0, #" << term_int_lit->int_lit.value.value() << "\n";
                gen.push_point("x0", gen.m_stack_size);
            }

            void operator()(const NodeTermIdent *term_ident) const {
                const auto it = std::ranges::find_if(
                    gen.m_vars.cbegin(),
                    gen.m_vars.cend(),
                    [&](const Var &var) { return var.name == term_ident->ident.value.value(); });
                if (it == gen.m_vars.cend()) {
                    std::cerr << "Undeclared Identifier: " << term_ident->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_output << "    ldr x0, [sp, #" << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]\n";
                gen.push_point("x0", gen.m_stack_size);
            }

            void operator()(const NodeTermParen *term_paren) const {
                gen.gen_expr(term_paren->expr);
            }
        };
        TermVisitor visitor({.gen = *this});
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr *bin_expr) {
        struct BinExprVisitor {
            Generator &gen;

            void operator()(const NodeBinExprSub *sub) const {
                gen.gen_expr(sub->rhs);
                gen.gen_expr(sub->lhs);
                size_t stack_size = gen.m_stack_size;
                gen.pop_point("x1", stack_size-1);
                gen.pop_point("x0", stack_size-2);
                gen.m_output << "    sub x0, x0, x1\n";
                gen.push_point("x0", gen.m_stack_size);
            }

            void operator()(const NodeBinExprAdd *add) const {
                gen.gen_expr(add->rhs);
                gen.gen_expr(add->lhs);
                size_t stack_size = gen.m_stack_size;
                gen.pop_point("x1", stack_size-1);
                gen.pop_point("x0", stack_size-2);
                gen.m_output << "    add x0, x0, x1\n";
                gen.push_point("x0", gen.m_stack_size);
            }

            void operator()(const NodeBinExprMult *mult) const {
                gen.gen_expr(mult->rhs);
                gen.gen_expr(mult->lhs);
                size_t stack_size = gen.m_stack_size;
                gen.pop_point("x1", stack_size-1);
                gen.pop_point("x0", stack_size-2);
                gen.m_output << "    mul x0, x0, x1\n";
                gen.push_point("x0", gen.m_stack_size);
            }

            void operator()(const NodeBinExprDiv *div) const {
                gen.gen_expr(div->rhs);
                gen.gen_expr(div->lhs);
                size_t stack_size = gen.m_stack_size;
                gen.pop_point("x1", stack_size-1);
                gen.pop_point("x0", stack_size-2);
                gen.m_output << "    sdiv x0, x0, x1\n";
                gen.push_point("x0", gen.m_stack_size);
            }
        };

        BinExprVisitor visitor{.gen = *this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr *expr) {
        struct ExprVisitor {
            Generator &gen;

            void operator()(const NodeTerm *term) const {
                gen.gen_term(term);
            }

            void operator()(const NodeBinExpr *bin_expr) const {
                gen.gen_bin_expr(bin_expr);
            }
        };
        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);
    }

    void gen_scope(const NodeScope *scope) {
        begin_scope();
        for (const NodeStmt *stmts: scope->stmts) {
            gen_stmt(stmts);
        }
        end_scope();
    }
    void gen_if_pred(const NodeIfPred* pred, const std::string& end_label)
    {
        struct PredVisitor {
            Generator& gen;
            const std::string& end_label;

            void operator()(const NodeIfPredElif* elif) const
            {
                gen.m_output << "    ;; elif\n";
                gen.gen_expr(elif->expr);
                gen.pop_point("x0", gen.m_stack_size-1);
                const std::string label = gen.create_label();
                gen.m_output << "    cbz x0, " << label << "\n";
                gen.gen_scope(elif->scope);
                gen.m_output << "    b " << end_label << "\n";
                if (elif->pred.has_value()) {
                    gen.m_output << label << ":\n";
                    gen.gen_if_pred(elif->pred.value(), end_label);
                }
            }

            void operator()(const NodeIfPredElse* else_) const
            {
                gen.m_output << "    ;; else\n";
                gen.gen_scope(else_->scope);
            }
        };

        PredVisitor visitor { .gen = *this, .end_label = end_label };
        std::visit(visitor, pred->var);
    }
    void gen_stmt(const NodeStmt *stmt) {
        struct StmtVisitor {
            Generator &gen;

            void operator()(const NodeStmtExit *stmt_exit) const {
                gen.gen_expr(stmt_exit->expr);
                gen.m_output << "    ;; exit\n";
                gen.m_output << "    mov x16, #1\n";
                gen.pop_point("x0", gen.m_stack_size-1);
                gen.m_output << "    svc #0\n";
                gen.m_output << "    ;; /exit\n";
            }

            void operator()(const NodeStmtLet *stmt_let) const {
                const auto it = std::ranges::find_if(
                    gen.m_vars.cbegin(),
                    gen.m_vars.cend(),
                    [&](const Var &var) { return var.name == stmt_let->ident.value.value(); });
                if (it != gen.m_vars.cend()) {
                    std::cerr << "Identifier already used: " << stmt_let->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }


                gen.m_vars.push_back({.name = stmt_let->ident.value.value(), .stack_loc = gen.m_stack_size});
                gen.gen_expr(stmt_let->expr);
            }
            void operator()(const NodeStmtAssign* stmt_assign) const {
                const auto it = std::ranges::find_if(gen.m_vars, [&](const Var& var) {
                    return var.name == stmt_assign->ident.value.value();
                });
                if (it == gen.m_vars.end()) {
                    std::cerr << "Undeclared identifier" << stmt_assign->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen.m_output << "    ;; var reassignment\n";
                gen.gen_expr(stmt_assign->expr);
                gen.pop_point("x0", gen.m_stack_size-1);
                gen.m_output << "    str x0, [sp, #" << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]\n";
            }

            void operator()(const NodeScope *scope) const {
                gen.gen_scope(scope);
            }

            void operator()(const NodeStmtIf* stmt_if) const
            {
                gen.m_output << "    ;; if\n";
                gen.gen_expr(stmt_if->expr);
                gen.pop_point("x0", gen.m_stack_size-1);
                const std::string label = gen.create_label();
                gen.m_output << "    cbz x0, " << label << "\n";
                gen.gen_scope(stmt_if->scope);
                if (stmt_if->pred.has_value()) {
                    const std::string end_label = gen.create_label();
                    gen.m_output << "    b " << end_label << "\n";
                    gen.m_output << label << ":\n";
                    gen.gen_if_pred(stmt_if->pred.value(), end_label);
                    gen.m_output << end_label << ":\n";
                }
                else {
                    gen.m_output << label << ":\n";
                }
                gen.m_output << "    ;; /if\n";
            }

        };

        StmtVisitor visitor{.gen = *this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string gen_prog() {
        m_output << ".global _main\n_main:\n";
        // Setup proper stack frame
        m_output << "    stp x29, x30, [sp, #-16]!\n";
        m_output << "    mov x29, sp\n";

        for (const NodeStmt *stmt: m_prog.stmts) {
            gen_stmt(stmt);
        }

        // Restore stack frame
        m_output << "    mov sp, x29\n";
        m_output << "    ldp x29, x30, [sp], #16\n";
        m_output << "    mov x16, #1\n";
        m_output << "    mov x0, #0\n";
        m_output << "    svc #0";
        return m_output.str();
    }

private:
    void push_point(const std::string &reg, const size_t pointer) {
        m_output << "    str " << reg << ", [sp, #" << pointer*8 <<"]\n";
        m_stack_size++;
    }

    void push_offset(const std::string &reg, const size_t offset) {
        m_output << "    str " << reg << ", [sp, #" << offset*8 << "]\n";
        m_stack_size++;
    }

    void pop_point(const std::string &reg, const size_t pointer) {
        m_output << "    ldr " << reg << ", [sp, #" << pointer*8 << "]\n";
        m_stack_size--;
    }

    void pop_offset(const std::string &reg, const size_t offset) {
        m_output << "    ldr " << reg << ", [sp, #" << offset*8 << "]\n";
        m_stack_size--;
    }

    void begin_scope() {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope() {
        size_t pop_count = m_vars.size() - m_scopes.back();
        if (pop_count > 0) {
            m_output << "    add sp, sp, #" << pop_count * 8 << "\n";
            m_stack_size -= pop_count;
            for (int i = 0; i < pop_count; i++) {
                m_vars.pop_back();
            }
        }
        m_scopes.pop_back();
    }

    std::string create_label()
    {
        std::stringstream ss;
        ss << "label" << m_label_count++;
        return ss.str();
    }

    struct Var {
        std::string name;
        size_t stack_loc;
    };

    const NodeProgram m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::vector<Var> m_vars{};
    std::vector<size_t> m_scopes{};
    int m_label_count = 0;
};