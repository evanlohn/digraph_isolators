import sys
import pickle
import util

def process_output(outfile, loadfile, should_print=False):
    with open(outfile, 'r') as f:
        lines = f.readlines()
    soln = []
    for line in lines:
        if line[0] == 's':
            result = line.split(' ')[1]
            if result.strip() != 'SATISFIABLE':
                if should_print:
                    print('solver returned {}'.format(result))
                return None
        if line[0] != 'v':
            continue
        soln.extend(line.strip().split(' ')[1:])

    soln = [int(x) for x in soln if int(x) != 0]
    asn = {abs(x):x>0 for x in soln}

    with open(loadfile, 'rb') as f:
        C, N, cnf, edges = pickle.load(f)

    if should_print:
        print()

        for i,j in sorted(vs.items()):
            print(i, asn[j])

    clauses = util.CNF()
    clauses += [[-(edges[-1]+1)]]
    for c in range(C):
        cur = []
        for e in edges:
            if asn[cnf['p',c,e]]: cur += [e]
            elif asn[cnf['n',c,e]]: cur += [-e]
        clauses += [cur]
    for e in edges:
        if asn[cnf['u','p',e]]: clauses += [[e]]
        if asn[cnf['u','n',e]]: clauses += [[-e]]

    print(clauses)

    clauses = clauses.naive_unitprop()

    # check is valid
    graphs = util.map_graphs(N)

    classes_total = set(v for k,v in graphs.items())
    classes_found = set()
    for graph,class_ in graphs.items():
        if clauses.satisfies(util.Indexer(lambda i: i in graph)):
            print(*graph,"is canonical for class",class_)
            assert class_ not in classes_found
            classes_found.add(class_)
    assert classes_total == classes_found

    print()
    print('clauses:')
    print(clauses)

if __name__ == '__main__':
    fname_stub = sys.argv[1]
    outfile = fname_stub + '.out'
    loadfile = fname_stub + '.pkl'
    process_output(outfile, loadfile)
