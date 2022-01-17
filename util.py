import sys

def isolator_clauses(n, only_units=False):
    with open(f"isolator{n}.txt") as f:
        cs = []
        in_clauses = False
        for line in f:
            if in_clauses:
                c = [int(x) for x in line.split()[:-1]]
                if only_units and len(c) > 1:
                    continue
                cs += [c]
            else: in_clauses = line.strip() == "clauses:"
    return CNF(cs)

def map_graphs(n, mapname=None):
    if mapname is None:
        mapname = f"map{n}.txt"
    with open(mapname) as f:
        data = [[int(x) for x in line.split()] for line in f]
    graphs = {frozenset(line[1:-1]): line[0] for line in data}
    return graphs

def filter_graphs(graphs, clauses, do_print=True):
    prev_classes = set(graphs.values())
    filt_graphs = {g: graphs[g] for g in graphs if clauses.satisfies(Indexer(lambda i: i in g))}
    filt_classes = set(filt_graphs.values())
    if len(prev_classes) != len(filt_classes):
        if do_print:
            print(f'using given isolator deletes equivalence classes:')
            for c in prev_classes:
                if c in filt_classes:
                    continue
                for g in graphs:
                    if graphs[g] == c and g not in filt_graphs:
                        print(f'class {c}, example graph {list(g)}')
                        break
        assert False
    return filt_graphs

from collections import defaultdict


# potential name collisions but this doesn't need to be perfect
class CNF:
    def __init__(self, clauses = None, var = None):
        self.var = defaultdict(self.next_literal)
        if var is not None:
            for k,v in var.items():
                self.var[k] = v
        self.clauses = [] if clauses is None else clauses
        self.unit = None
    def next_literal(self):
        return len(self.var)+1
    def fresh(self):
        return self.var["fresh", len(self.var)]
    def __getitem__(self, key):
        return self.var[key]
    def __iadd__(self, clauses):
        self.add_all(clauses)
        return self
    def __len__(self):
        return len(self.clauses)

    def non_units(self):
        return len([x for x in self.clauses if len(x) > 1])

    def get_units(self):
        return sum([ c for c in self.clauses if len(c) == 1], [])
        
    def true(self):
        if self.unit is None:
            self.unit = self.var["unit"]
            self.clauses += [[self.unit]]
        return self.unit
    def add(self, clause):
        self.clauses.append(list(clause))
    def add_all(self, clauses):
        for clause in clauses:
            self.add(clause)
    def naive_unitprop(self):
        asn = {}
        last_len = -1
        cnf = self.clauses
        while len(asn) > last_len:
            try:
                unit = next(c[0] for c in cnf if len(c) == 1)
            except:
                break
            asn[abs(unit)] = unit > 0
            cnf = [[l for l in c if l != -unit] for c in cnf if unit not in c]
        if [] in cnf:
            return CNF([[]], self.var)
        cnf = cnf + [[l] if v else [-l] for l,v in asn.items()]
        return CNF(cnf, self.var)
    def atmostn(self, n, lits):
        lits = list(lits)
        dp = [[self.fresh() for _ in range(n+1)] for _ in lits]
        cs = []
        for i,x in enumerate(lits):
            for j in range(n+1):
                if i: cs += [[-dp[i-1][j], dp[i][j]]]
                if not j: cs += [[-x, dp[i][j]]]
                elif i: cs += [[-dp[i-1][j-1], -x, dp[i][j]]]
        cs += [[-dp[-1][n]]]
        return cs
    def satisfies(self, assignment):
        return all(
            any(assignment[abs(lit)] == (lit > 0) for lit in clause)
            for clause in self.clauses
        )
    def __str__(self):
        nvars = max([0] + [abs(lit) for clause in self.clauses for lit in clause])
        header = f"p cnf {nvars} {len(self.clauses)}"
        #comments = [f"c {k} => {v}" for k,v in self.var.items()]
        comments = []
        clauses = [" ".join(str(x) for x in clause + [0]) for clause in self.clauses]
        return "\n".join([header]+comments+clauses)

class Indexer:
    def __init__(self, fcn):
        self.fcn = fcn
    def __getitem__(self, key):
        return self.fcn(key)

def convert_edge_ind(n):
    ctr = 0
    ret = {}
    for end in range(1, n):
        for start in range(end):
            ctr += 1
            ret[ctr] = (start+1, end+1)
            ret[-ctr] = (end+1, start+1)
    return ret

def edge2ind(e):
    v, w = e
    if (v > w):
        v, w = w, v

    return (w - 2) * (w - 1) // 2 + v

def from_edge_inds(n, edges):
    edge_verts = convert_edge_ind(n)
    edges = list(edges)
    for e_ind in range(1, n*(n-1)//2 + 1):
        if e_ind not in edges:
            edges.append(-e_ind)

    

    return [edge_verts[e] for e in edges]

def print_degree_counts(graphs, n):
    counts = defaultdict(lambda: (0, set()))
    max_iso_class = 1
    for graph in graphs:
        edges = from_edge_inds(n, graph)
        local_degrees = defaultdict(lambda: 0)
        for source, _ in edges:
            local_degrees[source] += 1
        key = defaultdict(lambda: 0)
        for i in range(1, n+1):
            key[local_degrees[i]] += 1
        key = [(k, v) for k, v in key.items()]
        key.sort(key=lambda pair: pair[0])
        key = tuple(key)
        #print(key)
        
        n_seen, iso_classes = counts[key]
        iso_classes.add(graphs[graph])
        counts[key] = (n_seen + 1, iso_classes)
        max_iso_class = max(max_iso_class, graphs[graph])
    print(f'num degree classes: {len(counts)}')
    print(f'num iso classes: {max_iso_class}')

    sus_classes = 0
    for deg_class in counts:
        seen, classes = counts[deg_class]
        if len(classes) > 1:
            sus_classes += 1
            #print(deg_class, seen, len(classes))
            print(f'{seen} graphs in degree class, containing {len(classes)} isomorphism classes')
    print(f'{sus_classes} degree classes contain multiple iso classes')

def count_degrees():
    map_file = sys.argv[1]
    n = int(map_file[3])
    graphs = map_graphs(n, mapname=map_file)
    print_degree_counts(graphs, n)

def filter_map_file():
    map_file = sys.argv[1]
    filter_cnf = sys.argv[2]
    fcnf = CNF()

    with open(filter_cnf, 'r') as f:
        lines = f.readlines()[1:]
        fcnf += [[int(x) for x in line.strip().split(' ')[:-1]] for line in lines]
    #fcnf += [[17]]
    n = int(map_file[3])
    graphs = map_graphs(n, mapname=map_file)
    


    gs = filter_graphs(graphs, fcnf)
    print(len(gs))
    for g in gs:
        print(f'{list(g)} is canonical for class {gs[g]}')
    #print(fcnf)

if __name__ == '__main__':
    # filter_map_file()
    count_degrees()

