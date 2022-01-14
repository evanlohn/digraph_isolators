import sys
import os
import argparse
import util
import numpy as np


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
    return all([g_bin_int[unit - 1] == '1' for unit in units])

def get_all_binary_graphs(n_edges, units=[]):
    assert all([unit > 0 for unit in units])
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

def fix_backbone(canon_tourneys):
    n = len(canon_tourneys[0])
    for adj_mat in canon_tourneys:
        


def produce_canon_reps(n):
    canon_base = f'canon_base_{n}.txt'
    os.system(f'gentourng {n} -z -q -l > {canon_base}')
    with open(canon_base, 'r') as ctf:
        canon_tourneys_base_raw = ctf.readlines()
    canon_tourneys = [np.array(nauty_R_inv(nauty_tourney, n)) for nauty_tourney in canon_tourneys_base_raw]

    # do things to make the tourneys better

    return [edges_from_graph(adj_mat) for adj_mat in canon_tourneys]

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


def write_to_file(graphs, fname):
    with open(fname, 'w') as f:
        f.writelines(graphs)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='generate a map file')
    parser.add_argument('N', type=int, help='vertices in the graph')
    parser.add_argument('--use-last-units', action='store_true', help='only use graphs matching the units from the last isolator')
    parser.add_argument('--filename', type=str, default=None, help='stub name for output files. default to stdout')
    parser.add_argument('--canon-reps', action='store_true', help='instead of a map file, generate a file of canon reps')

    args = parser.parse_args()
    n = args.N
    if args.canon_reps:
        canon_tourneys = produce_canon_reps(n)
        print(eval_canon_set(canon_tourneys))
        canon_tourneys = [' '.join([str(x) for x in tourney if x > 0]) for tourney in canon_tourneys]
        print(canon_tourneys)
        write_to_file(canon_tourneys, f'canon_reps_{n}.txt')
    else:
        fname = 'map{}.txt'.format(n) if args.filename is None else args.filename
        units = util.isolator_clauses(n - 1, only_units=True).get_units() if args.use_last_units else []
        print(units)
        graphs = gen_graphs(n, units)
        all_tourney_name = 'all_tourney{}.d6'.format(n)
        canon_tourney_name = 'canon{}.d6'.format(n)
        write_to_file(graphs, all_tourney_name)
        os.system('labelg {} -q > {}'.format(all_tourney_name, canon_tourney_name))

        produce_map_file(n, all_tourney_name, canon_tourney_name, fname, units)
