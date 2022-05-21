import sys
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np

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

alphabet = ' abcdefghijklmnopqrstuvwxyz'

def plot_graph(g, n, units, arrowsize=25, use_edge_labels=True):
    edges= list(g.edges())
    e_colors = [('red' if (edge2ind(e) in units or -edge2ind(e) in units) else 'black') for e in edges]
    edges.sort(key=lambda e: edge2ind(e))

    pos = nx.circular_layout(g, scale=0.1)
    pos2 = {}
    for v, v2 in zip(range(1, n+1), pos):
        pos2[v] = pos[v2]

    #print(pos)
    labels = {i : c for i, c in enumerate(alphabet) if i>0 and i <= n}
    nx.draw(g, pos2, edge_color=e_colors, font_color='white', arrowsize=arrowsize) # with_labels = True,
    nx.draw_networkx_labels(g, pos2, labels, font_color="white")
    if use_edge_labels:
        lp = 0.75 if n != 6 else 0.8
        nx.draw_networkx_edge_labels(g, pos2, edge_labels = {e:i + 1 for i, e in enumerate(edges)}, label_pos=lp)

def visualize_graphs(graphs, units, n, n_to_plot):
    N = len(graphs) if n_to_plot is None else n_to_plot
    w = int(N**0.5)
    h = w if w*w == N else w + 1

    print(h,w,units)
    SAVE_MODE=False
    for ind, g in enumerate(graphs[:N]):
        if SAVE_MODE:
            plt.figure(figsize=(2,2))
            plot_graph(g, n, units,arrowsize=40, use_edge_labels=False)
            plt.savefig(f'ramsey_{n}_{ind}.png', bbox_inches='tight', transparent=True)
        else:
            plt.subplot(h, w, ind+1)
            plot_graph(g, n, units)
    if not SAVE_MODE:
        plt.show()

def find_similar_graphs(n, all_edges):
    graphs = [sorted(add_neg_edges(n,edges), key=lambda x: abs(x)) for edges in all_edges]
    #graphs = sorted(graphs, key=lambda y: sum([x > 0 for x in y]))
    # there's probably a cool alg for doing this; I'm going to brute force since n is small
    pairs = []
    stats = [0 for _ in range(len(graphs))]
    for g1_ind, g1 in enumerate(graphs):
        for g2_ind in range(g1_ind + 1, len(graphs)):
            last_neq = None
            for v1, v2 in zip(g1, graphs[g2_ind]):
                if v1 != v2:
                    if last_neq is None:
                        last_neq = abs(v1)
                    else:
                        last_neq = -1
                        break
            if last_neq is not None and last_neq != -1:
                pairs.append((g1_ind, g2_ind, last_neq))
                stats[g1_ind] += 1
                stats[g2_ind] += 1

    print(f'total equiv classes: {len(graphs)}')

    max_edges = max(stats)
    num_at_max = len([i for i in range(len(graphs)) if stats[i] == max_edges])
    print(f'max edges (i.e. "connected" eq classes): {max_edges}. A total of {num_at_max} equiv classes have this number of edges.')
    min_edges = min(stats)
    num_at_min = len([i for i in range(len(graphs)) if stats[i] == min_edges])
    print(f'min edges (i.e. "connected" eq classes): {min_edges}. A total of {num_at_min} equiv classes have this number of edges.')
    print(f'average degree: {sum(stats)/len(stats)}')
                
    return pairs

def visualize_similarity(n, all_edges):
    g = nx.DiGraph()
    sim_graphs = find_similar_graphs(n, all_edges)
    sim_g_edges = [(x, y) for x,y,_ in sim_graphs]
    g.add_edges_from(sim_g_edges)
    if n == 5:
        pos = nx.spectral_layout(g)
    else:
        pos = nx.circular_layout(g)
    pos2 = {}
    
    for v, v2 in zip(range(0, len(all_edges)), pos):
        pos2[v] = pos[v2]

    #pos[5][0] -= 0.5
    #pos[7][0] += 0.5
    #pos[5][1] += 0.1
    #pos[7][1] += 0.1
    #pos[3][0] -= 0.1
    #pos[8][0] += 0.1
    #pos[3][1] += 0.2
    #pos[8][1] += 0.2

    USE_IMAGES=False
    if USE_IMAGES:
        for i, edges in enumerate(all_edges):
            img=mpimg.imread(f'ramsey_{n}_{i}.png')
            g.add_node(i, image=img)
        fig=plt.figure(figsize=(20,20))
        ax=plt.subplot(111)
        ax.set_aspect('equal')

        plt.xlim(-1.5,1.5)
        plt.ylim(-1.5,1.5)

        trans=ax.transData.transform
        trans2=fig.transFigure.inverted().transform

        #nx.draw(g, pos2)
        nx.draw_networkx_edges(g, pos, ax=ax, arrowsize=20)
        piesize=0.1 # this is the image size
        p2=piesize/2.0
        n_graphs = len(g)
        for i,node in enumerate(g):
            xx,yy=trans(pos[node]) # figure coordinates
            xa,ya=trans2((xx,yy)) # axes coordinates
            theta = i*2*np.pi/n_graphs
            delta_x = np.cos(theta)*p2*0.8
            delta_y = np.sin(theta)*p2*0.8
            a = plt.axes([xa-p2 + delta_x,ya-p2 + delta_y, piesize, piesize])
            a.set_aspect('equal')
            a.imshow(g.nodes()[node]['image'])
            a.axis('off')
        ax.axis('off')
    else:
        nx.draw(g, pos, with_labels=True)
    print(n)
    plt.savefig(f'ramsey_{n}_flowchart.png', bbox_inches='tight')
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



    visualize_graphs(graphs, units, n, n_to_plot)
    visualize_similarity(n, all_edges)

if __name__ == '__main__':
    n = int(sys.argv[1])
    iso_file = f'isolator{n}.txt' if len(sys.argv) <=2 else sys.argv[2]
    n_to_plot = None if len(sys.argv) <=3 else int(sys.argv[3])
    #n_to_plot=1
    main(n, iso_file, n_to_plot)
