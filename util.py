def isolator_clauses(n):
    with open(f"isolator{n}.txt") as f:
        cs = []
        in_clauses = False
        for line in f:
            if in_clauses:
                cs += [[int(x) for x in line.split()[:-1]]]
            else: in_clasuses = line.strip() == "clauses"
    return cs

def map_graphs(n):
    with open(f"map{n}.txt") as f:
        data = [[int(x) for x in line.split()] for line in f]
    graphs = {frozenset(line[1:-1]): line[0] for line in data}
    return graphs

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
