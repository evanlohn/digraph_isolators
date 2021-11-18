def isolator_clauses(n, only_units=False):
    with open(f"isolator{n}.txt") as f:
        cs = []
        in_clauses = False
        for line in f:
            if in_clauses:
                c = [int(x) for x in line.split()[:-1]]
                if only_units and len(c) > 1:
                    continue
                cs += [c]
            else: in_clauses = line.strip() == "clauses:"
    return CNF(cs)

def map_graphs(n, mapname=None):
    if mapname is None:
        mapname = f"map{n}.txt"
    with open(mapname) as f:
        data = [[int(x) for x in line.split()] for line in f]
    graphs = {frozenset(line[1:-1]): line[0] for line in data}
    return graphs

def filter_graphs(graphs, clauses):

    prev_classes = set(graphs.values())
    filt_graphs = {g: graphs[g] for g in graphs if clauses.satisfies(Indexer(lambda i: i in g))}
    filt_classes = set (filt_graphs.values())
    if len(prev_classes) != len(filt_classes):
        print('using {} isolator deletes equivalence classes:')
        for c in prev_classes:
            if c in filt_classes:
                continue
            for g in graphs:
                if graphs[g] == c:
                    print(f'class {c}, example graph {list(g)}')
        assert False
    return filt_graphs

from collections import defaultdict


# potential name collisions but this doesn't need to be perfect
class CNF:
    def __init__(self, clauses = None, var = None):
        self.var = defaultdict(self.next_literal)
        if var is not None:
            for k,v in var.items():
                self.var[k] = v
        self.clauses = [] if clauses is None else clauses
        self.unit = None
    def next_literal(self):
        return len(self.var)+1
    def fresh(self):
        return self.var["fresh", len(self.var)]
    def __getitem__(self, key):
        return self.var[key]
    def __iadd__(self, clauses):
        self.add_all(clauses)
        return self
    def __len__(self):
        return len(self.clauses)

    def non_units(self):
        return len([x for x in self.clauses if len(x) > 1])
    def true(self):
        if self.unit is None:
            self.unit = self.var["unit"]
            self.clauses += [[self.unit]]
        return self.unit
    def add(self, clause):
        self.clauses.append(list(clause))
    def add_all(self, clauses):
        for clause in clauses:
            self.add(clause)
    def naive_unitprop(self):
        asn = {}
        last_len = -1
        cnf = self.clauses
        while len(asn) > last_len:
            try:
                unit = next(c[0] for c in cnf if len(c) == 1)
            except:
                break
            asn[abs(unit)] = unit > 0
            cnf = [[l for l in c if l != -unit] for c in cnf if unit not in c]
        if [] in cnf:
            return CNF([[]], self.var)
        cnf = cnf + [[l] if v else [-l] for l,v in asn.items()]
        return CNF(cnf, self.var)
    def atmostn(self, n, lits):
        lits = list(lits)
        dp = [[self.fresh() for _ in range(n+1)] for _ in lits]
        cs = []
        for i,x in enumerate(lits):
            for j in range(n+1):
                if i: cs += [[-dp[i-1][j], dp[i][j]]]
                if not j: cs += [[-x, dp[i][j]]]
                elif i: cs += [[-dp[i-1][j-1], -x, dp[i][j]]]
        cs += [[-dp[-1][n]]]
        return cs
    def satisfies(self, assignment):
        return all(
            any(assignment[abs(lit)] == (lit > 0) for lit in clause)
            for clause in self.clauses
        )
    def __str__(self):
        nvars = max([0] + [abs(lit) for clause in self.clauses for lit in clause])
        header = f"p cnf {nvars} {len(self.clauses)}"
        #comments = [f"c {k} => {v}" for k,v in self.var.items()]
        comments = []
        clauses = [" ".join(str(x) for x in clause + [0]) for clause in self.clauses]
        return "\n".join([header]+comments+clauses)

class Indexer:
    def __init__(self, fcn):
        self.fcn = fcn
    def __getitem__(self, key):
        return self.fcn(key)
