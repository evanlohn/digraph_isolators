#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <assert.h>
#define MAXN 33
#include "nautinv.h"
#include "naututil.h"

#define LINE_LENGTH 256
#define BUFSIZE 8

//NOTE: to do arbitrary graph isomorphisms, you should change MAXN above to the size of the graphs you care about.
//      Due to the way nauty's data structures work you can probably get away with leaving MAXN at 33 as long as
//      your graph has <=64 vertices, but I wouldn't recommend it. for >64 vertices, DEFINITELY change MAXN. You might
//      also need to explicitly pass in the calculated m in some places (I got lazy and used MAXM everywhere), but 
//      honestly that isn't too likely.

//crash course on NAUTY data structures; all Nauty types are actually just aliases for long ints; use their macros
// after defining arrays of the right size. m (MAXM) is the number of long ints needed to have enough bits to store a row
// of an adjacency matrix for graphs on n (MAXN) vertices.

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
    for (int v = 0; v < n; v++) {
        printSet(n, GRAPHROW(g,v,MAXM));
        //fprintf(stderr, "%d: %lx\n", v, [0]);
    }
}

// need to take verts into account
int outDegree(int n, graph *g, set *verts, int vert) {
  int ct = 0;
  set *row = GRAPHROW(g,vert,MAXM);
  for (int v = 0; v < n; v++) {
    if (ISELEMENT(row, v) && ISELEMENT(verts, v)) {
      ct++;
    }
  }
  return ct;
}

// only works for MAXM=1, which is true when MAXN <= 64
//to fix, add an inner loop over M
bool graphEqual(int n, graph *g1, graph *g2) {
    for (int v = 0; v < n; v++) {
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
  
  graph canon_g1[n*MAXM];
  graph canon_g2[n*MAXM];
  densenauty(g1, lab, ptn, orbits, &options, &stats, MAXM, n, canon_g1);
  densenauty(g2, lab, ptn, orbits, &options, &stats, MAXM, n, canon_g2);
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
  return lines;
}

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
  //fprintf(stderr, "WS: %d\n", WORDSIZE);
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
        //fprintf(stderr, "breaking %lu\n", strlen(graphline));
        break;
    }
    //printf("%lu\n", x);

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
          //printf("yoinks %d\n", ch_read);
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
}

void print_d6(int n, graph *g) {
  printf("&%c",(char)(n+63));
  int bit_ind = 0;
  int curr = 0;
  for (int row = 0; row < n; row++) {
    for (int col = 0; col < n; col++) {
      if (ISELEMENT(GRAPHROW(g,row,MAXM), col)) {
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

void read_d6(int n, char *fname, graph **graphs) {
  char * graphline = (char *) malloc (LINE_LENGTH * sizeof(char));
  FILE * input = fopen(fname, "r");

  int ct = 0;
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  //fprintf(stderr, "WS: %d\n", WORDSIZE);
  while(1)
  {
    size_t x;
    if(getline(&graphline, &x, input) == -1)
    {
        //fprintf(stderr, "breaking %lu\n", strlen(graphline));
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
        //if((ch- 63) >> 6 != 0) printf("error on %d %d\n", graphline[l], l) ;
        //printf("ch: %x\n", ch-63);
        for (int i = 5; i >= 0; i--) {
            //printf("%x\n", (1<<i) & (ch-63));
            if (((1<<i) & (ch-63)) != 0) {
                ADDONEARC(graphs[ct],edge_ind/n, edge_ind%n, m);
                //fprintf(stderr,"verts: %d %d\n", edge_ind/n, edge_ind%n);
            }
            edge_ind++;
        }
    }
    ct++;
  }
  //fprintf(stderr, "read %d graphs\n", ct);
  free(graphline);
  fclose(input);
}

void fileIsoCheck(int n, char *fname1, char *format1, char *fname2, char *format2) {
  //supported formats: d6, cnf

  int lines1 = countLines(fname1);
  bool g1d6 = (strcmp(format1, "d6") == 0);
  int numGraphs1 = g1d6 ? lines1 : 1;
  graph **graphs1 = malloc (numGraphs1 * sizeof(graph *));
  for (int g_id = 0; g_id < numGraphs1; g_id++) {
    graphs1[g_id] = malloc(n*MAXM*sizeof(graph));
  }

  if (g1d6) {
    read_d6(n, fname1, graphs1);
  } else {
    read_cnf(n, fname1, graphs1);
  }


  int lines2 = countLines(fname2);
  bool g2d6 = (strcmp(format2, "d6") == 0);
  int numGraphs2 = g2d6 ? lines2 : 1;
  graph **graphs2 = malloc (numGraphs2 * sizeof(graph *));
  for (int g_id = 0; g_id < numGraphs2; g_id++) {
    graphs2[g_id] = malloc(n*MAXM*sizeof(graph));
  }

  if (g2d6) {
    read_d6(n, fname2, graphs2);
  } else {
    read_cnf(n, fname2, graphs2);
  }

  int numIso1[numGraphs1];
  int numIso2[numGraphs2];
  for (int g_id = 0; g_id < numGraphs2; g_id++) {
    numIso2[g_id] = 0;
  }
  for (int g1 = 0; g1 < numGraphs1; g1++) {
    numIso1[g1] = 0;
    for (int g2 = 0; g2 < numGraphs2; g2++) {
      if (graphsIso(n, graphs1[g1], graphs2[g2])) {
        numIso1[g1] = numIso1[g1] + 1;
        numIso2[g2] = numIso2[g2] + 1;
      }
    }
  }



  for (int g_id = 0; g_id < numGraphs1; g_id++) {
    if (numIso1[g_id] > 0) {
      //printf("graph %d from file 1 isomorphic to %d graph(s) from file 2\n", g_id, numIso1[g_id]);
    }
    free(graphs1[g_id]);
  }
  free(graphs1);

  for (int g_id = 0; g_id < numGraphs2; g_id++) {
    //if (numIso2[g_id] > 0) {
      //printf("graph %d from file 2 isomorphic to %d graph(s) from file 1\n", g_id, numIso2[g_id]);
    //}
    if (numIso2[g_id] == 0) {
      printf ("graph %d in file %s non-isomorphic to graphs from file 1\n", g_id, fname2);
      print_d6(n, graphs2[g_id]);
    }
    free(graphs2[g_id]);
  }
  free(graphs2);
  //printf("iso checking done\n");

}

int main (int argc, char** argv) {
  if (argc != 6) {
    printf("expected 5 arguments (n, inpfile1, format1, inpfile2, format2), got %d", argc);
    return 1;
  }

  int n = atoi (argv[1]);
  fileIsoCheck(n, argv[2], argv[3], argv[4], argv[5]);
}
