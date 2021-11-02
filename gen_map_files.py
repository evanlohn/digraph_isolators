import sys
import os


def nauty_R(bits):
    #print(bits)
    #bit_str = ''.join(format(x, 'b') for x in bits)
    bit_str = ''.join([''.join(x) for x in bits])

    if len(bit_str) % 6 != 0:
        bit_str += '0'*(6 - len(bit_str)%6)
    #print(bit_str)
    return [chr(int(bit_str[6*i:6*(i+1)], 2) + 63) for i in range(len(bit_str)//6)]



def gen_graphs(n):
    n_edges = n*(n-1)//2
    graphs = [format(x,'0{}b'.format(n_edges))[::-1] for x in range(2**n_edges)]
    adj_mats = []
    for g in graphs:
        ctr = 0
        adj_mat = [['0' for _ in range(n)] for _ in range(n)]
        for end in range(1, n):
            for start in range(end):
                val = g[ctr]
                ctr += 1
                if val == '1':
                    adj_mat[start][end] = '1'
                elif val == '0':
                    adj_mat[end][start] = '1'


        adj_mats.append('&{}'.format(chr(63+n)) + ''.join(nauty_R(adj_mat)) + '\n')
    return adj_mats

def produce_map_file(n, all_tourney_name, canon_tourney_name, fname):
    #with open(all_tourney_name, 'r') as atf:
    #    all_tourneys = atf.readlines()
    with open(canon_tourney_name, 'r') as ctf:
        canon_tourneys = ctf.readlines()
    n_edges = n*(n-1)//2
    graphs = [format(x,'0{}b'.format(n_edges))[::-1] for x in range(2**n_edges)]
    canon_indices = {}
    results = []
    new_label = 1
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




def write_to_file(graphs, fname):
    with open(fname, 'w') as f:
        f.writelines(graphs)


if __name__ == '__main__':
    n = int(sys.argv[1])
    fname = 'map{}.txt'.format(n) if len(sys.argv) <=2 else sys.argv[2]
    graphs = gen_graphs(n)
    all_tourney_name = 'all_tourney{}.d6'.format(n)
    canon_tourney_name = 'canon{}.d6'.format(n)
    write_to_file(graphs, all_tourney_name)
    os.system('labelg {} -q > {}'.format(all_tourney_name, canon_tourney_name))

    produce_map_file(n, all_tourney_name, canon_tourney_name, fname)
