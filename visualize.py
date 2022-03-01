import sys
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

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

def plot_graph(g, n, units):
    edges= list(g.edges())
    e_colors = [('red' if (edge2ind(e) in units or -edge2ind(e) in units) else 'black') for e in edges]
    edges.sort(key=lambda e: edge2ind(e))

    pos = nx.circular_layout(g)
    pos2 = {}
    for v, v2 in zip(range(1, n+1), pos):
        pos2[v] = pos[v2]

    #print(pos)
    nx.draw(g, pos2, with_labels = True, edge_color=e_colors, font_color='white', arrowsize=25)
    nx.draw_networkx_edge_labels(g, pos2, edge_labels = {e:i + 1 for i, e in enumerate(edges)}, label_pos=0.75)

def visualize_graphs(graphs, units, n, n_to_plot):
    N = len(graphs) if n_to_plot is None else n_to_plot
    w = int(N**0.5)
    h = w if w*w == N else w + 1

    print(h,w,units)
    for ind, g in enumerate(graphs[:N]):
        #plt.subplot(h, w, ind+1)
        plot_graph(g, n, units)
        plt.show()

def find_similar_graphs(n, all_edges):
    graphs = sorted([sorted(add_neg_edges(n,edges), key=lambda x: abs(x)) for edges in all_edges], key=lambda y: sum([x > 0 for x in y]))

    # there's probably a cool alg for doing this; I'm going to brute force since n is small
    pairs = []
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
                
    return pairs

def visualize_similarity(n, all_edges):
    g = nx.DiGraph()
    sim_graphs = find_similar_graphs(n, all_edges)
    sim_g_edges = [(x, y) for x,y,_ in sim_graphs]
    g.add_edges_from(sim_g_edges)
    pos = nx.spring_layout(g)
    pos2 = {}
    
    for v, v2 in zip(range(0, len(all_edges)), pos):
        pos2[v] = pos[v2]

    USE_IMAGES=False
    if USE_IMAGES:
        for i, edges in enumerate(all_edges):
            img=mpimg.imread(f'ramsey_{n}_{i}.png')
            g.add_node(i, image=img)
        fig=plt.figure(figsize=(5,5))
        ax=plt.subplot(111)
        ax.set_aspect('equal')

        plt.xlim(-1.5,1.5)
        plt.ylim(-1.5,1.5)

        trans=ax.transData.transform
        trans2=fig.transFigure.inverted().transform

        #nx.draw(g, pos2)
        nx.draw_networkx_edges(g, pos, ax=ax)
        piesize=0.05 # this is the image size
        p2=piesize/2.0
        for n in g:
            xx,yy=trans(pos[n]) # figure coordinates
            xa,ya=trans2((xx,yy)) # axes coordinates
            a = plt.axes([xa-p2,ya-p2, piesize, piesize])
            a.set_aspect('equal')
            a.imshow(g.nodes()[n]['image'])
            a.axis('off')
        ax.axis('off')
    else:
        nx.draw(g, pos)
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
