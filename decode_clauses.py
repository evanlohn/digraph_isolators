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
        pkl_dct = pickle.load(f)
        C, N, vs, edges = pkl_dct['C'], pkl_dct['N'], pkl_dct['vs'], pkl_dct['edges'], 
        use_last = 'last_isolator' in pkl_dct

    if should_print:
        print()

        for i,j in sorted(vs.items()):
            print(i, asn[j])

    clauses = []
    for c in range(C):
        cur = []
        for e in edges:
            if asn[vs['p',c,e]]: cur += [e]
            elif asn[vs['n',c,e]]: cur += [-e]
        clauses += [cur]

    if use_last:
        clauses += pkl_dct['last_isolator']
    else:
        clauses += [[-(edges[-1]+1)]]
    clauses = util.naive_unitprop(clauses)


    if should_print:
        print()
        print('clauses:')
        for cl in clauses: print(*cl,0)

    # check is valid
    with open(f"map{N}.txt") as f:
        data = [[int(x) for x in line.split()] for line in f]
        graphs = {frozenset(line[1:-1]): line[0] for line in data}

    classes_total = set(v for k,v in graphs.items())
    classes_found = set()
    for graph, class_ in graphs.items():
        for cl in clauses:
            good = False
            for lit in cl:
                if lit < 0 and abs(lit) not in graph: good = True
                if lit > 0 and abs(lit) in graph: good = True
            if not good:
                break
        else:
            if should_print:
                print(*graph,"is canonical for class",class_)
            assert class_ not in classes_found
            classes_found.add(class_)
    assert classes_total == classes_found


    return clauses

def main(fname_stub, should_print=False):
    outfile = fname_stub + '.out'
    loadfile = fname_stub + '.pkl'
    return process_output(outfile, loadfile, should_print=should_print)

if __name__ == '__main__':
    fname_stub = sys.argv[1]
    main(fname_stub, should_print=True)
