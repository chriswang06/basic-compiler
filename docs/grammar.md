$$

\begin{align}
    [\text{Prog}] &\to [\text{Stmt}]^*\\
    [\text{Stmt}] &\to 
    \begin{cases}
        \text{exit}([\text{Expr}]);\\
        \text{let} \space\text{ident} = [\text{Expr}]\\
                \text{ident} = [\text{Expr}]\\
        [\text{Scope}]\\
        \text{if} ([\text{Expr}])[\text{Scope}]\\

    \end{cases}\\
    [\text{Scope}] &\to
    \begin{cases}
        \{[\text{Stmt}]*\}\\
    \end{cases}\\
    [\text{IfPred}] & \to
    \begin{cases}
        \text{elif}(\text{[Expr]})\text{[Scope]}\\
        \text{else}\text{[Scope]}\\
        \epsilon
    \end{cases}\\

    [\text{Expr}] &\to 
    \begin{cases}
        \text{Term};\\
        \text{BinExpr}\\
        \text{CondExpr}\\

    \end{cases}\\
    [\text{Term}] &\to 
    \begin{cases}
        \text{int\_lit};\\
        \text{ident}\\
        ([\text{Expr}])

    \end{cases}\\
    [\text{BinExpr}] &\to
    \begin{cases}
        [\text{Expr}] * [\text{Expr}] & \text{prec} = 1\\
        [\text{Expr}] / [\text{Expr}] & \text{prec} = 1\\
        [\text{Expr}] + [\text{Expr}] & \text{prec} = 0\\
        [\text{Expr}] - [\text{Expr}] & \text{prec} = 0\\
    \end{cases}\\
    [\text{CondExpr}] &\to
    \begin{cases}
        [\text{Expr}] > [\text{Expr}] \\
        [\text{Expr}] < [\text{Expr}] \\
        [\text{Expr}] >= [\text{Expr}] \\
        [\text{Expr}] <= [\text{Expr}] \\
        [\text{Expr}] == [\text{Expr}] \\
    \end{cases}
\end{align}
$$