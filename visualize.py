import sys
import networkx as nx
import matplotlib.pyplot as plt

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

def add_neg_edges(n, edges):
    ret = edges[:]
    for e_ind in range(1, n*(n-1)//2 + 1):
        if e_ind not in edges:
            ret.append(-e_ind)
    return ret

def from_edges(n, edges):
    edge_verts = convert_edge_ind(n)
    edges = add_neg_edges(n, edges)

    edges = [edge_verts[e] for e in edges]
    g = nx.DiGraph()
    g.add_edges_from(edges)
    return g

def visualize_graphs(graphs, units, n, n_to_plot):
    N = len(graphs) if n_to_plot is None else n_to_plot
    w = int(N**0.5)
    h = w if w*w == N else w + 1

    print(h,w,units)
    for ind, g in enumerate(graphs[:N]):
        edges= list(g.edges())
        e_colors = [('red' if (edge2ind(e) in units or -edge2ind(e) in units) else 'black') for e in edges]
        edges.sort(key=lambda e: edge2ind(e))

        plt.subplot(h, w, ind+1)
        pos = nx.circular_layout(g)
        pos2 = {}
        for v, v2 in zip(range(1, n+1), pos):
            pos2[v] = pos[v2]

        #print(pos)
        nx.draw(g, pos2, with_labels = True, edge_color=e_colors, font_color='white', arrowsize=20)
        nx.draw_networkx_edge_labels(g, pos2, edge_labels = {e:i + 1 for i, e in enumerate(edges)}, label_pos=0.75)
    plt.show()

def find_similar_graphs(n, all_edges):
    graphs = [sorted(add_neg_edges(n,edges), key=lambda x: abs(x)) for edges in all_edges]

    # there's probably a cool alg for doing this; I'm going to brute force since n is small
    pairs = []
    for g1_ind, g1 in enumerate(graphs):
        for g2_ind in range(g1_ind + 1, len(graphs)):
            if sum([v1!=v2 for v1, v2 in zip(g1, graphs[g2_ind])]) == 1:
                pairs.append((g1_ind, g2_ind))
    return pairs

def visualize_similarity(n, all_edges):
    g = nx.Graph()
    g.add_edges_from(find_similar_graphs(n, all_edges))
    pos = nx.spring_layout(g)
    pos2 = {}
    for v, v2 in zip(range(0, len(all_edges)), pos):
        pos2[v] = pos[v2]
    #nx.draw(g, pos2)
    nx.draw(g)
    plt.show()

def main(n, iso_file, n_to_plot):
    with open(iso_file, 'r') as f:
        lines = f.readlines()

    graphs = []
    units = []
    start_clauses = False
    all_edges = []
    for line in lines:
        i = line.find(' is canonical for')
        if i >= 0:
            edges = [int(x) for x in line[:i].split(' ')]
            all_edges.append(edges)
            graphs.append(from_edges(n, edges))
        elif 'clauses:' in line:
            start_clauses = True
        elif start_clauses:
            clause = [int(x) for x in line.split(' ')[:-1]]
            if len(clause) == 1:
                units.append(clause[0])

    #for i, edges in enumerate(all_edges):
    #    print(i, edges)
    #print(find_similar_graphs(n, all_edges))



    #visualize_graphs(graphs, units, n, n_to_plot)
    visualize_similarity(n, all_edges)

if __name__ == '__main__':
    n = int(sys.argv[1])
    iso_file = f'isolator{n}.txt' if len(sys.argv) <=2 else sys.argv[2]
    n_to_plot = None if len(sys.argv) <=3 else int(sys.argv[3])
    main(n, iso_file, n_to_plot)
