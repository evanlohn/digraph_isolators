#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX	50

int n, k;

int **matrix;

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

int litcmp (const void * a, const void * b) {
   return ( abs(*(int*)a) - abs(*(int*)b) );
}

void permutations (int *order, int s) {
  int a, b, i;
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
      for (i = 0; i < size; i++)
        printf ("%i ", cls[i]);
      printf ("0\n"); } }
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

void printClause (int* order) {
  int a, b, c;

  for (a = 0; a < k; a++)
    for (b = a + 1; b < k; b++)
      for (c = b + 1; c < k; c++)
        if (matrix[order[a]][order[b]] == -matrix[order[a]][order[c]] &&
            matrix[order[a]][order[b]] ==  matrix[order[b]][order[c]] &&
            matrix[order[a]][order[b]] != 0)
              return;

  int i;
  int* tmpOrder = (int *) malloc (sizeof (int) * k);
  for (i = 0; i < k; i++) tmpOrder[i] = order[i];
  permutations (tmpOrder, k);
  free (tmpOrder);

}

void extendRec (int* order, int size) {
  if (size == k) printClause (order);
  else {
    int i;
    int start = 0;
    if (size > 0) start = order[size-1];
    for (i = start; i < n; i++) {
        order[size] = i + 1;
        extendRec (order, size+1);
    }
  }
}

int main (int argc, char** argv) {
  int i, j, tmp, row = 0, m = 1;
  char ch;

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
        m++; } } }

  int nVar = n * (n-1) / 2;

  printf ("p cnf %i %i\n", nVar, 5000);

  for (i = 1; i <= n; i++)
    for (j = 1; j <= n; j++)
      if (matrix[i][j] == 1) printf ("%i 0\n", getEdge(i,j));

  extendRec (order, 0);

}
