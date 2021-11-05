import sys
import pickle

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


    print('clauses:')
    print(-(edges[-1]+1))
    for c in range(C):
        for e in edges:
            if asn[vs['p',c,e]]: print(e,end=' ')
            elif asn[vs['n',c,e]]: print(-e,end=' ')
        print()

if __name__ == '__main__':
    fname_stub = sys.argv[1]
    outfile = fname_stub + '.out'
    loadfile = fname_stub + '.pkl'
    process_output(outfile, loadfile, should_print=True)
