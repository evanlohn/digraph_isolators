import argparse
import pickle

import util
import sys
import os
from collections import defaultdict
from itertools import combinations

# TODO:
# compute 6 (fully or incrementally (units only or all))
#
# add extension via graph filtering / base
# allsat to enumerate all isolators on 4/5
# require unit clause to erase all others

def main(args, out_dir='./'):
    n_verts, n_sbp_clauses = args.N, args.C
    use_last, use_last_units = args.use_last, args.use_last_units
    mapname, file_stub = args.mapname, args.filename
    E = n_verts * (n_verts-1) // 2
    graphs = util.map_graphs(n_verts, mapname=mapname)
    if use_last:
        last_isolator = util.isolator_clauses(n_verts - 1, only_units=use_last_units)
        graphs = util.filter_graphs(graphs, last_isolator)

        n_sbp_clauses -= last_isolator.non_units()
        edges = range(1,E+1)
    else:
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
        print(f"{str(cnf)}\n")
        pkl_file = 'tmp.pkl'
    else:
        pkl_file = os.path.join(out_dir,file_stub + '.pkl')
        cnf_file = os.path.join(out_dir, file_stub + '.cnf')
        with open(cnf_file, 'w') as f:
            f.write(str(cnf)+"\n")
    with open(pkl_file, 'wb') as f:
        pkl_dct = {'C':n_sbp_clauses, 'N':n_verts, 'cnf':cnf, 'edges': edges}
        if use_last:
            pkl_dct['last_isolator'] = last_isolator
        pickle.dump(pkl_dct, f)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='produce cnf for complete digraph perfect SBP')
    parser.add_argument('N', type=int, help='vertices in the graph')
    parser.add_argument('C', type=int, help='non-unit clauses in the SBP (Symmetry Breaking Predicate)')
    parser.add_argument('--use-last', action='store_true', help='build the isolator for N off the isolator for N-1 from isolator{N-1}.txt')
    parser.add_argument('--use-last-units', action='store_true', help='use only the unit clauses from the last isolator')
    parser.add_argument('--mapname', type=str, default=None, help='map file produced by gen_map_files.py. default is map\\{N\\}.txt')
    parser.add_argument('--filename', type=str, default=None, help='stub name for output files. default to stdout')
    args = parser.parse_args()
    if args.mapname is None:
        args.mapname = f'map{args.N}.txt'
    main(args)

