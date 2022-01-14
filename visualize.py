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

def from_edges(n, edges):
    edge_verts = convert_edge_ind(n)
    for e_ind in range(1, n*(n-1)//2 + 1):
        if e_ind not in edges:
            edges.append(-e_ind)

    

    edges = [edge_verts[e] for e in edges]
    g = nx.DiGraph()
    g.add_edges_from(edges)
    return g

def visualize_graphs(graphs, units, n, n_to_plot):
    N = len(graphs) if n_to_plot is None else n_to_plot
    w = int(N**0.5)
    h = w if w*w == N else w + 1

    print(units)
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
        nx.draw(g, pos2, with_labels = True, edge_color=e_colors, font_color='white')
        nx.draw_networkx_edge_labels(g, pos2, edge_labels = {e:i + 1 for i, e in enumerate(edges)}, label_pos=0.75)
    plt.show()


def main(n, iso_file, n_to_plot):
    with open(iso_file, 'r') as f:
        lines = f.readlines()

    graphs = []
    units = []
    start_clauses = False
    for line in lines:
        i = line.find(' is canonical for')
        if i >= 0:
            graphs.append(from_edges(n, [int(x) for x in line[:i].split(' ')]))
        elif 'clauses:' in line:
            start_clauses = True
        elif start_clauses:
            clause = [int(x) for x in line.split(' ')[:-1]]
            if len(clause) == 1:
                units.append(clause[0])

    visualize_graphs(graphs, units, n, n_to_plot)


if __name__ == '__main__':
    n = int(sys.argv[1])
    iso_file = sys.argv[2]
    n_to_plot = None if len(sys.argv) <=3 else int(sys.argv[3])
    main(n, iso_file, n_to_plot)