import argparse
import pickle

import util
import sys
from collections import defaultdict
from itertools import combinations

def main(n_verts, n_sbp_clauses, mapname, file_stub):
    E = n_verts * (n_verts-1) // 2
    graphs = util.map_graphs(n_verts)
    graphs = {k:v for k,v in graphs.items() if E not in k}
    edges = range(1,E) # edge E is forced not present

    # sat variables and clauses:
    # 'k'ills,clause,graph
    # 'c'anon,graph
    # 'p'ositive literal,clause,edge
    # 'n'egative literal,clause,edge
    # 'u'nit,'p/n'os/neg,edge
    cnf = util.CNF()

    for i,g in enumerate(graphs):
        # set up kills
        for c in range(n_sbp_clauses):
            good_lits = [cnf[('p' if e in g else 'n'),c,e] for e in edges]
            # one good literal implies the graph is alive
            cnf += [[-l,-cnf['k',c,i]] for l in good_lits]
            # the graph being alive implies a good literal
            cnf += [[cnf['k',c,i]] + good_lits]
        # set up canon
        # the kill clauses are either a real clause or a unit that
        # goes against the graph
        kill_clauses = [cnf['k',c,i] for c in range(n_sbp_clauses)] \
            + [cnf['u',('n' if e in g else 'p'),e] for e in edges]
        # any kill clause implies the graph is not canonical
        cnf += [[-c, -cnf['c',i]] for c in kill_clauses]
        # the graph not being canonical implies a kill clause
        cnf += [[cnf['c',i]] + kill_clauses]

    classes = defaultdict(list)
    for i,g in enumerate(graphs): classes[graphs[g]].append(i)

    # ensure we have exactly one canonical graph
    for cl,gs in classes.items():
        # at least one
        cnf += [[cnf['c',i] for i in gs]]
        # at most one
        cnf += cnf.atmostn(1, [cnf['c',i] for i in gs])

    # ensure that if a unit exists we don't have it in a clause
    for c in range(n_sbp_clauses):
        for e in edges:
            cnf += [
                [-cnf['u','p',e], -cnf['p',c,e]],
                [-cnf['u','n',e], -cnf['p',c,e]],
                [-cnf['u','p',e], -cnf['n',c,e]],
                [-cnf['u','n',e], -cnf['n',c,e]],
            ]

    # ensure we don't have a positive and a negative
    cnf += [[-cnf['p',c,e],-cnf['n',c,e]] for c in range(n_sbp_clauses) for e in edges]

    # ensure we have a lexicographic ordering of clauses
    for c1,c2 in zip(range(n_sbp_clauses),range(1,n_sbp_clauses)):
        # two empty sequences are equal
        last_eq = cnf.true()
        for e in edges:
            # lex on positive and negative
            for kind in 'pn':
                next_eq = cnf.fresh()
                cnf += [
                    # if equal so far, we are not lex smaller
                    [-last_eq, cnf[kind,c1,e], -cnf[kind,c2,e]],
                    # if not equal, then remain not equal
                    [last_eq, -next_eq],
                    # if greater, then not equal
                    [-cnf[kind,c1,e], cnf[kind,c2,e], -next_eq],
                    # convert the above two and apply demorgans so we end up
                    # with "not equal iff previously not equal or currently
                    # greater"
                    [next_eq, -last_eq, cnf[kind,c1,e]],
                    [next_eq, -last_eq, -cnf[kind,c2,e]],
                ]
                last_eq = next_eq
        # should not have two equal clauses
        cnf += [[-last_eq]]

    # dump as dimacs
    if file_stub is None:
        print(f"p cnf {len(vs)} {len(cs)}")
        for c in cs: print(*c, 0)
        pkl_file = 'tmp.pkl'
    else:
        pkl_file = file_stub + '.pkl'
        cnf_file = file_stub + '.cnf'
        with open(cnf_file, 'w') as f:
            f.write(str(cnf)+"\n")
    with open(pkl_file, 'wb') as f:
        pickle.dump((n_sbp_clauses, n_verts, cnf, edges), f)

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
