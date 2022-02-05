import sys
import os
import argparse
import util
import numpy as np
import random


def nauty_R(bits):
    #print(bits)
    #bit_str = ''.join(format(x, 'b') for x in bits)
    bit_str = ''.join([''.join(x) for x in bits])

    if len(bit_str) % 6 != 0:
        bit_str += '0'*(6 - len(bit_str)%6)
    #print(bit_str)
    return [chr(int(bit_str[6*i:6*(i+1)], 2) + 63) for i in range(len(bit_str)//6)]

def nauty_R_inv(nauty_str, n):
    bit_str = ''
    # first two characters are & and chr(63+n)
    for c in nauty_str[2:]:
        bit_str += format(ord(c) - 63, '06b')
    adj_mat = [['0' for _ in range(n)] for _ in range(n)]
    ctr = 0
    for r in range(n):
        for c in range(n):
            adj_mat[r][c] = bit_str[ctr]
            ctr += 1
    return adj_mat

def test_nauty_funcs(n):
    n_edges = n*(n-1)//2
    for g in get_all_binary_graphs(n_edges):
        adj_mat = graph_from_int(str(g),n)
        nauty_str = '&&' + ''.join(nauty_R(adj_mat))
        adj_mat2 = nauty_R_inv(nauty_str, n)
        if adj_mat != adj_mat2:
            print(adj_mat)
            print()
            print(adj_mat2)
            assert adj_mat == adj_mat2
    print('done')

def bin_int(x, n_edges):
    return format(x,'0{}b'.format(n_edges))[::-1]

def matches_units_pred(g_bin_int, units):
    if units[0] > 0:
        return all([g_bin_int[unit - 1] == '1' for unit in units])
    else:
        return not any([g_bin_int[-unit - 1] == '1' for unit in units])

def get_all_binary_graphs(n_edges, units=[]):
    assert all([unit > 0 for unit in units]) or all([unit < 0 for unit in units])
    return [bin_int(x, n_edges) for x in range(2**n_edges) if matches_units_pred(bin_int(x, n_edges), units)]

def graph_from_int(g_int, n):
    ctr = 0
    adj_mat = [['0' for _ in range(n)] for _ in range(n)]
    for end in range(1, n):
        for start in range(end):
            val = g_int[ctr]
            ctr += 1
            if val == '1':
                adj_mat[start][end] = '1'
            elif val == '0':
                adj_mat[end][start] = '1'
    return adj_mat

def edges_from_graph(adj_mat):
    n = len(adj_mat)
    ctr = 1
    edges = []
    for end in range(1, n):
        for start in range(end):
            if adj_mat[start][end] == '1':
                edges.append(ctr)
            else:
                assert adj_mat[end][start] == '1'
                edges.append(-ctr)
            ctr += 1
    return edges

def adj_mat_from_edges(edges, n):
    adj_mat = np.array([['0' for _ in range(n)] for _ in range(n)])
    ctr = 1
    for end in range(1, n):
        for start in range(end):
            if ctr in edges:
                adj_mat[start][end] = '1'
            else:
                # implicitly -ctr in edges, but we allow for edges to contain only positives
                adj_mat[end][start] = '1'
            ctr += 1
    return adj_mat


def gen_graphs(n, units):
    n_edges = n*(n-1)//2
    graphs = get_all_binary_graphs(n_edges, units)
    adj_mats = []
    for g in graphs:
        adj_mat = graph_from_int(g, n)
        adj_mats.append('&{}'.format(chr(63+n)) + ''.join(nauty_R(adj_mat)) + '\n')
    return adj_mats

def produce_map_file(n, all_tourney_name, canon_tourney_name, fname, units):
    #with open(all_tourney_name, 'r') as atf:
    #    all_tourneys = atf.readlines()
    with open(canon_tourney_name, 'r') as ctf:
        canon_tourneys = ctf.readlines()
    n_edges = n*(n-1)//2
    graphs = get_all_binary_graphs(n_edges, units)
    canon_indices = {}
    results = []
    new_label = 1

    # g is the graph (represented by an int from 0 to 2^E - 1 as a string in binary), canon is the nauty string rep of g's canonical equivalent
    for g, canon in zip(graphs, canon_tourneys):
        edges = []
        for i in range(len(g)):
            if g[i] == '1':
                edges.append(str(i+1))
        edges.append('0')
        if canon in canon_indices:
            c_label = canon_indices[canon]
        else:
            canon_indices[canon] = new_label
            c_label = new_label
            new_label += 1
        results.append('{}\t{}\n'.format(c_label, ' '.join(edges)))
    with open(fname, 'w') as f:
        f.writelines(results)

def find_hamiltonian_path(adj_mat):
    def edge_from_to(src, dest):
        return adj_mat[src][dest] == '1'

    def build_path_up_to(curr_path, new_ind):
        if edge_from_to(new_ind, curr_path[0]):
            curr_path.insert(0, new_ind)
        elif edge_from_to(curr_path[-1], new_ind):
            curr_path.append(new_ind)
        else:
            #first edge is "down". last is "up". Find the first "up" edge
            # to know where the new index fits into the path.
            for vert_ind in range(1, len(curr_path)):
                if edge_from_to(new_ind, curr_path[vert_ind]):
                    curr_path.insert(vert_ind, new_ind)
                    break
    path = [0]
    for new_ind in range(1, len(adj_mat)):
        build_path_up_to(path, new_ind)
    return path

def permute_by(adj_mat, perm):
    return adj_mat[:, perm][perm, :]

def fix_backbone(canon_tourneys):
    n = len(canon_tourneys[0])
    ret = []
    for adj_mat in canon_tourneys:
        hpath = find_hamiltonian_path(adj_mat)
        ret.append(permute_by(adj_mat, hpath))
    return ret

def improve_canon_reps(canon_tourneys):
    return fix_backbone(canon_tourneys)

def produce_canon_reps_from_gentourng(n):
    canon_base = f'canon_base_{n}.txt'
    os.system(f'gentourng {n} -z -q -l > {canon_base}')
    with open(canon_base, 'r') as ctf:
        canon_tourneys_base_raw = ctf.readlines()
    canon_tourneys = [np.array(nauty_R_inv(nauty_tourney, n)) for nauty_tourney in canon_tourneys_base_raw]

    return canon_tourneys

def produce_canon_reps_from_map(n):
    all_graphs = util.map_graphs(n)
    n_eq_classes = {3: 2, 4:4, 5: 12, 6: 56, 7: 456, 8: 6880}
    rand_p = n_eq_classes[n]/len(all_graphs)
    canon_tourneys = [None for _ in range(n_eq_classes[n])]
    for eset in all_graphs:
        eq_class = all_graphs[eset] - 1
        if canon_tourneys[eq_class] is None:
            canon_tourneys[eq_class] = eset
        else:
            if random.random() < rand_p:
                canon_tourneys[eq_class] = eset
    return [adj_mat_from_edges(eset, n) for eset in canon_tourneys]

def produce_canon_reps_from_isolator(n):
    n_edges = n * (n-1)//2
    adj_mats = []
    with open(f'isolator{n}.txt', 'r') as f:
        for line in f.readlines():
            ind = line.find('is canonical')
            if ind != -1:
                canon_rep_raw = line[:ind].strip()
                canon_rep = [] if len(canon_rep_raw) == 0 else [int(x) for x in canon_rep_raw.split(' ')]
                adj_mats.append(adj_mat_from_edges(canon_rep, n))
    return adj_mats


def eval_canon_set(canon_tourneys):
    n_edges = len(canon_tourneys[0])
    n_classes = len(canon_tourneys)
    score = 0
    for e in range(n_edges):
        num_pos = 0
        for tourney in canon_tourneys:
            if tourney[e] > 0:
                num_pos += 1
        score += abs(n_classes/2 - num_pos)
    return score

def canon_set_to_bica_cnf(canon_tourneys_edges, n):
    n_edges = n * (n-1)//2
    #canon_tourneys_edges is a DNF; each member list is a clause 
    canon_cnf = util.dnf2cnf(canon_tourneys_edges)
    return str(canon_cnf) + f'\nc n orig vars {n_edges}'
    
def canon_set_to_bica_cnf_neg(canon_tourneys_edges):
    return str(util.CNF([[-v for v in clause] for clause in canon_tourneys_edges]))

def write_to_file(graphs, fname):
    with open(fname, 'w') as f:
        f.writelines(graphs)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='generate a map file')
    parser.add_argument('N', type=int, help='vertices in the graph')
    parser.add_argument('--use-last-units', action='store_true', help='only use graphs matching the units from the last isolator')
    parser.add_argument('--use-arbitrary-units', type=str, default=None, help='filename of units to filter by when producing map file')
    parser.add_argument('--filename', type=str, default=None, help='stub name for output files. default to stdout')
    parser.add_argument('--canon-reps', action='store_true', help='instead of a map file, generate a file of canon reps')

    args = parser.parse_args()
    n = args.N
    if args.canon_reps:

        # manual generation of arbitrary canon set from map file for when gentourng is unavailable
        #canon_tourneys = produce_canon_reps_from_map(n)

        # use nauty's gentourng tool to produce the graphs (most general approach, should work for n <=9)
        canon_tourneys = produce_canon_reps_from_gentourng(n)

        # if we have an isolator produced by decode_clauses.py, use its canon set. Mostly for testing, works for n <=6
        #canon_tourneys = produce_canon_reps_from_isolator(n)

        # do things (permutations) to make the tourneys better
        canon_tourneys = improve_canon_reps(canon_tourneys)

        canon_tourneys_edges = [edges_from_graph(adj_mat) for adj_mat in canon_tourneys]
        print(eval_canon_set(canon_tourneys_edges))
        
        canon_reps_lines = [' '.join([str(x) for x in tourney if x > 0]) + '\n' for tourney in canon_tourneys_edges]
        print(canon_reps_lines)
        write_to_file(canon_reps_lines, f'canon_reps_{n}.txt')

        cnf_str = canon_set_to_bica_cnf(canon_tourneys_edges, n)
        with open(f'bica_canon_{n}.cnf', 'w') as f:
            f.write(cnf_str)

        cnf_neg_str = canon_set_to_bica_cnf_neg(canon_tourneys_edges)
        with open(f'bica_canon_{n}_neg.cnf', 'w') as f:
            f.write(cnf_neg_str)
    else:
        fname = 'map{}.txt'.format(n) if args.filename is None else args.filename
        units = util.isolator_clauses(n - 1, only_units=True).get_units() if args.use_last_units else []
        if args.use_arbitrary_units is not None:
            with open(args.use_arbitrary_units, 'r') as f:
                lines = f.readlines()
                for line in lines[1:]:
                    units.append(int(line.split(' ')[0]))
        print('using units: ', units)
        #exit()
        graphs = gen_graphs(n, units)
        all_tourney_name = 'all_tourney{}.d6'.format(n)
        canon_tourney_name = 'canon{}.d6'.format(n)
        write_to_file(graphs, all_tourney_name)
        os.system('labelg {} -q > {}'.format(all_tourney_name, canon_tourney_name))

        produce_map_file(n, all_tourney_name, canon_tourney_name, fname, units)
