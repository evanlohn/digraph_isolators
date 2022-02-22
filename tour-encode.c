#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define LIMIT	1

#define MAX	50

int n, k, limit, aux;

int **matrix;

int *list, *mark, listSize;

int getEdge (int a, int b) {
  assert (a != b);
  assert (a >  0);
  assert (b >  0);
  int min = a, max = b;
  if (min > max) { min = b; max = a; }

  int res = min + (max - 2) * (max - 1) / 2;

  if (a < b) return res;
  return -res;
}

int getTriangle (int a, int b, int c) {
  assert (a != b);
  assert (a != c);
  assert (b != c);
  assert (a >  0);
  assert (b >  0);
  assert (c >  0);
  int min = a, mid = b, max = c, swap;
  if (min > mid) { min = b;   mid = a; }
  if (min > max) { max = min; min = c; }
  if (mid > max) { swap = max; max = mid; mid = swap; }

  int res = min + n * (n-1) / 2;;
  res += (mid - 2) * (mid - 1) / 2;
  res += (max - 3) * (max - 2) * (max - 1) / 6;

  return res;
}


int litcmp (const void * a, const void * b) {
   return ( abs(*(int*)a) - abs(*(int*)b) );
}

void permutations (int *order, int s) {
  int a, b, i;
  if (listSize > limit) return;

  if (s == 1) {
    int flag = 1;
    for (a = 0; a < k; a++)
      for (b = a + 1; b < k; b++) {
         int m = matrix[order[a]][order[b]];
         if (m == 1) flag = 0;
      }
    if (flag) {
      int* cls = (int *) malloc (sizeof(int) * k * k);
      int size = 0;
      for (a = 0; a < k; a++)
        for (b = a + 1; b < k; b++) {
           int m = matrix[order[a]][order[b]];
           if (m == 0) cls[size++] = getEdge (order[a], order[b]);
        }

      qsort (cls, size, sizeof(int), litcmp);
      int mask = 0;
      for (i = 0; i < size; i++)
        if (cls[i] < 0) mask |= (1 << i);
      mark[listSize  ] = 0;
      list[listSize++] = mask; } }

  else {
    permutations (order, s - 1);

    for (i = 0; i < s-1; i++) {
      if ((s % 2) == 0) {
        int tmp = order[i];
        order[i]   = order[s-1];
        order[s-1] = tmp; }
      else {
        int tmp = order[0];
        order[0]   = order[s-1];
        order[s-1] = tmp; }
      permutations (order, s - 1); }
  }
}

void printReducedClause (int *order, int mask, int xor) {
  int a, b, s = 0;
  for (b = 0; b < k; b++)
    for (a = 0; a < b; a++)
      if (matrix[order[a]][order[b]] == 0) {
        if ((xor & (1 << s)) == 0) {
          if (mask & (1 << s)) printf ("-");
          printf ("%i ", getEdge(order[a],order[b])); }
        s++; }
  printf ("0\n");
}

int extendXOR (int *order, int mask, int xor) {
  int i, j;

  for (i = 0; i < listSize; i++) {
    int x = mask ^ list[i];
    if (xor == (xor | x)) continue;
    while ((x % 2) == 0) x = x >> 1;
    if (x == 1) {
      x = mask ^ list[i];
      for (j = 0; j < listSize; j++)
        if (list[j] == (mask ^ (xor|x)))
          return x;
    }
  }
  return 0;
}

void printClause (int* order) {
  int a, b, c, i, j;
  listSize = 0;

  for (a = 0; a < k; a++)
    for (b = a + 1; b < k; b++)
      for (c = b + 1; c < k; c++)
        if (matrix[order[a]][order[b]] == -matrix[order[a]][order[c]] &&
            matrix[order[a]][order[b]] ==  matrix[order[b]][order[c]] &&
            matrix[order[a]][order[b]] != 0)
              return;

  int* tmpOrder = (int *) malloc (sizeof (int) * k);
  for (i = 0; i < k; i++) tmpOrder[i] = order[i];
  permutations (tmpOrder, k);
  free (tmpOrder);

  int iter = 1;
  while (iter) {
    iter = 0;
    for (i = 0; i < listSize; i++) {
      if (mark[i]) continue;
      int xor   = 0;
      for (j = 0; j < listSize; j++) {
        if (i == j) continue;
        int x = list[i] ^ list[j];
        while ((x % 2) == 0) x = x >> 1;
        if (x == 1) xor |= list[i] ^ list[j]; }

      int flag = 0;
      for (j = 0; j < listSize; j++)
        if (xor && (list[j] == (list[i] ^ xor))) {
          flag = 1; break; }

      // turn this list element into a reduced clause
      if (flag == 1) {
        while (1) {
          int x = extendXOR (order, list[i], xor);
          if (x == 0) break;
          xor |= x; }

//      printf ("c printing reduced clause %i %i %i\n", i, list[i], xor);
      printReducedClause (order, list[i], xor);
        iter = 1;
        for (j = 0; j < listSize; j++)
          if ((xor | (list[i] ^ list[j])) == xor)
            mark[j] = 1; } } }

  if (listSize <= limit) {
    for (i = 0; i < listSize; i++) {
      if (mark[i] == 0) {
        int xor = 0;
        while (1) {
          int x = extendXOR (order, list[i], xor);
          if (x == 0) break;
          xor |= x; }

        printReducedClause (order, list[i], xor); } } }
  else {
    aux = 1;
    for (a = 0; a < k; a++)
      for (b = a + 1; b < k; b++)
        for (c = b + 1; c < k; c++)
          printf ("%i ", getTriangle (order[a], order[b], order[c]));
    printf ("0\n"); }
}

void extendRec (int* order, int size) {
  if (size == k) printClause (order);
  else {
    int i;
    int start = 0;
    if (size > 0) start = order[size-1];
    for (i = start; i < n; i++) {
        order[size] = i + 1;
        extendRec (order, size+1); } }
}

int main (int argc, char** argv) {
  int i, j, tmp, row = 0, m = 1;
  char ch;

  // initialize global variables
  limit = LIMIT;
  aux   = 0;

  int line[MAX];
  for (i = 0; i < MAX; i++) line[i] = -1;

  assert (argc > 2);
  n = atoi (argv[1]);
  k = atoi (argv[2]);

  int *order = malloc (sizeof (int) * n);

  matrix = (int **) malloc (sizeof (int *) * (MAX+1));
  for (i = 1; i <= MAX; i++) {
    matrix[i] = (int *) malloc (sizeof(int) * (MAX+1));
    for (j = 1; j <= MAX; j++)
      if (i == j) matrix[i][j] = -1;
      else        matrix[i][j] =  0;
  }

  if (argc > 3) {
    FILE* input = fopen (argv[3], "r");

    int l = 1;
    while (1) {
      tmp = fscanf (input, "%c", &ch);
      if (tmp == EOF) break;
      if (ch == ' ') continue;
      if (ch == '1') line[l++] = 1;
      if (ch == '0') line[l++] = 0;
      if (ch == '*') line[l++] = -1;
      if (ch == '\n') {
        l--;
        for (i = m + 1; i <= l; i++) {
          if (line[i] == -1) continue;
            matrix[m][i] = line[i] ? 1 : -1;
            matrix[i][m] = -matrix[m][i]; }
        l = 1;
        m++;
      }
    }
  }

  if (argc > 4) {
    limit = atoi (argv[4]); }

  int nVar = n * (n-1) / 2 + n * (n-1) * (n-2) / 6;

  printf ("p cnf %i %i\n", nVar, 5000);

  for (i = 1; i <= n; i++)
    for (j = 1; j <= n; j++)
      if (matrix[i][j] == 1) printf ("%i 0\n", getEdge(i,j));

  // initialize list
  listSize = 1;
  for (i = 2; i <= k; i++)
    listSize *= k;

  list = (int *) malloc (sizeof (int) * listSize);
  mark = (int *) malloc (sizeof (int) * listSize);
  for (i = 0; i < listSize; i++) mark[i] = 0;

  listSize = 0;

  extendRec (order, 0);

  if (aux) {
    int a, b, c;
    for (a = 1; a <= n; a++)
      for (b = a+1; b <= n; b++)
        for (c = b+1; c <= n; c++) {
          printf ("%i %i %i 0\n", -getTriangle (a, b, c), -getEdge (a,b), getEdge (b,c));
          printf ("%i %i %i 0\n", -getTriangle (a, b, c), -getEdge (b,c), getEdge (c,a));
          printf ("%i %i %i 0\n", -getTriangle (a, b, c), -getEdge (c,a), getEdge (a,b));
          printf ("%i %i %i 0\n", -getTriangle (a, b, c), -getEdge (a,b), getEdge (c,a));
          printf ("%i %i %i 0\n", -getTriangle (a, b, c), -getEdge (c,a), getEdge (b,c));
          printf ("%i %i %i 0\n", -getTriangle (a, b, c), -getEdge (b,c), getEdge (a,b)); }
  }
}
