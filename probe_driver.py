import os
import sys
from collections import Counter
import time


if __name__ == '__main__':
    N = int(sys.argv[1])
    num_experiments = int(sys.argv[2])
    start = 0 if len(sys.argv) <=3 else int(sys.argv[3])
    n_clauses_counts = Counter()
    examples = {}
    probe_time = 0
    io_time = 0
    for i in range(start, num_experiments + start):
        s = time.time()
        os.system(f'./probe map{N}.txt {i} > sus.cnf')
        os.system(f'./filter map{N}.txt sus.cnf > sus.out')
        s2 = time.time()
        probe_time += s2 -s
        with open('sus.out', 'r') as f:
            for line in f.readlines():
                if 'ERROR' in line:
                    print(f'found a bug: experiment {i}')
        with open('sus.cnf', 'r') as f:
            lines = f.readlines()
            n_clauses = len(lines)
            examples[n_clauses] = (i, lines)
            n_clauses_counts[n_clauses] += 1
        io_time += time.time() - s2
    counts_ordered = [(k, n_clauses_counts[k]) for k in n_clauses_counts]
    counts_ordered.sort(key=lambda kvp: kvp[0])
    print(counts_ordered)
    print(f'probe time: {probe_time}, io_time:{io_time}')
    for n_clauses, ct in counts_ordered[:3]:
        seed, ex = examples[n_clauses]
        print(f'length {n_clauses} seed {seed}')
        for line in ex:
            print(line)
        print()