import math
import util
import sys
from nlogn_units import known_ramsey_nums_cond, naive_ramsey_cond

def make_big_iso(N, use_iso_at):
    ret = []
    remaining = N
    while remaining > use_iso_at:
        TT_verts = known_ramsey_nums_cond(remaining)
        #print('r',remaining, 'v', TT_verts)
        start = remaining - TT_verts + 1
        for src in range(start, start + TT_verts - 1):
            for snk in range(src + 1, start + TT_verts):
                print(src, snk)
                ret.append([util.edge2ind((src,snk))])
        remaining -= TT_verts
    curr_iso = util.CNF(ret)
    if remaining > 2:
        remaining_iso = util.isolator_clauses(remaining)
        #print('iso with', remaining)
        curr_iso += remaining_iso.clauses
    return curr_iso

def compare_remaining_classes():
    for n in range(3, 8):
        map_graphs = util.map_graphs(n)
        perfect = util.isolator_clauses(n)
        ramsey_TT = make_big_iso(n,1)
        perfect_filt = util.filter_graphs(map_graphs, perfect)
        ramsey_filt = util.filter_graphs(map_graphs, ramsey_TT)
        print(f'{n} vertices. perfect iso: {len(perfect_filt)},  TT-fix iso: {len(ramsey_filt)}')

def compare_large_n():
    seq = [2, 4, 12, 56, 456, 6880, 191536, 9733056, 903753248, 154108311168, 48542114686912, 28401423719122304, 31021002160355166848, 63530415842308265100288, 244912778438520759443245824, 1783398846284777975419600287232, 24605641171260376770598003978281472]
    print('max n', len(seq) + 2)
    trips = []
    for i, elem in enumerate(seq):
        n = i + 3
        n_units = len(make_big_iso(n,1))
        log_n_graphs = ((n*(n-1))//2 - n_units)
        trips.append((n, log_n_graphs, int(math.log(elem,2))))
    print('log_graphs_after_TT_fix(n)')
    print(''.join([f'({x},{y})' for x, y, _ in trips]))
    print()
    print()
    print('log_num_eq_classes(n)')
    print(''.join([f'({x},{y})' for x, _, y in trips]))


if __name__ == '__main__':
    n = int(sys.argv[1])
    use_iso_at = 8 if len(sys.argv) <=2 else int(sys.argv[2])
    use_iso_at = max([1, use_iso_at])
    #print(make_big_iso(n, use_iso_at))
    #compare_remaining_classes()
    compare_large_n()