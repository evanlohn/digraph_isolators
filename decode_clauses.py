import sys
import pickle

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
        C, N, vs, edges = pickle.load(f)

    if should_print:
        print()

        for i,j in sorted(vs.items()):
            print(i, asn[j])

    clauses = [[-(edges[-1]+1)]]
    for c in range(C):
        cur = []
        for e in edges:
            if asn[vs['p',c,e]]: cur += [e]
            elif asn[vs['n',c,e]]: cur += [-e]
        clauses += [cur]

    clauses = naive_unitprop(clauses)

    # check is valid
    with open(f"map{N}.txt") as f:
        data = [[int(x) for x in line.split()] for line in f]
        graphs = {frozenset(line[1:-1]): line[0] for line in data}

    classes_total = set(v for k,v in graphs.items())
    classes_found = set()
    for graph,class_ in graphs.items():
        for cl in clauses:
            good = False
            for lit in cl:
                if lit < 0 and abs(lit) not in graph: good = True
                if lit > 0 and abs(lit) in graph: good = True
            if not good:
                break
        else:
            print(*graph,"is canonical for class",class_)
            assert class_ not in classes_found
            classes_found.add(class_)
    assert classes_total == classes_found

    print()
    print('clauses:')
    for cl in clauses: print(*cl,0)

if __name__ == '__main__':
    fname_stub = sys.argv[1]
    outfile = fname_stub + '.out'
    loadfile = fname_stub + '.pkl'
    process_output(outfile, loadfile)
