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

void printSet(int n, set *s);

void printPerm(int n, int *p);

void printGraph(int n, graph *g);

int outDegree(int n, graph *g, set *verts, int vert);

bool graphEqual(int n, graph *g1, graph *g2);

// get the canonical form of each graph, check if they're equal
// see find_ST25.c for examples that use 'lab' to extract the permutations to/from the canon form
bool graphsIso(int n, graph *g1, graph *g2) ;

int factorial(int x);

int countLines(char *fname);

void read_cnf(int n, char *fname, graph **graphs);

void print_d6(int n, graph *g);

graph* read_d6_str(char *graphline, int *n_verts);

void read_d6(int n, char *fname, graph **graphs); 

void makeSubGraph(int n, graph *g, set *verts, graph *output);

// lab is a permutation on the indices of the elements of verts.
// basically, we want the vertices in verts to be permuted by lab in output
// if verts is the set of all vertices, this permutes the entire graph
void permuteSubGraph(int n, graph *g, set *verts, int* lab, graph *output);

void invertPerm(int n, int* lab, int *inv_lab);

void composePerms(int n, int* lab1, int *lab2, int *comp_lab);

