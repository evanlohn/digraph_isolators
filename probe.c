#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// this code is a slightly modified version of https://github.com/marijnheule/isolator/blob/master/probe_orig.c

#define ALLOC     10000000
#define MAXMAX       10000
#define LISTSIZE        30
#define PRIME     16777619

#define CLSMAX        1000

#define RAND

//#define BINARY

//#define REP

//#define POPCOUNT

#define HASH

// #define COMMENTS

#define CLAUSES

//#define CANON

//#define MAXLENGTH

int max, *maxList, maxDepth;
int nEdge, nGraph, nClass, nNode;
int allone;

unsigned int roundSeed;

unsigned int *bloom;

int *mask, *eqcl, *out, **implied, *canon;
int *count, *red_count;
int *rep;
int *cover;

struct clause {
  unsigned int posMask;
  unsigned int negMask;
  unsigned int subsumed; };

struct clause* cList;

int activeClauses, allocatedClauses;

int pcount, ecount, ncount;

int Min, Tie;
unsigned int *listPos, *listNeg, *listTie, *listMin;

unsigned int maskHash (unsigned int pos, unsigned int neg) {
  return pos * PRIME + neg; }

unsigned int edgeMask (int v, int w) {
  int a = v, b = w;
  if (v > w) { a = w; b = v; }

  //CHANGE 1: use different edge numbering
  int i = (w - 2) * (w - 1) / 2 + v; 

  return (unsigned int) (1 << i); }

void setCanon (unsigned int gMask) {
  int g;

  for (g = 0; g < nGraph; g++) {
    if (mask[g] == gMask) {
      canon[g] = 1;
      return; } }

  printf("c WARNING did not find canon graph %u\n", gMask); }

// num pos edges?
int popCount (int mask) {
  int pcount = 0;
  while (mask) {
    if (mask & 1) pcount++;
    mask = mask / 2; }
  return pcount; }

// modifies nGraph, mask, canon, eqcl, count to reflect the removal
// of all graphs matching (all of) a particular posMask and negmask.
// errors if an equivalence class loses all its members
int filter (int posMask, int negMask) {
  int i;
  int removed = 0;

  // mask holds masks for each graph
  // eqcl holds the equivalence class of each graph
  for (i = 0; i < nGraph; i++) {
    int a = mask[i] & posMask; 
    int b = (mask[i] ^ allone) & negMask;
    int c = eqcl[i];
    if ((a == posMask) && (b == negMask)) { 
      // if the current mask (mask[i]) has all the positives from posMask
      //    and all the negatives (zeros) from negmask:...
      nGraph--;
      int tmp = mask[i];
      mask[i] = mask[nGraph];
      mask[nGraph] = tmp;
      tmp = canon[i];
      canon[i] = canon[nGraph];
      canon[nGraph] = tmp;
      eqcl[i] = eqcl[nGraph];
      eqcl[nGraph] = c;
      count[c]--;
      removed++;
      if (count[c] == 0) return -1; // ERROR!!!
      i--; } }

  return removed; }

void printClause (int posMask, int negMask) {
  int i;

  for (i = nEdge; i > 0; i--)
    if (posMask & (1 << (i-1)))
      printf("%i ", i);

  for (i = 1; i <= nEdge; i++)
    if (negMask & (1 << (i-1)))
      printf("-%i ", i);

  printf("0\n"); }

// same as filter, but returns a count of the number of graphs matching
// the clauses instead of removing them from the data structures.
int seive (int posMask, int negMask) {
  int i, removed = 0;
  for (i = 1; i <= nClass; i++) red_count[i] = 0;

  for (i = 0; i < nGraph; i++) {
    int a = mask[i] & posMask;
    int b = (mask[i] ^ allone) & negMask;
    int c = eqcl[i];
    if ((a == posMask) && (b == negMask)) {
      removed++; red_count[c]++;
      if (count[c] == red_count[c]) {
        return -1;
        int tmp = mask[i];
        mask[i] = mask[0];
        mask[0] = tmp;
        tmp = canon[i];
        canon[i] = canon[0];
        canon[0] = tmp;
        eqcl[i] = eqcl[0];
        eqcl[0] = c;
        return -1; } } }

  return removed; }

static unsigned long long cover_updates = 0;
static unsigned long long cover_hits = 0;

// the ith cover is the & of all graphs sharing edge i, except the cover doesnt include i.
void updateCover ( ) {
  cover_updates++;
  int i, j;
  for (i = 0; i < nEdge; i++) {
    cover[i] = allone ^ (1 << i);
    for (j = 0; j < nGraph; j++) {
      if ((mask[j] & (1 << i)))
        cover[i] &= mask[j]; } } }

// reshuffle sthe graphs so the ones not admitted by the masks are at the end.
// returns the index of the first excluded graph.
int moveActive (int posMask, int negMask) {
  int i, out = 0;

  for (i = 0; i < nGraph - out; i++) {
    int a = mask[i] & posMask;
    int b = (mask[i] ^ allone) & negMask;
    if ((a == posMask) && (b == negMask)) continue;
    out++;
    int tmp = eqcl[i];
    eqcl[i] = eqcl[nGraph - out];
    eqcl[nGraph - out] = tmp;
    tmp = mask[i];
    mask[i] = mask[nGraph - out];
    mask[nGraph - out] = tmp;
    tmp = canon[i];
    canon[i] = canon[nGraph - out];
    canon[nGraph - out] = tmp;
    i--; }

  return nGraph - out; }

int cycle (int posMask, int negMask) {
#ifdef REP
  int i;
  int pos = 0, neg = 0;

  for (i = 1; i <= nEdge; i++)
    if (posMask & (1 << (i - 1))) {
      if (pos) return 0; // no binary clause
      pos = i; }

  for (i = 1; i <= nEdge; i++)
    if (negMask & (1 << (i - 1))) {
      if (neg) return 0; // no binary clause
      neg = i; }

  if (rep[pos] == rep[neg]) return 1;
#endif
  return 0;
}

void updateRep (unsigned int posMask, unsigned int negMask) {
#ifdef REP
  int i;
  int pos = 0, neg = 0;

  for (i = 1; i <= nEdge; i++)
    if (posMask & (1 << (i - 1))) {
      if (pos) return; // no binary clause
      pos = rep[i]; }

  for (i = 1; i <= nEdge; i++)
    if (negMask & (1 << (i - 1))) {
      if (neg) return; // no binary clause
      neg = rep[i]; }

  for (i = 1; i <= nEdge; i++)
    if (rep[i] == neg)  {
//      printf("c changing rep[%i] = %i\n", i, pos);
      rep[i] = pos;
   }
#endif
}

void replace (unsigned int posMask, unsigned int negMask, unsigned int min, unsigned int tie) {
  int i;
  for (i = 0; i < LISTSIZE; i++) {
    if ((listMin[i] == Min) && (listTie[i] == Tie)) {
      Min = min + 1;
      listPos[i] = posMask; listNeg[i] = negMask;
      listMin[i] = min;     listTie[i] = tie; break; } }

  for (i = 0; i < LISTSIZE; i++)
    if (listMin[i] < Min) Min = listMin[i], Tie = listTie[i];

  for (i = 0; i < LISTSIZE; i++)
    if ((listMin[i] == Min) && (listTie[i] < Tie)) Tie = listTie[i];
}

unsigned int getHash () {
  unsigned int hash = roundSeed;

  int i;
  for (i = 1; i <= nClass; i++) {
    hash *= PRIME;
    hash ^= (unsigned int) (count[i] - red_count[i]); }

  return hash; }

void addClause (unsigned int posMask, unsigned int negMask) {
  if (activeClauses == allocatedClauses) {
    allocatedClauses *= 2;
    cList = (struct clause*) realloc (cList, sizeof (struct clause) * allocatedClauses);
    bloom = (unsigned  int*) realloc (bloom, sizeof (unsigned  int) * allocatedClauses);
  }

  cList[activeClauses].posMask = posMask;
  cList[activeClauses].negMask = negMask;

  activeClauses++;
}

void makeClauseRec (int start, int posMask, int negMask, int depth) {
  int i;
  if (posMask & negMask) return; // skip tautologies

  if (depth == 0) {
    //CHANGE: allow positive units
    if (posMask == 0) return;
    //if (negMask == 0) return;
    int removed;
    for (i = 0; i < nEdge; i++) {
      if ((posMask & (1 << i)) && (negMask & cover[i])) {
        cover_hits++;
        return; } }

    // CHANGE: removed sus if
    //if (popCount (posMask) + popCount (negMask) <= 3) removed = 1;
    removed = seive (posMask, negMask);


    if (removed <  0) ncount++;
    if (removed == 0) ecount++;
    if (removed >  0) pcount++;
    if (removed > 0)
      addClause (posMask, negMask);
  }
  else {
    int _nGraph = nGraph;
    nGraph = moveActive (posMask, negMask);
    for (i = start; i <= 2 * nEdge + 1; i++) {
      int l = (i >> 1);
      if (i & 1) l *= -1;

      if (l < 0) makeClauseRec (i + 1, posMask, negMask | (1 << (abs(l)-1)), depth-1);
      else       makeClauseRec (i + 1, posMask | (1 << (abs(l)-1)), negMask, depth-1);
    }
    nGraph = _nGraph;
  }
}

void evaluateClauses ( ) {
  int i, j, k, count = 0;

  for (i = 0; i < activeClauses; i++) {
    cList[i].subsumed = 0;
    bloom[i]          = 0; }

  k = 0;
  for (i = 0; i < activeClauses; i++) {
    unsigned int posMask = cList[i].posMask;
    unsigned int negMask = cList[i].negMask;
    unsigned int h, hi, hb;

    for (j = 0; j < nEdge; j++) {
      if (posMask & (1 << j)) {
        h = maskHash (posMask ^ (1 << j), negMask) % (32 * activeClauses);
        hi = h / 32; hb = h % 32;
        if (bloom[hi] & (1 << hb)) cList[i].subsumed = 1; } }

    for (j = 0; j < nEdge; j++) {
      if (negMask & (1 << j)) {
        h = maskHash (posMask, negMask ^ (1 << j)) % (32 * activeClauses);
        hi = h / 32; hb = h % 32;
        if (bloom[hi] & (1 << hb)) cList[i].subsumed = 1; } }

    if (cList[i].subsumed) {
      h = maskHash (posMask, negMask) % (32 * activeClauses);
      hi = h / 32; hb = h % 32;
      bloom[hi] |= (1 << hb);
      count++;
      cList[k].posMask = posMask;
      cList[k].negMask = negMask;
      k++;
    }

    else {
      int removed = seive (posMask, negMask);

      if (removed > 0) {
        h = maskHash (posMask, negMask) % (32 * activeClauses);
        hi = h / 32; hb = h % 32;
        bloom[hi] |= (1 << hb);
        cList[k].posMask = posMask;
        cList[k].negMask = negMask;
        k++;

        if (removed >= Min) {
          unsigned int tie;
          tie = getHash ();
          for (j = 0; j < LISTSIZE; j++)
            if ((listMin[j] == removed) && (listTie[j] == tie))
              removed = 0;
          if (cycle (posMask, negMask)) removed = 0;
          if (removed > Min)
            replace (posMask, negMask, removed, tie);
          else if ((removed == Min) && (tie > Tie))
            replace (posMask, negMask, removed, tie); }
      } } }

#ifdef COMMENTS
  printf("c active clauses reduced from %i to %i (%i)\n", i, k, count);
#endif
  activeClauses = k;
  if (activeClauses == 0) {
    maxDepth++;
    ncount = ecount = pcount = 0;
    updateCover ();
    makeClauseRec (2, 0, 0, maxDepth);
#ifdef COMMENTS
    printf("c counts: %i %i %i %i (sum %i)\n", maxDepth, ncount, ecount, pcount, ncount + ecount + pcount);
#endif
    evaluateClauses ();

  }

}

int main (int argc, char** argv) {
  int tmp;
  int nat;
  int first;
  int nClause = 0;

  int i, j;
  /* int k; */

  nGraph = 0;
  nClass = 0;
  nEdge  = 0;

  eqcl = (int*) malloc (sizeof(int) * ALLOC);
  mask = (int*) malloc (sizeof(int) * ALLOC);

  int cClass, cMask;

  FILE* input;

  int nLit = 0;

  input = fopen(argv[1], "r");
  int seed = 123456;
  if (argc > 2) seed = atoi(argv[2]);
  srand(seed);

  first = 1;
  while (1) {    tmp = fscanf (input, " %i ", &nat);

    if (tmp == EOF) break;

    if (first == 1) {
      cMask  = 0;
      cClass = nat;
      if (nat > nClass) nClass = nat;
    }
    else if (nat > 0) {
      cMask |= 1 << (nat - 1);
      if (nat > nEdge) nEdge = nat;
    }

    first = 0;
    if (nat == 0) {
      eqcl[nGraph] = cClass;
      mask[nGraph] = cMask;
      nGraph++;
      first = 1;
    }
  }

  allone = (1 << nEdge) - 1;

  count      = (int*) malloc (sizeof(int) * (nClass+1));
  red_count  = (int*) malloc (sizeof(int) * (nClass+1));

  for (i = 1; i <= nClass; i++) count[i] = 0;
  for (i = 0; i <  nGraph; i++) count[eqcl[i]]++;

  // CHANGE: num classes per nEdges
  if ((nEdge ==  6) && (nClass !=   4)) { printf("ERROR: not all classes present\n"); goto end; }
  if ((nEdge == 10) && (nClass !=   12)) { printf("ERROR: not all classes present\n"); goto end; }
  if ((nEdge == 15) && (nClass !=  56)) { printf("ERROR: not all classes present\n"); goto end; }
  if ((nEdge == 21) && (nClass != 456)) { printf("ERROR: not all classes present\n"); goto end; }
  
  if (nEdge ==  6) nNode = 4;
  if (nEdge == 10) nNode = 5;
  if (nEdge == 15) nNode = 6;
  if (nEdge == 21) nNode = 7;

  fclose(input);

  cover = (int*) malloc (sizeof(int) * nEdge);
  for (i = 0; i < nEdge; i++) cover[i] = 0;

  maxList = (int*) malloc (sizeof(int) * MAXMAX);

  canon = (int*) malloc (sizeof(int) * nGraph);
  for (i = 0; i < nGraph; i++) canon[i] = 0;

  out = (int*) malloc (sizeof (int) * (nEdge + 1));
  for (i = 1; i <= nEdge; i++) out [i] = 0;
/*
  implied = (int**) malloc (sizeof (int*) * (nEdge + 1));
  for (i = 1; i <= nEdge; i++) implied[i] = malloc (sizeof(int) * (nEdge + 1));

  for (i = 1; i <= nEdge; i++)
    for (j = 1; j <= nEdge; j++)
      implied[i][j] = 0;

  for (i = 1; i <= nEdge; i++)
    for (j = 1; j <= nEdge; j++) {
      if (out[i]) continue;
      if (out[j]) continue;
      if (i == j) continue;
      int flag = 1;
      int mm = (1 << (i - 1)) | (1 << (j-1));
      for (k = 0; k < nGraph; k++) {
        int m = mask[k] & mm;
        if (m == (1 << (j-1))) { flag = 0; break; } }

      if (flag) {
//        printf("c edge %i is implied by edge %i\n", i, j);
        implied[i][j] = 1; } }
*/
#ifdef CANON
  if (nNode >= 3) {
    setCanon (edgeMask (1, 2));
    setCanon (edgeMask (1, nNode) | edgeMask (2, nNode)); }

  if (nNode >= 4) {
    setCanon (edgeMask (1, 2) | edgeMask (3, 4));
    setCanon (edgeMask (3, 4) | edgeMask (1, nNode) | edgeMask (2, nNode));
    setCanon (edgeMask (1, 2) | edgeMask (1, nNode) | edgeMask (2, nNode));
    setCanon (edgeMask (1, 2) | edgeMask (3, 4) | edgeMask (1, nNode) | edgeMask (2, nNode));
    setCanon (edgeMask (1, 2) | edgeMask (3, 4) | edgeMask (2, 3));
    setCanon (edgeMask (1, 2) | edgeMask (3, 4) | edgeMask (2, 3) | edgeMask (1,4));
    setCanon (edgeMask (1, 2) | edgeMask (3, 4) | edgeMask (2, 3) | edgeMask (1,4) | edgeMask (2,4));
    setCanon (edgeMask (1, 2) | edgeMask (3, 4) | edgeMask (2, 3) | edgeMask (1,4) | edgeMask (2,4) | edgeMask (1,3));
  }
#endif

#ifdef REP
  rep = (int*) malloc (sizeof(int) * (nEdge + 1));
  for (i = 1; i <= nEdge; i++) rep[i] = i;
#endif

  listPos  = (unsigned int*) malloc (sizeof(unsigned int) * LISTSIZE);
  listNeg  = (unsigned int*) malloc (sizeof(unsigned int) * LISTSIZE);
  listTie  = (unsigned int*) malloc (sizeof(unsigned int) * LISTSIZE);
  listMin  = (unsigned int*) malloc (sizeof(unsigned int) * LISTSIZE);

/* int maxi; */

  activeClauses = 0;
  allocatedClauses = 10;
  cList = (struct clause*) malloc (sizeof (struct clause) * allocatedClauses);
  bloom = (unsigned  int*) malloc (sizeof (unsigned  int) * allocatedClauses);


  ncount = ecount = pcount = 0;
  makeClauseRec(2, 0, 0, 1);
#ifdef COMMENTS
  printf("c counts: %i %i %i %i (sum %i)\n", maxDepth, ncount, ecount, pcount, ncount + ecount + pcount);
#endif

  ncount = ecount = pcount = 0;
  makeClauseRec(2, 0, 0, 2);
#ifdef COMMENTS
  printf("c counts: %i %i %i %i (sum %i)\n", maxDepth, ncount, ecount, pcount, ncount + ecount + pcount);
#endif
  ncount = ecount = pcount = 0;
  makeClauseRec(2, 0, 0, 3);
#ifdef COMMENTS
  printf("c counts: %i %i %i %i (sum %i)\n", maxDepth, ncount, ecount, pcount, ncount + ecount + pcount);
#endif
  maxDepth = 3;

while (1) {
  int flag = 1;
  int active = 0;
  for (i = 1; i <= nClass; i++) {
    active += count[i];
    if (count[i] != 1) flag = 0;
  }

  if (flag) {
    #ifdef COMMENTS
    printf("\t%i\t%i\t%i\n", nClause, nLit, seed);
    #endif
    goto end; }

#ifdef COMMENTS
  printf("c active %i\n", active);
#endif
  roundSeed = rand();

  for (j = 0; j < LISTSIZE; j++) {
    listPos[j] = listNeg[j] = listTie[j] = listMin[j] = 0; }

  Min = Tie = 0;

  roundSeed = rand();

  for (j = 0; j < LISTSIZE; j++) {
    listPos[j] = listNeg[j] = listTie[j] = listMin[j] = 0; }

  Min = Tie = 0;

  evaluateClauses();

  int r = 1;
#ifdef RAND
  r = rand();
#endif

  int posSize = LISTSIZE;
  for (i = 0; i < posSize; i++)
    if (listMin[i] == 0) {
      posSize--;
      listMin[i] = listMin[posSize];
      listTie[i] = listTie[posSize];
      listPos[i] = listPos[posSize];
      listNeg[i] = listNeg[posSize];
      i--; }


  int maxj = 0;
  for (j = 0; j < LISTSIZE; j++) {
    if (listMin[j] >  listMin[maxj]) maxj = j;
    if (listMin[j] == listMin[maxj] && listTie[j] > listTie[maxj]) maxj = j;
  }

  int p = popCount (listPos[maxj]) + popCount (listNeg[maxj]);

#ifdef MAXLENGTH
  if (maxDepth < (nNode - 1))     
#endif
  if (maxDepth == p) {
    maxDepth++;
    ncount = ecount = pcount = 0;
    updateCover ();
    makeClauseRec (2, 0, 0, maxDepth);
#ifdef COMMENTS
    printf("c counts: %i %i %i %i (sum %i)\n", maxDepth, ncount, ecount, pcount, ncount + ecount + pcount);
#endif
    continue;
  }

  while (posSize > 0) {
    maxj = 0;
    if (posSize == 1) break;
    for (j = 0; j < LISTSIZE; j++) {
      if (listMin[j] >  listMin[maxj]) maxj = j;
      if (listMin[j] == listMin[maxj] && listTie[j] > listTie[maxj]) maxj = j;
    }

    if (r % 2) break;
    r = r >> 1;
    posSize--;
    listMin[maxj] = listMin[posSize];
    listTie[maxj] = listTie[posSize];
    listPos[maxj] = listPos[posSize];
    listNeg[maxj] = listNeg[posSize];
    i--;
  }

  unsigned int posMask = listPos[maxj];
  unsigned int negMask = listNeg[maxj];

  int removed = seive (posMask, negMask);

  if (removed > 0) {
#ifdef CLAUSES
    printClause (posMask, negMask);
#endif
  }

  int res = filter (posMask, negMask);
  updateRep (posMask, negMask);

  nClause++;
  nLit += popCount(posMask) + popCount (negMask);

  if (res == -1) {
    printf("c ERROR!!! one class got eliminated %i %i %i\n", posMask, negMask, seed);
    goto end; }
  }


  end:
  #ifdef COMMENTS
 printf("c cover updated %llu times, hit %llu times\n", cover_updates, cover_hits);
 #endif
  free (listPos);
  free (listNeg);
  free (listMin);
  free (listTie);
}

