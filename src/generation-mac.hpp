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
                gen.push_expr("x0");
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
                size_t offset = it->stack_loc * 8;

                gen.m_output << "    ;; Loading variable " << term_ident->ident.value.value()
                            << " (stack_loc=" << it->stack_loc
                            << ") from offset " << offset << "\n";
                gen.m_output << "    ldr x0, [sp, #" << offset << "]\n";
                gen.push_expr("x0");
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
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs
                gen.m_output << "    sub x0, x0, x1\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeBinExprAdd *add) const {
                gen.gen_expr(add->rhs);
                gen.gen_expr(add->lhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs
                gen.m_output << "    add x0, x0, x1\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeBinExprMult *mult) const {
                gen.gen_expr(mult->rhs);
                gen.gen_expr(mult->lhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs
                gen.m_output << "    mul x0, x0, x1\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeBinExprDiv *div) const {
                gen.gen_expr(div->rhs);
                gen.gen_expr(div->lhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs
                gen.m_output << "    sdiv x0, x0, x1\n";
                gen.push_expr("x0");
            }
        };

        BinExprVisitor visitor{.gen = *this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_cond_expr(const NodeCondExpr *cond_expr) {
        struct CondExprVisitor {
            Generator &gen;

            void operator()(const NodeCondExprGreater *greater) const {
                gen.gen_expr(greater->lhs);
                gen.gen_expr(greater->rhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs

                const std::string true_label = "greater_" + gen.create_label();
                const std::string end_label = "end_" + gen.create_label();

                gen.m_output << "    cmp x0, x1\n";
                gen.m_output << "    b.gt " << true_label << "\n";

                // Not greater
                gen.m_output << "    mov x0, #0\n";
                gen.m_output << "    b " << end_label << "\n";

                gen.m_output << true_label << ":\n";
                // Greater
                gen.m_output << "    mov x0, #1\n";

                gen.m_output << end_label << ":\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeCondExprGreaterEq *greater_eq) const {
                gen.gen_expr(greater_eq->lhs);
                gen.gen_expr(greater_eq->rhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs

                const std::string true_label = "greater_eq_" + gen.create_label();
                const std::string end_label = "end_" + gen.create_label();

                gen.m_output << "    cmp x0, x1\n";
                gen.m_output << "    b.ge " << true_label << "\n";

                // Not greater equal
                gen.m_output << "    mov x0, #0\n";
                gen.m_output << "    b " << end_label << "\n";

                gen.m_output << true_label << ":\n";
                // Greater equal
                gen.m_output << "    mov x0, #1\n";

                gen.m_output << end_label << ":\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeCondExprLess *less) const {
                gen.gen_expr(less->lhs);
                gen.gen_expr(less->rhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs

                const std::string true_label = "less_" + gen.create_label();
                const std::string end_label = "end_" + gen.create_label();

                gen.m_output << "    cmp x0, x1\n";
                gen.m_output << "    b.lt " << true_label << "\n";

                // Not less
                gen.m_output << "    mov x0, #0\n";
                gen.m_output << "    b " << end_label << "\n";

                gen.m_output << true_label << ":\n";
                // Less
                gen.m_output << "    mov x0, #1\n";

                gen.m_output << end_label << ":\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeCondExprLessEq *less_eq) const {
                gen.gen_expr(less_eq->lhs);
                gen.gen_expr(less_eq->rhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs

                const std::string true_label = "less_eq_" + gen.create_label();
                const std::string end_label = "end_" + gen.create_label();

                gen.m_output << "    cmp x0, x1\n";
                gen.m_output << "    b.le " << true_label << "\n";

                // Not less equal
                gen.m_output << "    mov x0, #0\n";
                gen.m_output << "    b " << end_label << "\n";

                gen.m_output << true_label << ":\n";
                // Less equal
                gen.m_output << "    mov x0, #1\n";

                gen.m_output << end_label << ":\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeCondExprEq *eq) const {
                gen.gen_expr(eq->lhs);
                gen.gen_expr(eq->rhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs

                const std::string true_label = "eq_" + gen.create_label();
                const std::string end_label = "end_" + gen.create_label();

                gen.m_output << "    cmp x0, x1\n";
                gen.m_output << "    b.eq " << true_label << "\n";

                // Not equal
                gen.m_output << "    mov x0, #0\n";
                gen.m_output << "    b " << end_label << "\n";

                gen.m_output << true_label << ":\n";
                // Equal
                gen.m_output << "    mov x0, #1\n";

                gen.m_output << end_label << ":\n";
                gen.push_expr("x0");
            }

            void operator()(const NodeCondExprNotEq *not_eq_) const {
                gen.gen_expr(not_eq_->lhs);
                gen.gen_expr(not_eq_->rhs);
                gen.pop_expr("x1");  // rhs
                gen.pop_expr("x0");  // lhs
                const std::string true_label = "not_eq_" + gen.create_label();
                const std::string end_label = "end_" + gen.create_label();

                gen.m_output << "    cmp x0, x1\n";
                gen.m_output << "    b.ne " << true_label << "\n";

                // Equal
                gen.m_output << "    mov x0, #0\n";
                gen.m_output << "    b " << end_label << "\n";

                gen.m_output << true_label << ":\n";
                // Not equal
                gen.m_output << "    mov x0, #1\n";

                gen.m_output << end_label << ":\n";
                gen.push_expr("x0");
            }
        };
        CondExprVisitor visitor{.gen = *this};
        std::visit(visitor, cond_expr->var);
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
            void operator()(const NodeCondExpr *cond_expr) const {
                gen.gen_cond_expr(cond_expr);
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

    void gen_if_pred(const NodeIfPred* pred, const std::string& end_label) {
        struct PredVisitor {
            Generator& gen;
            const std::string& end_label;

            void operator()(const NodeIfPredElif* elif) const {
                gen.m_output << "    ;; elif\n";
                gen.gen_expr(elif->expr);
                gen.pop_expr("x0");
                const std::string label = gen.create_label();
                gen.m_output << "    cbz x0, " << label << "\n";
                gen.gen_scope(elif->scope);
                gen.m_output << "    b " << end_label << "\n";
                if (elif->pred.has_value()) {
                    gen.m_output << label << ":\n";
                    gen.gen_if_pred(elif->pred.value(), end_label);
                }
            }

            void operator()(const NodeIfPredElse* else_) const {
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
                gen.m_output << "    ;; Evaluating exit expression\n";
                gen.gen_expr(stmt_exit->expr);
                gen.m_output << "    ;; Exit value on stack\n";

                gen.m_output << "    ;; exit\n";
                gen.m_output << "    mov x16, #1\n";
                gen.pop_expr("x0");
                gen.m_output << "    ;; Exit with value in x0\n";
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

                // Allocate a location for the variable
                size_t var_loc = gen.m_var_count;
                gen.m_vars.push_back({.name = stmt_let->ident.value.value(), .stack_loc = var_loc});
                gen.m_var_count++;
                gen.m_output << "    ;; variable '" << stmt_let->ident.value.value()
              << "' allocated at offset " << var_loc * 8 << "\n";

                // Evaluate the expression
                gen.gen_expr(stmt_let->expr);

                // Store the result to the variable location
                gen.pop_expr("x0");
                gen.m_output << "    str x0, [sp, #" << var_loc * 8 << "]\n";
            }

            void operator()(const NodeStmtAssign* stmt_assign) const {
                const auto it = std::ranges::find_if(gen.m_vars, [&](const Var& var) {
                    return var.name == stmt_assign->ident.value.value();
                });
                if (it == gen.m_vars.end()) {
                    std::cerr << "Undeclared identifier" << stmt_assign->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen.m_output << "    ;; reassigning variable '" << stmt_assign->ident.value.value()
                << "' at offset " << it->stack_loc * 8 << "\n";
                gen.gen_expr(stmt_assign->expr);
                gen.pop_expr("x0");
                gen.m_output << "    str x0, [sp, #" << it->stack_loc * 8 << "]\n";
            }

            void operator()(const NodeScope *scope) const {
                gen.gen_scope(scope);
            }

            void operator()(const NodeStmtIf* stmt_if) const {
                gen.m_output << "    ;; if\n";
                gen.gen_expr(stmt_if->expr);
                gen.pop_expr("x0");
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
    // Helper methods for expression evaluation
    void push_expr(const std::string &reg) {
        size_t offset = m_var_count * 8 + m_expr_stack_size * 8;
        m_output << "    str " << reg << ", [sp, #" << offset << "]\n";
        m_expr_stack_size++;
    }

    void pop_expr(const std::string &reg) {
        m_expr_stack_size--;
        size_t offset = m_var_count * 8 + m_expr_stack_size * 8;
        m_output << "    ldr " << reg << ", [sp, #" << offset << "]\n";
    }

    void begin_scope() {
        m_scopes.push_back(m_vars.size());
    }

    void end_scope() {
        size_t pop_count = m_vars.size() - m_scopes.back();
        if (pop_count > 0) {
            m_var_count -= pop_count; // Adjust variable count
            for (int i = 0; i < pop_count; i++) {
                m_vars.pop_back();
            }
        }
        m_scopes.pop_back();
    }

    std::string create_label() {
        std::stringstream ss;
        ss << "label" << m_label_count++;
        return ss.str();
    }

    struct Var {
        std::string name;
        size_t stack_loc; // Fixed offset for this variable
    };

    const NodeProgram m_prog;
    std::stringstream m_output;
    size_t m_var_count = 0;        // Number of variables
    size_t m_expr_stack_size = 0;  // Size of expression evaluation stack
    std::vector<Var> m_vars{};
    std::vector<size_t> m_scopes{};
    int m_label_count = 0;
};