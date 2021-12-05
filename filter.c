#include <stdio.h>
#include <stdlib.h>

#define ALLOC   10000000

#define POS(c,e)   ((c) * 2 * nEdge + (e) + 1)
#define NEG(c,e)   ((c) * 2 * nEdge + (e) + 1 + nEdge)
#define DEAD(g)    ((g) + deadStart + 1)
#define ALIVE(c,g) ((c) + (g) * nCls + aliveStart + 1)

#define ORDER
#define REDUNDANT
//#define SORT

//#define BREAK
//#define CHAIN
//#define FIRSTCHAIN
//#define PARTIAL

//#define BASESIX

int deadStart, aliveStart;

int nCls;
int nNode, nEdge, nGraph, nClass;
int allone;

int *mask, *eqcl;
int *count;

int filter (int posMask, int negMask) {
  int i;
  int removed = 0;

  //printf("filter %i %i\n", posMask, negMask);
  //CHANGE: shouldn't we be filtering out graphs that _don't_ satisfy the clause?

  for (i = 0; i < nGraph; i++) {
    int a = mask[i] & posMask;
    int b = (mask[i] ^ allone) & negMask;
    int c = eqcl[i];
    if ((a == posMask) && (b == negMask)) {
      nGraph--;
      int tmp = mask[i];
      mask[i] = mask[nGraph];
      mask[nGraph] = tmp;
      eqcl[i] = eqcl[nGraph];
      eqcl[nGraph] = c;
      count[c]--;
      if (count[c] == 0) {
        printf ("c class %i is eliminated\n", c);
        return -1; // ERROR!!!
      }
      i--; } }

  return removed; }

int main (int argc, char** argv) {
  int *thisClass, size;
  int *minSub;

  int i, j, k;

  nGraph = 0;
  nClass = 0;
  nEdge  = 0;

  eqcl = (int*) malloc (sizeof(int) * ALLOC);
  mask = (int*) malloc (sizeof(int) * ALLOC);

  FILE *map;
  map = fopen(argv[1], "r");

  int nat, cClass, cMask;
  int first = 1;
  while (1) {
    int tmp = fscanf (map, " %i ", &nat);
    if (tmp == EOF) break;

    if (first == 1) {
      cMask  = 0;
      cClass = nat;
      if (nat > nClass) nClass = nat; }
    else if (nat > 0) {
      cMask |= 1 << (nat - 1);
      if (nat > nEdge) nEdge = nat; }

    first = 0;
    if (nat == 0) {
      eqcl[nGraph] = cClass;
      mask[nGraph] = cMask;
      nGraph++;
      first = 1; } }

  fclose(map);

  allone = (1 << nEdge) - 1;

                   nNode = 0;
  if (nEdge ==  3) nNode = 3;
  if (nEdge ==  6) nNode = 4;
  if (nEdge == 10) nNode = 5;
  if (nEdge == 15) nNode = 6;

  count  = (int*) malloc (sizeof(int) * (nClass+1));

  for (i = 1; i <= nClass; i++) count[i] = 0;
  for (i = 0; i <  nGraph; i++) count[eqcl[i]]++;

  if ((nEdge ==  6) && (nClass !=   4)) { printf("ERROR: not all classes are present\n"); exit (0); }
  if ((nEdge == 10) && (nClass !=   12)) { printf("ERROR: not all classes are present\n"); exit (0); }
  if ((nEdge == 15) && (nClass !=  56)) { printf("ERROR: not all classes are present\n"); exit (0); }
  if ((nEdge == 21) && (nClass != 456)) { printf("ERROR: not all classes are present\n"); exit (0); }

  //CHANGE: Double free??
  //fclose(map);

//  printf("c finished parsing map: %i graphs\n", nGraph);

  FILE *cnf;
  cnf = fopen (argv[2], "r");

  int lit, a, b;
  int posMask = 0;
  int negMask = 0;
  int tmp = fscanf(cnf, " p cnf %i %i ", &a, &b);
  while (1) {
    tmp = fscanf (cnf, " %i ", &lit);
    if (tmp == EOF) break;

    if (lit > 0) posMask |= 1 << (lit - 1);

    if (lit < 0) negMask |= 1 << (-lit - 1);

    if (lit == 0) {
      int removed = filter (posMask, negMask);
      if (removed == -1) {
        printf("ERROR: at least one class was eliminated\n");
        exit(0); }
      posMask = negMask = 0; } }

  fclose (cnf);

  for (i = 0; i < nGraph; i++) {
    printf("%i ", eqcl[i]);
    for (j = 0; j < nEdge; j++) {
      if (mask[i] & (1 << j))
        printf("%i ", j + 1); }
    printf("0\n"); }
}