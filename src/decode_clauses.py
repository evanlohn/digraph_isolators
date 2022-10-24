import sys
import pickle
import util

def extract_kissat(fd):
    soln = {}
    for line in fd:
        if line[0] == 's':
            result = line.split(' ')[1]
            if result.strip() != 'SATISFIABLE':
                print('solver returned {}'.format(result))
                return None
        if line[0] != 'v':
            continue
        for lit in line.strip().split()[1:]:
            soln[abs(int(lit))] = int(lit) > 0
    return soln

def extract_allsat(fd):
    solns = []
    for line in fd:
        if not line.strip().endswith(" 0"): continue
        asn = {}
        for lit in line.strip().split()[:-1]:
            x = int(lit)
            asn[abs(x)] = x > 0
        solns += [asn]
    return solns

def from_asn(asn, pkl_dct, should_print=False):
    C, N, cnf, edges = pkl_dct['C'], pkl_dct['N'], pkl_dct['cnf'], pkl_dct['edges'],
    use_last = 'last_isolator' in pkl_dct

    clauses = util.CNF()
    for c in range(C):
        cur = []
        for e in edges:
            if asn[cnf['p',c,e]]: cur += [e]
            elif asn[cnf['n',c,e]]: cur += [-e]
        clauses += [cur]
    for e in edges:
        if asn[cnf['u','p',e]]: clauses += [[e]]
        if asn[cnf['u','n',e]]: clauses += [[-e]]

    if use_last:
        clauses += pkl_dct['last_isolator'].clauses
    clauses = clauses.naive_unitprop()

    # check is valid
    graphs = util.map_graphs(N)

    classes_total = set(v for k,v in graphs.items())
    classes_found = set()

    for graph,class_ in graphs.items():
        if clauses.satisfies(util.Indexer(lambda i: i in graph)):
            if should_print:
                print(*graph,"is canonical for class",class_)

            assert class_ not in classes_found
            classes_found.add(class_)
    assert classes_total == classes_found
    if should_print:
        print()
        print('clauses:')
        for cl in clauses.clauses: print(*cl,0)

    return clauses

from itertools import permutations

def to_edgelabel(start, end):
    ma = max(start, end)
    mi = min(start, end)
    lit = (ma - 2) * (ma - 1) // 2 + mi
    return lit if start < end else -lit

edgedict = {}
nextstart, nextend = 1, 2

def from_edgelabel(edge):
    global nextstart, nextend, edgedict
    while abs(edge) not in edgedict:
        edgedict[to_edgelabel(nextstart, nextend)] = nextstart, nextend
        nextstart += 1
        if nextstart == nextend:
            nextstart, nextend = 1, nextend + 1
    fst, snd = edgedict[abs(edge)]
    return (fst, snd) if edge > 0 else  (snd, fst)

def process_allsat(outfile, loadfile):
    with open(outfile) as fd:
        asns = extract_allsat(fd)
    with open(loadfile, 'rb') as f:
        pkl_dct = pickle.load(f)
    isolators = [from_asn(asn, pkl_dct) for asn in asns]
    # make frozen so we can reduce more easily
    isolators = [
        frozenset(frozenset(clause) for clause in isolator.clauses)
        for isolator in isolators]
    lits = set(
        abs(lit)
        for isolator in isolators
        for clause in isolator
        for lit in clause)
    unique = set()
    verts = pkl_dct["N"]
    for isolator in isolators:
        for perm in permutations(range(1,verts+1)):
            def apply_to_edge(e):
                start, end = from_edgelabel(e)
                start = perm[start-1]
                end = perm[end-1]
                return to_edgelabel(start, end)
            permuted = frozenset(
                frozenset(apply_to_edge(lit) for lit in clause)
                for clause in isolator)
            if permuted in unique:
                break
        else:
            unique.add(isolator)
    for isolator in unique:
        for clause in isolator:
            print(*clause)
        print()


def process_output(outfile, loadfile, should_print=False):
    with open(outfile) as fd:
        asn = extract_kissat(fd)
    if asn is None:
        return 
    
    with open(loadfile, 'rb') as f:
        pkl_dct = pickle.load(f)
    return from_asn(asn, pkl_dct, should_print=should_print)

def main(fname_stub, do_allsat):
    outfile = fname_stub + '.out'
    loadfile = fname_stub + '.pkl'
    if do_allsat:
        process_allsat(do_allsat, loadfile)
    else:
        process_output(outfile, loadfile, should_print=True)

if __name__ == '__main__':
    fname_stub = sys.argv[1]
    do_allsat = len(sys.argv) > 2 and sys.argv[2]
    main(fname_stub, do_allsat)
