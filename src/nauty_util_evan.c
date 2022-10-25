#include "nauty_util_evan.h"

void printSet(int n, set *s) {
    for (int v2 = 0; v2 < n; v2 ++){
        printf(ISELEMENT(s, v2)?"1 ":"0 ");
    }
    printf("\n");
}

void printPerm(int n, int *p) {
    for (int v = 0; v < n; v++){
        printf("%d ", p[v]);
    }
    printf("\n");
}

void printGraph(int n, graph *g) {
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  for (int v = 0; v < n; v++) {
    printSet(n, GRAPHROW(g,v,m));
  }
}

/*
returns the out-degree of vertex `vert` in the `n`-vertex tournament `g` when restricted to the vertices in `verts`
*/
int outDegree(int n, graph *g, set *verts, int vert) {
  int ct = 0;
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  set *row = GRAPHROW(g,vert,m);
  for (int v = 0; v < n; v++) {
    if (ISELEMENT(row, v) && ISELEMENT(verts, v)) {
      ct++;
    }
  }
  return ct;
}

bool graphEqual(int n, graph *g1, graph *g2) {
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  for (int v = 0; v < m*n; v++) {
    if (g1[v] != g2[v]) {
      return false;
    }
  }
  
  return true;
}

// get the canonical form of each graph, check if they're equal
// see find_ST25.c for examples that use 'lab' to extract the permutations to/from the canon form
bool graphsIso(int n, graph *g1, graph *g2) {
  static DEFAULTOPTIONS_DIGRAPH(options);
  statsblk stats;
  options.getcanon=TRUE;
  options.writeautoms=FALSE;
  int lab[n],ptn[n],orbits[n];
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  
  graph canon_g1[n*m];
  graph canon_g2[n*m];
  densenauty(g1, lab, ptn, orbits, &options, &stats, m, n, canon_g1);
  densenauty(g2, lab, ptn, orbits, &options, &stats, m, n, canon_g2);
  return graphEqual(n, canon_g1, canon_g2);

}

// of the graphs are isomorphic, lab will contain the permutation that sends g1 to g2
// untested, might need to invert the other one or compose in the other order...
bool graphsIsoPerm(int n, graph *g1, graph *g2, int *lab) {
  static DEFAULTOPTIONS_DIGRAPH(options);
  statsblk stats;
  options.getcanon=TRUE;
  options.writeautoms=FALSE;
  int lab1[n],lab2[n],ptn[n],orbits[n];
  
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  graph canon_g1[n*m];
  graph canon_g2[n*m];
  densenauty(g1, lab1, ptn, orbits, &options, &stats, m, n, canon_g1);
  densenauty(g2, lab1, ptn, orbits, &options, &stats, m, n, canon_g2);
  
  int lab1_inv[n];
  invertPerm(n,lab1, lab1_inv);
  composePerms(n, lab1_inv, lab2, lab);
  
  return graphEqual(n, canon_g1, canon_g2);
}


int factorial(int x) {
  int ret = 1;
  for (int i = 1; i <= x; i++) {
    ret = ret * i;
  }
  return ret;
}

int countLines(char *fname) {
  FILE *input = fopen(fname, "r");
  int lines = 0;
  while (!feof(input)) {
    if (fgetc(input) == '\n') {
      lines++;
    }
  }
  fclose(input);
  return lines;
}

/*
A bit of a misnomer; stores a single graph at `graphs[0]` from a DIMACs output file.
*/
void read_cnf(int n, char *fname, graph **graphs) {
  graph *g = graphs[0];
  int num_edges = n * (n-1) / 2;
  int *edges = malloc((num_edges+1)*2 * sizeof(int));

  int tmp = 1;
  for (int end = 1; end < n; end++) {
    for (int start = 0; start < end; start++) {
      edges[2*tmp] = start;
      edges[2*tmp+1] = end;
      tmp++;
    }
  }

  FILE * input = fopen(fname, "r");

  int ct = 0;
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  EMPTYGRAPH(g,m,n);
  char * graphline = (char *) malloc (LINE_LENGTH * sizeof(char));
  int edges_added = 0;
  int ch_read = 0;
  bool is_pos = true;
  char *buf = malloc (BUFSIZE * sizeof(char));
  while(1)
  {
    size_t x = 0;
    if(getline(&graphline, &x, input) == -1)
    {
        break;
    }

    if(graphline[0] != 'v')
        continue;

    for (int i = 0; i<BUFSIZE;i++) {
      buf[i] = '\0';
    }

    ch_read = 0;
    is_pos = true;
    x = strlen(graphline);
    for(int l = 2; l < x; l++)
    {
        char ch = graphline[l];
        if ((ch == '\n') || (ch == ' ')) {
          if (ch_read > 0) {
            int edge_ind = atoi(buf);
            // do stuff
            if (edge_ind <= num_edges && edge_ind > 0) { // goes from 1 to num_edges
              if (is_pos) {
                ADDONEARC(g,edges[2*edge_ind], edges[2*edge_ind+1], m);
              } else {
                ADDONEARC(g,edges[2*edge_ind+1], edges[2*edge_ind], m);
              }
              edges_added++;
            }
          }

          //cleanup
          ch_read = 0;
          is_pos = true;
          for (int i = 0; i<BUFSIZE;i++) {
            buf[i] = '\0';
          }
          
        } else if (ch == '-') {
          is_pos = false;
        } else {
          buf[ch_read++] = ch;
        }
    }
    
    if (ch_read != 0) {
      fprintf(stderr, "int of len %d remaining in buffer '%s', line len %ld \n", ch_read, buf, strlen(graphline));
    }
    ct++;
  }
  if (edges_added != num_edges) {
    fprintf(stderr, "expected %d edges, got %d\n", num_edges, edges_added);
  }
  free(buf);
  free(graphline);
  free(edges);
  fclose(input);
}

/* print the d6 string of a graph g; doesn't work for n >= 64 since the graph size characters at the start are wrong */
void print_d6(int n, graph *g) {
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  printf("&%c",(char)(n+63));
  int bit_ind = 0;
  int curr = 0;
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < n; col++) {
      if (ISELEMENT(GRAPHROW(g,row,m), col)) {
        curr = curr | (1 << (5-bit_ind));
      } 
      bit_ind++;
      if (bit_ind == 6) {
        bit_ind = 0;
        printf("%c",(char)(curr + 63));
        curr = 0;
      }
    }
  }
  if (bit_ind != 0) {
    printf("%c",(char)(curr + 63));
  }
  printf("\n");
}

/* read a graph in from a d6 string; doesn't work for n >= 64 since the graph size characters at the start are wrong */
graph* read_d6_str(char *graphline, int *n_verts) {
  if(graphline[0] != '&')
      fprintf(stderr, "ERROR: non-& first char\n");
  
  int n =graphline[1] - 63;
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  graph *g = malloc(sizeof(graph) *m*n);

  EMPTYGRAPH(g,m,n);
  int edge_ind = 0;
  for(int l = 2; l < strlen(graphline); l++)
  {
      char ch = graphline[l];
      if (ch == '\n') {
          break;
      }
      for (int i = 5; i >= 0; i--) {
          if (edge_ind == n*n) { //stop when you reach the end of the graph
            if (l != strlen(graphline) - 1) {
              fprintf(stderr, "did not use all characters. %d characters produced the expected %d edges\n", l, edge_ind);
            }
            continue;
          }
          if (((1<<i) & (ch-63)) != 0) {
              ADDONEARC(g,edge_ind/n, edge_ind%n, m);
          }
          edge_ind++;
      }
  }
  if (edge_ind != n*n) {
    fprintf(stderr, "incorrect number of edges; expected %d got %d\n", n*n, edge_ind);
  }
  *n_verts = n;
  return g;
}

/* reads and stores all tournaments from the d6 file named `fname` into `graphs[0]`, `graphs[1]`, etc. use countLines on the file to know the size of the resulting malloc'ed memory */
void read_d6(int n, char *fname, graph **graphs) {
  char * graphline = (char *) malloc (LINE_LENGTH * sizeof(char));
  FILE * input = fopen(fname, "r");

  int ct = 0;
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  while(1)
  {
    size_t x;
    if(getline(&graphline, &x, input) == -1)
    {
        break;
    }

    if(graphline[0] != '&')
        fprintf(stderr, "ERROR: non-& first char at line %d\n", ct);
    
    if (graphline[1] - 63 != n) {
        fprintf(stderr, "ERROR: graph size %d\n", graphline[1]);
        break;
    }

    EMPTYGRAPH(graphs[ct],m,n);
    // *x is > size of text in line. \n denotes the last char
    int edge_ind = 0;
    for(int l = 2; l < x; l++)
    {
        char ch = graphline[l];
        if (ch == '\n') {
            break;
        }
        for (int i = 5; i >= 0; i--) {
            if (((1<<i) & (ch-63)) != 0) {
                ADDONEARC(graphs[ct],edge_ind/n, edge_ind%n, m);
            }
            edge_ind++;
        }
    }
    ct++;
  }
  free(graphline);
  fclose(input);
}


/* Store a tournament in `output` that is the subtournament of the `n`-vertex tournament `g` over the vertices specified in `verts`. */
void makeSubGraph(int n, graph *g, set *verts, graph *output) {
  int vmap[n];
  int ctr = 0;
  for (int v = 0; v < n; v++) {
    vmap[v] = (ISELEMENT(verts, v))? ctr++ : -1;
  }

  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  int m2 = SETWORDSNEEDED(ctr);
  nauty_check(WORDSIZE,m2,ctr,NAUTYVERSIONID);
  for (int v = 0; v < n-1; v++) {
    set *row = GRAPHROW(g, v, m);
    for (int v2 = v + 1; v2 < n; v2++) {
      if (ISELEMENT(verts,v) && ISELEMENT(verts, v2)) {
        if (ISELEMENT(row, v2)) {
          ADDONEARC(output,vmap[v],vmap[v2],m2);
        } else {
          ADDONEARC(output,vmap[v2],vmap[v],m2);
        }
      }
    }
  }
}

// lab is a permutation on the indices of the elements of verts.
// basically, we want the vertices in verts to be permuted by lab in output
// if verts is the set of all vertices, this permutes the entire graph
void permuteSubGraph(int n, graph *g, set *verts, int* lab, graph *output) {
  int vmap[n];
  int vmap_inv[n];
  for (int v = 0; v < n; v++) {
    vmap_inv[v] = -1;
  }
  int ctr = 0;
  for (int v = 0; v < n; v++) {
    if (ISELEMENT(verts, v)) {
      vmap[v] = ctr++;
      vmap_inv[ctr-1] = v; 
    } else {
      vmap[v] = -1;
    }
  }
  // vmap goes from full graph vertex to permutation ind

  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  int m2 = SETWORDSNEEDED(ctr);
  nauty_check(WORDSIZE,m2,ctr,NAUTYVERSIONID);
  for (int v = 0; v < n-1; v++) {
    int v_new = (ISELEMENT(verts, v))? vmap_inv[lab[vmap[v]]] : v;
    set *row = GRAPHROW(g, v, m);
    for (int v2 = v + 1; v2 < n; v2++) {
      int v2_new = (ISELEMENT(verts, v2))? vmap_inv[lab[vmap[v2]]] : v2;
      if (ISELEMENT(row, v2)) {
        ADDONEARC(output,v_new,v2_new,m2);
      } else {
        ADDONEARC(output,v2_new,v_new,m2);
      }
    }
  }
}

void invertPerm(int n, int* lab, int *inv_lab) {
  for (int i = 0; i < n; i++) {
    inv_lab[lab[i]] = i;
  }
}

void composePerms(int n, int* lab1, int *lab2, int *comp_lab) {
  for (int i = 0; i < n; i++) {
    comp_lab[i] = lab1[lab2[i]];
  }
}

