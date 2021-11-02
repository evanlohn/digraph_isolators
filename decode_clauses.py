import sys
import pickle

result = """
v 1 -2 -3 -4 -5 -6 7 -8 9 -10 11 -12 -13 -14 -15 -16 -17 -18 19 -20 -21 -22 23
v -24 25 -26 -27 28 29 -30 -31 32 -33 34 35 -36 37 -38 39 -40 41 -42 43 -44 45
v -46 47 -48 49 -50 51 -52 0
"""

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