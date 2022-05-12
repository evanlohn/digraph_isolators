import math
import util
import sys
from nlogn_units import known_ramsey_nums_cond, naive_ramsey_cond

def make_big_iso(N, use_iso_at):
    ret = []
    remaining = N
    mat = [[0 for _ in range(N)] for _ in range(N)]
    while remaining > use_iso_at:
        TT_verts = known_ramsey_nums_cond(remaining)
        #print('r',remaining, 'v', TT_verts)
        start = remaining - TT_verts + 1
        for src in range(start, start + TT_verts - 1):
            for snk in range(src + 1, start + TT_verts):
                #print(src, snk)
                ret.append([util.edge2ind((src,snk))])
                mat[src-1][snk-1] = 1
        remaining -= TT_verts
    for row in mat:
        print(row)
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
    print()
    print()
    print('log(total labeled graphs)')
    print(''.join([f'({x},{x*(x-1)//2})' for x, _, y in trips]))

def get_neighbors(row, col, n):
    ret = []
    if row > 0:
        ret.append((row-1, col))
    if col > 0:
        ret.append((row, col-1))
    if row < n - 1:
        ret.append((row+1, col))
    if col < n - 1:
        ret.append((row, col+1))
    return ret

def encode_no_islands(fresh_var, n):
    # main idea: distance from known peninsula
    # (0,0) is distance 0 from the peninsula (it is the peninsula)
    #      side note: for efficiency it might actually be better to use (i,i).
    # (i,j) is distance x from peninsula exactly when some (i+-1,j+-1) is distance x-1 and both are zero
    # (edge cases for literal edges and diagonal of the adjmat)

    # for all cells C, if C=1 then C is no distance away from the peninsula

    D = n*n
    d_vars = {}
    for row in range(n):
        for col in range(n):
            for dist in range(D):
                d_vars[(row,col,dist)] =fresh_var()

    ret = [[d_vars[(0,0,0)]]] # (0,0) is distance 0
    for dist in range(1,D):
        ret.append([-d_vars[(0,0,dist)]]) # (0,0) is not any farther distance away
    for row in range(n): 
        for col in range(n):
            for dist in range(row+col): # nothing else is distance 0, or distance < manhattan distance
                ret.append([-d_vars[(row, col, dist)]])

            # at least one distance variable must hold per zero cell
            # note that util.edge2ind switches the sign for us when row > col
            switch = [] if row == col else [util.edge2ind((row,col))]
            ret.append(switch + [d_vars[row,col, dist] for dist in range(row+col,D)])

            # no distance variable can hold for 1-cells
            if row != col:
                for dist in range(row+col,D):
                    ret.append([-util.edge2ind((row,col))] + [d_vars[row,col, dist]])

            if row + col > 0:
                for dist in range(row+col, D): 
                    # if a distance variable holds, one of its surrounding lesser distance variables must hold
                    ret.append([-d_vars[row, col, dist]] + [d_vars[rp, cp, dist-1] for rp, cp in get_neighbors(row, col, n)])
    return ret

def encode_no_islands_simple(fresh_var, n, upto):
    ret = []
    for row in range(upto): 
        for col in range(upto):
            coords = [(col,row)]
            if col < n-1:
                coords.append((row, col+1))
            if row > 0:
                coords.append((row-1, col))
            ret.append([util.edge2ind(coord) for coord in coords])
            ret.append([util.edge2ind((col,row)) for row, col in coords])
    return ret

if __name__ == '__main__':
    n = int(sys.argv[1])
    use_iso_at = 8 if len(sys.argv) <=2 else int(sys.argv[2])
    use_iso_at = max([1, use_iso_at])
    #print(make_big_iso(n, use_iso_at))
    #compare_remaining_classes()
    compare_large_n()