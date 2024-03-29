import os
import subprocess
import signal
import sys
from collections import Counter
import time


n_eq_classes = {3: 2, 4:4, 5: 12, 6: 56, 7: 456, 8: 6880}

def output_current_isos(n_clauses_counts, probe_time, io_time):
    counts_ordered = [(k, n_clauses_counts[k]) for k in n_clauses_counts]
    counts_ordered.sort(key=lambda kvp: kvp[0])
    print(counts_ordered)
    print(f'probe time: {probe_time}, io_time:{io_time}')
    for n_clauses, ct in counts_ordered[:3]:
        seed, ex = examples[n_clauses]
        print(f'length {n_clauses} seed {seed}')
        for line in ex:
            print(line[:-1])
        print()

if __name__ == '__main__':
    N = int(sys.argv[1])
    num_experiments = int(sys.argv[2])
    start = 0 if len(sys.argv) <=3 else int(sys.argv[3])
    mapfile = f'map{N}.txt' if len(sys.argv) <=4  else sys.argv[4]
    n_clauses_counts = Counter()
    examples = {}
    probe_time = 0
    io_time = 0
    def signal_handler(signal, frame):
        print(f'Terminated early with {i} experiments done')
        output_current_isos(n_clauses_counts, probe_time, io_time)
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_handler)
    for i in range(num_experiments):
        s = time.time()
        seed = i + start
        #os.system(f'./probe {mapfile} {seed} > sus.cnf')
        with open('sus.cnf', 'w') as f:
            subprocess.run(['./probe', f'{mapfile}', f'{seed}'], stdout=f)
        #os.system(f'./filter {mapfile} sus.cnf > sus.out')
        #with open('sus.cnf', 'r') as f:
        #    lines = f.readlines()
        #    if 'clause counts 0' in lines[-1]:
        #        print(i)
        #        exit()
        with open('sus.out', 'w') as f:
            subprocess.run(['./filter', f'{mapfile}', 'sus.cnf'], stdout=f)
        s2 = time.time()
        probe_time += s2 -s
        with open('sus.out', 'r') as f:
            lines = f.readlines()
            if len(lines) > n_eq_classes[N]:
                print(f'isolator not perfect! {len(lines)} matching graphs found')
            for line in lines:
                if 'ERROR' in line:
                    print(f'found a bug: experiment {seed}')
        with open('sus.cnf', 'r') as f:
            lines = f.readlines()
            n_clauses = len(lines)
            examples[n_clauses] = (seed, lines)
            n_clauses_counts[n_clauses] += 1
        io_time += time.time() - s2
        if num_experiments >= 10 and (i+1) % (num_experiments//10) == 0:
            print(f'finished experiment {i+1}/{num_experiments}, average time per experiment {(io_time + probe_time)/(i+1)}')
    output_current_isos(n_clauses_counts, probe_time, io_time)

