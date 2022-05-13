#include "nauty_util_evan.h"

//NOTE: to do arbitrary graph isomorphisms, you should change MAXN in the util file to the size of the graphs you care about.
//      Due to the way nauty's data structures work you can probably get away with leaving MAXN at 33 as long as
//      your graph has <=64 vertices, but I wouldn't recommend it. for >64 vertices, DEFINITELY change MAXN. You might
//      also need to explicitly pass in the calculated m in some places (I got lazy and used MAXM everywhere), but 
//      honestly that isn't too likely.

//crash course on NAUTY data structures; all Nauty types are actually just aliases for long ints; use their macros
// after defining arrays of the right size. m (MAXM) is the number of long ints needed to have enough bits to store a row
// of an adjacency matrix for graphs on n (MAXN) vertices.


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
