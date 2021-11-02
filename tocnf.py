import argparse
import pickle

import sys
from collections import defaultdict
from itertools import combinations

def main(n_verts, n_sbp_clauses, mapname, file_stub):

    with open(mapname, 'r') as f:
        data = [[int(x) for x in line.split()] for line in f]

    graphs = {frozenset(line[1:]): line[0] for line in data}
    E = n_verts * (n_verts-1) // 2
    edges = range(1,E+1)

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
        for c in range(n_sbp_clauses):
            good_lits = [vs[('p' if e in g else 'n'),c,e] for e in edges]
            # one good literal implies the graph is alive
            cs += [[-l,-vs['k',c,i]] for l in good_lits]
            # the graph being alive implies a good literal
            cs += [[vs['k',c,i]] + good_lits]
        # set up canon
        kill_clauses = [vs['k',c,i] for c in range(n_sbp_clauses)]
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
    cs += [[-vs['p',c,e],-vs['n',c,e]] for c in range(n_sbp_clauses) for e in edges]

    # dump as dimacs
    if file_stub is None:
        print(f"p cnf {len(vs)} {len(cs)}")
        for c in cs: print(*c, 0)
        pkl_file = 'tmp.pkl'
    else:
        pkl_file = file_stub + '.pkl'
        cnf_file = file_stub + '.cnf'
        lines = [f"p cnf {len(vs)} {len(cs)}\n"]
        lines += [' '.join([str(x) for x in c]) + ' 0\n' for c in cs]
        with open(cnf_file, 'w') as f:
            f.writelines(lines)
    with open(pkl_file, 'wb') as f:
        static_vs = {x:vs[x] for x in vs}
        pickle.dump((n_sbp_clauses, n_verts, static_vs, edges), f)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='produce cnf for complete digraph perfect SBP')
    parser.add_argument('N', type=int, help='vertices in the graph')
    parser.add_argument('C', type=int, help='clauses in the SBP (Symmetry Breaking Predicate)')
    parser.add_argument('--mapname', type=str, default=None, help='map file produced by gen_map_files.py. default is map\\{N\\}.txt')
    parser.add_argument('--filename', type=str, default=None, help='stub name for output files. default to stdout')
    args = parser.parse_args()
    if args.mapname is None:
        args.mapname = f'map{args.N}.txt'
    main(args.N, args.C, args.mapname, args.filename)