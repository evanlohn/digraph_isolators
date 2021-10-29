# verts in graph
N = 3
# clauses
C = 2


import sys
from collections import defaultdict

with open(sys.argv[1]) as f:
    data = [[int(x) for x in line.split()] for line in f]

graphs = {frozenset(line[1:]): line[0] for line in data}
E = N * (N-1) // 2
edges = range(1,E+1)

from itertools import combinations

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

# sat variables and clauses:
# 'k'ills,clause,graph
# 'c'anon,graph
# 'p'ositive literal,clause,edge
# 'n'egative literal,clause,edge
vs = defaultdict(lambda:len(vs)+1)
cs = []
fresh = lambda:vs['fresh',len(vs)]

for i,g in enumerate(graphs):
    # set up kills
    for c in range(C):
        good_lits = [vs[('p' if e in g else 'n'),c,e] for e in edges]
        # one good literal implies the graph is alive
        cs += [[-l,-vs['k',c,i]] for l in good_lits]
        # the graph being alive implies a good literal
        cs += [[vs['k',c,i]] + good_lits]
    # set up canon
    kill_clauses = [vs['k',c,i] for c in range(C)]
    # any kill clause implies the graph is not canonical
    cs += [[-c, -vs['c',i]] for c in kill_clauses]
    # the graph not being canonical implies a kill clause
    cs += [[vs['c',i]] + kill_clauses]

classes = defaultdict(list)
for i,g in enumerate(graphs): classes[graphs[g]].append(i)

# ensure we have exactly one canonical graph
for cl,gs in classes.items():
    # at least one
    cs += [[vs['c',i] for i in gs]]
    # at most one
    cs += atmostone([vs['c',i] for i in gs], fresh)

# ensure we don't have a positive and a negative
cs += [[-vs['p',c,e],-vs['n',c,e]] for c in range(C) for e in edges]

# dump as dimacs
print(f"p cnf {len(vs)} {len(cs)}")
for c in cs: print(*c, 0)

result = """
v 1 -2 -3 -4 -5 -6 7 -8 9 -10 11 -12 -13 -14 -15 -16 -17 -18 19 -20 -21 -22 23
v -24 25 -26 -27 28 29 -30 -31 32 -33 34 35 -36 37 -38 39 -40 41 -42 43 -44 45
v -46 47 -48 49 -50 51 -52 0
"""

result = sum([[int(x) for x in line.split()[1:] if int(x)] for line in result.splitlines()], [])
asn = {abs(x):x>0 for x in result}

print()

for i,j in sorted(vs.items()):
    print(i, asn[j])

for c in range(C):
    for e in edges:
        if asn[vs['p',c,e]]: print(e,end=' ')
        elif asn[vs['n',c,e]]: print(-e,end=' ')
    print()
