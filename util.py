def isolator_clauses(n):
    with open(f"isolator{n}.txt") as f:
        cs = []
        in_clauses = False
        for line in f:
            if in_clauses:
                cs += [[int(x) for x in line.split()[:-1]]]
            else: in_clauses = line.strip() == "clauses:"
    return cs

def map_graphs(n, mapname=None):
    if mapname is None:
        mapname = f"map{n}.txt"
    with open(mapname) as f:
        data = [[int(x) for x in line.split()] for line in f]
    graphs = {frozenset(line[1:-1]): line[0] for line in data}
    return graphs

def filter_graphs(graphs, clauses):
    def satisfies_var(graph, v):
        return v in graph if v>0 else abs(v) not in graph

    def admits(graph):
        # "all" clauses must admit the graph, clause admission determined
        # by the graph satisfying "any" variable in the clause.
        return all([any([satisfies_var(graph, v) for v in c]) for c in clauses])

    prev_classes = set(graphs.values())
    filt_graphs = {g: graphs[g] for g in graphs if admits(g)}
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

def atmostone(vs, fresh):
    dp = [[fresh() for _ in range(2)] for _ in vs]
    cs = []
    for i,x in enumerate(vs):
        for j in range(2):
            if i: cs += [[-dp[i-1][j], dp[i][j]]]
            if not j: cs += [[-x, dp[i][j]]]
            elif i: cs += [[-dp[i-1][j-1], -x, dp[i][j]]]
    cs += [[-dp[-1][1]]]
    return cs

def naive_unitprop(cnf):
    asn = {}
    last_len = -1
    while len(asn) > last_len:
        try:
            unit = next(c[0] for c in cnf if len(c) == 1)
        except:
            break
        asn[abs(unit)] = unit > 0
        cnf = [[l for l in c if l != -unit] for c in cnf if unit not in c]
    return cnf + [[l] if v else [-l] for l,v in asn.items()]
