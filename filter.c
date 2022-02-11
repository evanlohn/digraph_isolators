#include <stdio.h>
#include <stdlib.h>

#define ALLOC   400000000

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

long deadStart, aliveStart;

long nCls;
long nNode, nEdge, nGraph, nClass;
long allone;

long *mask, *eqcl;
long *count;

long filter (long posMask, long negMask) {
  long i;
  long removed = 0;

  //printf("filter %i %i\n", posMask, negMask);
  //CHANGE: shouldn't we be filtering out graphs that _don't_ satisfy the clause?

  for (i = 0; i < nGraph; i++) {
    long a = mask[i] & posMask;
    long b = (mask[i] ^ allone) & negMask;
    long c = eqcl[i];
    if (i==0 && posMask==1 && negMask==4194306) {
    //if (0) {
      printf("beginning filter for graph %ld. Equiv class %ld, with %ld remaining in eq class.\n", i, c, count[c]);
      printf("graph mask: %lx \n", mask[i]);
      printf("positive clauses mask: %lx \n", posMask);
      printf("bitwise AND of graph and pos clauses (matching posMask necessary for removal): %lx \n", a);
      printf("\n");
      printf("allone: %lx nEdges: %ld \n", allone, nEdge);
      printf("graph ^ allone: %lx \n", mask[i] ^ allone);
      printf("negative clauses mask: %ld \n", negMask);
      printf("bitwise AND of last two (matching negMask necessary for removal): %lx \n", b);
      printf("\n\n");
    }
    if ((a == 0) && (b == 0)) {
      nGraph--;
      long tmp = mask[i];
      mask[i] = mask[nGraph];
      mask[nGraph] = tmp;
      eqcl[i] = eqcl[nGraph];
      eqcl[nGraph] = c;
      count[c]--;
      if (count[c] == 0) {
        printf ("c class %ld is eliminated\n", c);
        return -1; // ERROR!!!
      }
      i--; } }

  return removed; }

long main (long argc, char** argv) {
  long *thisClass, size;
  long *minSub;

  long i, j, k;

  nGraph = 0;
  nClass = 0;
  nEdge  = 0;

  eqcl = (long*) malloc (sizeof(long) * ALLOC);
  mask = (long*) malloc (sizeof(long) * ALLOC);

  FILE *map;
  map = fopen(argv[1], "r");

  long nat, cClass, cMask;
  long first = 1;
  while (1) {
    long tmp = fscanf (map, " %ld ", &nat);
    if (tmp == EOF) break;

    if (first == 1) {
      cMask  = 0;
      cClass = nat;
      if (nat > nClass) nClass = nat; }
    else if (nat > 0) {
      cMask |= (long)(1) << (nat - 1);
      if (nat > nEdge) nEdge = nat; }

    first = 0;
    if (nat == 0) {
      eqcl[nGraph] = cClass;
      mask[nGraph] = cMask;
      nGraph++;
      first = 1; } }

  fclose(map);

  allone = ((long)(1) << nEdge) - 1;

                   nNode = 0;
  if (nEdge ==  3) nNode = 3;
  if (nEdge ==  6) nNode = 4;
  if (nEdge == 10) nNode = 5;
  if (nEdge == 15) nNode = 6;
  if (nEdge == 21) nNode = 7;

  count  = (long*) malloc (sizeof(long) * (nClass+1));

  for (i = 1; i <= nClass; i++) count[i] = 0;
  for (i = 0; i <  nGraph; i++) count[eqcl[i]]++;

  if ((nEdge ==  6) && (nClass !=   4)) { printf("ERROR: not all classes are present\n"); exit (0); }
  if ((nEdge == 10) && (nClass !=   12)) { printf("ERROR: not all classes are present\n"); exit (0); }
  if ((nEdge == 15) && (nClass !=  56)) { printf("ERROR: not all classes are present\n"); exit (0); }
  if ((nEdge == 21) && (nClass != 456)) { printf("ERROR: not all classes are present\n"); exit (0); }
  if ((nEdge == 28) && (nClass != 6880)) { printf("ERROR: not all classes are present\n"); exit (0); }

  //CHANGE: Double free??
  //fclose(map);

//  printf("c finished parsing map: %i graphs\n", nGraph);

  FILE *cnf;
  cnf = fopen (argv[2], "r");

  long lit, a, b;
  long posMask = 0;
  long negMask = 0;
  long tmp = fscanf(cnf, " p cnf %ld %ld ", &a, &b);
  //int meh = 0;
  while (1) {
    tmp = fscanf (cnf, " %ld ", &lit);
    if (tmp == EOF) break;

    if (lit > 0) posMask |= (long)(1) << (lit - 1);

    if (lit < 0) negMask |= (long)(1) << (-lit - 1);


    if (lit == 0) {
      long removed = filter (posMask, negMask);
      /*
      if (meh == 0) {
	printf("removed: %ld\n", removed);
	meh = 1;
      }*/
      if (removed == -1) {
        printf("ERROR: at least one class was eliminated\n");
        exit(0); }
      posMask = negMask = 0; } }

  fclose (cnf);

  for (i = 0; i < nGraph; i++) {
    printf("%ld ", eqcl[i]);
    for (j = 0; j < nEdge; j++) {
      if (mask[i] & ((long)(1) << j))
        printf("%ld ", j + 1); }
    printf("0\n"); }
}
