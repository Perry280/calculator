# n: number
# e: empty string
# $: end of input
#
# A  -> B0 | + B0 | - B0
# B0 -> C0 B1
# B1 -> + C0 B1 | - C0 B1 | e
# C0 -> D0 C1
# C1 -> * D0 C1 | / D0 C1 | e
# D0 -> X D1
# D1 -> ^ D0 | e
# X  -> ( A ) | n

import pprint
from typing import override

EMPTY_STR: str = "e"
EMPTY_STR_SET: frozenset[str] = frozenset({EMPTY_STR})

EOI: str = "$"
EOI_SET: frozenset[str] = frozenset({EOI})

class Rule:
    lhs: str
    rhs: list[str]

    def __init__(self: Rule, lhs: str, rhs: list[str]) -> None:
        self.lhs = lhs
        self.rhs = rhs

    @override
    def __str__(self: Rule) -> str:
        return f"'{self.lhs} -> {' '.join(self.rhs)}'"

    @override
    def __repr__(self: Rule) -> str:
        return self.__str__()

    @override
    def __eq__(self: Rule, o: object) -> bool:
        return isinstance(o, Rule) and self.lhs == o.lhs and self.rhs == o.rhs

    @override
    def __hash__(self: Rule) -> int:
        return hash((self.lhs, tuple(self.rhs)))

class Grammar:
    S: str
    T: frozenset[str]
    NT: frozenset[str]
    P: frozenset[Rule]

    def __init__(self: Grammar, S:str, T: frozenset[str], NT: frozenset[str], P: frozenset[Rule]) -> None:
        self.S = S
        self.T  = T
        self.NT = NT
        self.P  = P

    def symbols(self: Grammar) -> frozenset[str]:
        return self.T | self.NT

    def is_terminal(self: Grammar, symbol: str) -> bool:
        return symbol in self.T

    def is_non_terminal(self: Grammar, symbol: str) -> bool:
        return symbol in self.NT

def gammar_from_rules(rules: list[str], S: str) -> Grammar:
    pass

def first_sets(G: Grammar) -> dict[str, set[str]]:
    first: dict[str, set[str]] = { x: {x} for x in (G.T | EMPTY_STR_SET | EOI_SET) }
    first |= { x: set() for x in G.NT }

    sets_updated: bool = True
    while sets_updated:
        sets_updated = False
        for p in G.P:
            rhs: set[str] = (first[p.rhs[0]] - EMPTY_STR_SET)
            i: int = 0
            while i < len(p.rhs) - 1 and EMPTY_STR in first[p.rhs[i]]:
                rhs |= (first[p.rhs[i+1]] - EMPTY_STR_SET)
                i += 1
            
            if i == len(p.rhs) - 1 and EMPTY_STR in first[p.rhs[i]]:
                rhs |= EMPTY_STR_SET

            pre_update_len: int = len(first[p.lhs])
            first[p.lhs] |= rhs
            sets_updated |= len(first[p.lhs]) > pre_update_len

    return first

def FIRST(G: Grammar, first: dict[str, set[str]], string: str | list[str]) -> set[str]:
    if isinstance(string, str):
        return first[string].copy()
    if len(string) == 1:
        return first[string[0]].copy()
    symbol = string[0]
    if EMPTY_STR not in first[symbol]:
        return first[symbol].copy()
    return (first[symbol] - EMPTY_STR_SET) | FIRST(G, first, string[1:])


def follow_sets(G: Grammar, first: dict[str, set[str]]) -> dict[str, set[str]]:
    follow: dict[str, set[str]] = { x: set() for x in G.NT }
    follow[G.S] = set(EOI_SET)

    sets_updated: bool = True
    while sets_updated:
        sets_updated = False
        for p in G.P:
            trailer: set[str] = follow[p.lhs].copy()
            for b in reversed(p.rhs):
                if b in G.NT:
                    pre_update_len: int = len(follow[b])
                    follow[b] |= trailer
                    sets_updated |= len(follow[b]) > pre_update_len

                    if EMPTY_STR in first[b]:
                        trailer |= (first[b] - EMPTY_STR_SET)
                    else:
                        trailer = first[b].copy()
                else:
                    trailer = first[b].copy()

    return follow

def augmented_first_sets(G: Grammar, first: dict[str, set[str]], follow: dict[str, set[str]]) -> dict[Rule, set[str]]:
    augmented: dict[Rule, set[str]] = {}

    for p in G.P:
        augmented[p] = FIRST(G, first, p.rhs)
        if EMPTY_STR in augmented[p]:
            augmented[p] |= follow[p.lhs]

    return augmented

def LL1_table(G: Grammar, augmented: dict[Rule, set[str]]) -> dict[str, dict[str, list[str]]]:
    table: dict[str, dict[str, list[str]]] = { nt: {} for nt in G.NT }

    for p in G.P:
        for t in augmented[p]:
            if table[p.lhs].get(t):
                raise ValueError(f"LL(1) conflict at [{p.lhs}, {t}]: {table[p.lhs][t]} vs {p.rhs}")
            if t != EMPTY_STR:
                table[p.lhs][t] = p.rhs[:]
        if EOI in augmented[p]:
            table[p.lhs][EOI] = p.rhs[:]

    return table

def print_table(G: Grammar, table: dict[str, dict[str, list[str]]]) -> None:
    columns: list[str] = sorted(G.T) + [EOI]
    rows: list[str] = sorted(G.NT)

    def cell(nt: str, t: str) -> str:
        rhs = table.get(nt, {}).get(t)
        return ' '.join(rhs) if rhs else ""

    header: list[str] = [""] + columns
    body: list[list[str]] = [[nt] + [cell(nt, t) for t in columns] for nt in rows]

    widths: list[int] = [max(len(r[i]) for r in [header] + body) for i in range(len(header))]

    def fmt(row: list[str]) -> str:
        return "| " + " | ".join(c.ljust(w) for c, w in zip(row, widths)) + " |"

    sep: str = "+-" + "-+-".join("-" * w for w in widths) + "-+"

    print(sep)
    print(fmt(header))
    print(sep)
    for r in body:
        print(fmt(r))
    print(sep)

def main() -> None:
    G: Grammar = Grammar(
        S="A",
        T=frozenset({ "+", "-", "*", "/", "^", "(", ")", "n" }),
        NT=frozenset({ "A", "B0", "B1", "C0", "C1", "D0", "D1", "X" }),
        P=frozenset({ 
            Rule("A", ["B0"],),
            Rule("A", ["+", "B0"],),
            Rule("A", ["-", "B0"],),
            Rule("B0", ["C0", "B1"],),
            Rule("B1", ["+", "C0", "B1"],),
            Rule("B1", ["-", "C0", "B1"],),
            Rule("B1", ["e"],),
            Rule("C0", ["D0", "C1"],),
            Rule("C1", ["*", "D0", "C1"],),
            Rule("C1", ["/", "D0", "C1"],),
            Rule("C1", ["e"],),
            Rule("D0", ["X", "D1"],),
            Rule("D1", ["^", "D0"],),
            Rule("D1", ["e"],),
            Rule("X", ["(", "A", ")"],),
            Rule("X", ["n"],)
        })
    )

    first = first_sets(G)
    print("FIRST")
    pprint.pprint(first)

    follow = follow_sets(G, first)
    print("\nFOLLOW")
    pprint.pprint(follow)

    augmented = augmented_first_sets(G, first, follow)
    print("\nAUGMENTED FIRST")
    pprint.pprint(augmented)

    table = LL1_table(G, augmented)
    print("\nLL(1) table")
    print_table(G, table)

if __name__ == "__main__":
    main()
