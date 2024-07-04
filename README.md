# cadical-lit-count

Core idea: whenever you learn a clause, keep track of how many times 
each literal occurs. At specified intervals, dump the most seen literal which
is not already fixed

Ideas:
    1) flag to only count non-redundant clauses
