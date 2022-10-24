#include "nauty_util_evan.h"
#define MAX_CLAUSE_LENGTH 16

graph stn[MAXN*MAXM];
graph canon_stn[MAXN*MAXM];
int tot_checked = 0;
int stn_size;

bool isPrefix(char *s1, char *s2) {
  for (int i = 0; i < strlen(s1); i++) {
    if (s1[i] != s2[i]) {
      return false;
    }
  }
  return true;
}

int readIsolator(char *fname, int n, int** isolator) {
  char * graphline = (char *) malloc (LINE_LENGTH * sizeof(char));
  FILE * input = fopen(fname, "r");

  int ct = 0;
  bool ready = false;
  char buf[BUFSIZE];
  int ch_read = 0;
  int edge_ind = -1;
  int lit_ind = 0;
  while(1)
  {
        size_t x = 0;
        if(getline(&graphline, &x, input) == -1)
        {
            break;
        }
        if (ready) {
          
          for (int i = 0; i<BUFSIZE;i++) {
            buf[i] = '\0';
          }

          ch_read = 0;
          lit_ind = 0;
          x = strlen(graphline);
          for(int l = 0; l < x; l++)
          {
              char ch = graphline[l];
              if ((ch == ' ') || (ch == '\n')) {
                if (ch_read > 0) {
                  edge_ind = atoi(buf);
                  isolator[ct][lit_ind++] = edge_ind;
                }

                //cleanup
                ch_read = 0;
                for (int i = 0; i<BUFSIZE;i++) {
                  buf[i] = '\0';
                }
                if (edge_ind == 0) break; // just in case
                
              } else {
                buf[ch_read++] = ch;
              }
          }
          
          if (ch_read != 0) {
            edge_ind = atoi(buf);
            isolator[ct][lit_ind++] = edge_ind;
            //fprintf(stderr, "int of len %d remaining in buffer '%s', line len %ld \n", ch_read, buf, strlen(graphline));
          }
          ct++;

        } else {
          if (isPrefix("clauses", graphline)) {
            ready = true;
          }
        }
  }

  free(graphline);
  fclose(input);
  return ct;
}

int getMinDegreeVert(int n, graph *g, set *verts, set *exclude) {
  int minDegv = -1;
  int minDeg = n+1;
  for (int v = 0; v < n; v++) {
    if(ISELEMENT(verts,v) && !ISELEMENT(exclude, v)) {
      int outDeg = outDegree(n, g, verts, v);
      if (outDeg < minDeg) {
        minDeg = outDeg;
        minDegv = v;
      }
    }
  }
  if (minDegv == -1) {
    //fprintf(stderr, "failed to find a min degree vert\n");
    //printSet(n, verts);
    //fprintf(stderr,"tests on v4: is a vert: %d", ISELEMENT(verts,4));
  }
  return minDegv;
}

int iso8rows[11] = {0,0,1,1,2,2,3,4,5,4,6};
int iso8cols[11] = {1,3,2,4,3,5,4,5,6,7,7};

int verts2units[9] = {0,0,1,1,3,5,8,9,11}; // Note: the units above really aren't meant for n < 6

/*
bool hasIsoUnits(int n, graph *g, set *verts, int *perm) {

  for (int unit = 0; unit < verts2units[ctr]; unit++) {
    if (!ISELEMENT(GRAPHROW(g, vmap_inv[perm[iso8rows[unit]]], MAXM), vmap_inv[perm[iso8cols[unit]]])) {
      return false;
    }
  }
  return true;
}
*/


//permId is in the range [0..n!-1]
void getPerm(int n, int permId, int *perm) {
  int permVert = 0;
  int slicer = factorial(n);
  for (int i = 0; i < n; i++) {
    perm[i] = -1;
  }
  while (permVert < n) {
    slicer = slicer / (n - permVert);
    int outV = permId / slicer;
    permId = permId % slicer;
    for (int i = 0; i < n; i++) {
      if (perm[i] == -1) {
        if (outV == 0) {
          perm[i] = permVert++;
          break;
        } else {
          outV--;
        }
      } 
    }
  }
}

bool satisfies_isolator(int n, graph *g, int *perm, int **isolator, int iso_len, int *edges, int *vmap_inv) {
  for (int c = 0; c < iso_len; c++) {
    for (int v = 0; v < MAX_CLAUSE_LENGTH; v++) {
      int var = isolator[c][v];
      if (var == 0) return false; // unable to find a satisfied literal in the clause
      if (var < 0) {
        if (ISELEMENT(GRAPHROW(g, vmap_inv[perm[edges[2*(-var) + 1]]],MAXM), vmap_inv[perm[edges[2*(-var)]]])) {
          break; // found a literal that satisfies the clause
        }
      } else { //positive var
        if (ISELEMENT(GRAPHROW(g, vmap_inv[perm[edges[2*var]]],MAXM), vmap_inv[perm[edges[2*var + 1]]])) {
          break; // found a literal that satisfies the clause
        }
      }
    }
  }
  return true;
}


void findIsoPerm(int n, graph *g, set *verts, int *perm, int **isolator, int iso_len) {
  int vmap_inv[n];
  int ctr = 0;
  for (int v = 0; v < n; v++) {
    if (ISELEMENT(verts, v)) {
      vmap_inv[ctr++] = v; 
    }
  }

  int num_edges = n*(n-1)/2;
  int *edges = malloc ((num_edges+1)*2 * sizeof(int));
  int tmp = 1;
  for (int end = 1; end < n; end++) {
    for (int start = 0; start < end; start++) {
      edges[2*tmp] = start;
      edges[2*tmp+1] = end;
      tmp++;
    }
  }


  bool flag = false;
  for (int pid = 0; pid < factorial(ctr); pid++) {
    getPerm(ctr, pid, perm);
    if (satisfies_isolator(n, g, perm, isolator, iso_len, edges, vmap_inv)) {
      flag = true;
      break;
    }
  }
  if (!flag) {
    fprintf(stderr, "failed to find permutation matching isolator!\n");
  }
  free(edges);
}
void findIsoUnitsPerm(int n, graph *g, set *verts, int *perm) {
  int vmap_inv[n];
  int ctr = 0;
  for (int v = 0; v < n; v++) {
    if (ISELEMENT(verts, v)) {
      vmap_inv[ctr++] = v; 
    }
  }

  for (int pid = 0; pid < factorial(ctr); pid++) {
    getPerm(ctr, pid, perm);
    bool flag = true;
    for (int unit = 0; unit < verts2units[ctr]; unit++) {
      if (!ISELEMENT(GRAPHROW(g, vmap_inv[perm[iso8rows[unit]]], MAXM), vmap_inv[perm[iso8cols[unit]]])) {
        flag = false;
        break;
      }
    }
    if (flag) {
      return; 
    }
  }
  
}

// modifies verts to contain exactly the vertices isomorphic to STN 
// used is a set of vertices for which removal has been exhaustively tried, i.e. it should not be a removal candidate at this depth.
bool findSTN(int n, set *verts, int* lab,  set *used, graph* g, int depth) {
  //printSet(n, verts);
  //fprintf(stderr, "\n");
  set local_used[n];
  memcpy(local_used, used, n*sizeof(set));
  //EMPTYSET(local_used, MAXM);
  if (depth == (n-stn_size)) {
    if (POPCOUNT(verts[0]) != stn_size) {
      fprintf(stderr, "remaining verts wrong, expected %d got %d\n",stn_size, POPCOUNT(verts[0]));
    }
    graph subg[stn_size*MAXM];
    graph canon_subg[stn_size*MAXM];
    EMPTYGRAPH(subg, MAXM, stn_size);
    makeSubGraph(n, g, verts, subg);

    //printGraph(stn_size, subg);
    

    static DEFAULTOPTIONS_DIGRAPH(options);
    statsblk stats;
    options.getcanon=TRUE;
    options.writeautoms=FALSE;
    int ptn[stn_size],orbits[stn_size];
    densenauty(subg, lab, ptn, orbits, &options, &stats, MAXM, stn_size, canon_subg);

    tot_checked++;
    // iso with STN check
    for (int v = 0; v < stn_size; v++) {
      if (canon_stn[v] != canon_subg[v]) {
        return false;
      }
    }
    
    return true;
  }

  // whenever a vertex with degree < 12 is found, it MUST be removed; no need to check any other vertices. 
  // optimization: if there are more than (8 - depth) vertices with degree < 12, fail immediately
/*
  for (int v = 0; v < n; v++) {
    if(ISELEMENT(verts,v)) {
      int outDeg = outDegree(n, g, verts, v);
      if (outDeg < 12) {
        //fprintf(stderr, "found degree < 12: %d\n", v);
        DELELEMENT(verts, v);
        bool found = findSTN(n,verts,lab,local_used, g, depth + 1);
        if (found) {
          return true; // verts is correct, found an isomorphism, we're done
        }
        // otherwise, need to consider v in the future, but not in this branch 
        // (having degree < 12 means v MUST be removed in this branch for an isomorphism to be found)
        ADDELEMENT(verts, v);
        return false;
      } 
    }
  }
*/

  int vremove = getMinDegreeVert(n,g,verts, local_used);//minDegv; // = blah
  while(POPCOUNT(local_used[0]) < stn_size + 1) {
    DELELEMENT(verts, vremove);
    bool found = findSTN(n,verts,lab,local_used, g, depth + 1);
    if (found) {
      return true; // verts is correct, found an isomorphism, we're done
    }
    ADDELEMENT(verts, vremove);
    ADDELEMENT(local_used, vremove); // we searched for all isomorphs with vremove gone, so we locally protect it from being removed again.

    //pick vertex to remove (use verts, local_used)
    vremove = getMinDegreeVert(n,g,verts, local_used);
    if (vremove == -1) {
      break;
    }
  }
  return false;
}

// counts the number of subgraphs isomorphic to STN 
// used is a set of vertices for which removal has been exhaustively tried, i.e. it should not be a removal candidate at this depth.
int countSTN(int n, set *verts, int* lab,  set *used, graph* g, int depth) {
  set local_used[n];
  memcpy(local_used, used, n*sizeof(set));
  if (depth == (n-stn_size)) {
    if (POPCOUNT(verts[0]) != stn_size) {
      fprintf(stderr, "remaining verts wrong, expected %d got %d\n",stn_size, POPCOUNT(verts[0]));
    }
    graph subg[stn_size*MAXM];
    graph canon_subg[stn_size*MAXM];
    EMPTYGRAPH(subg, MAXM, stn_size);
    makeSubGraph(n, g, verts, subg);
    
    static DEFAULTOPTIONS_DIGRAPH(options);
    statsblk stats;
    options.getcanon=TRUE;
    options.writeautoms=FALSE;
    int ptn[stn_size],orbits[stn_size];
    densenauty(subg, lab, ptn, orbits, &options, &stats, MAXM, stn_size, canon_subg);

    tot_checked++;
    // iso with STN check
    return graphEqual(stn_size, canon_stn, canon_subg) ? 1 : 0;
  }

  int ret = 0;
  int vremove = getMinDegreeVert(n,g,verts, local_used);//not necessary for counting, but if it aint broke dont fix it
  while(POPCOUNT(local_used[0]) < stn_size + 1) {
    DELELEMENT(verts, vremove);
    ret += countSTN(n,verts,lab,local_used, g, depth + 1);
    ADDELEMENT(verts, vremove);
    ADDELEMENT(local_used, vremove); // we searched for all isomorphs with vremove gone, so we locally protect it from being removed again.

    //pick vertex to remove (use verts, local_used)
    vremove = getMinDegreeVert(n,g,verts, local_used);
    if (vremove == -1) {
      break;
    }
  }
  return ret;
}

/*
    set *vrm_adj = GRAPHROW(g,vremove,MAXM);
    for (int v = 0; v < n; v++) {
      if (ISELEMENT(vrm_adj, v)) {
        vert2deg[v] -= 1;
        if (vert2deg[v] == 12) {
          ADDELEMENT(protect_set, v);
        }
      }
    }
    

    for (int v = 0; v < n; v++) {
      if (ISELEMENT(vrm_adj, v)) {
        vert2deg[v] += 1;
        if (vert2deg[v] > 12) {
          DELETEELEMENT(protect_set, v);
        }
      }
    }
    */



void printIsolator(int n) {
  char fname[15]; // up to two digit isolators just in case
  sprintf(fname, "isolator%d.txt", n);
  int isolines = countLines(fname);
  int **isolator = (int **) (malloc (isolines * sizeof(int*)));
  for (int c = 0; c < isolines; c++) {
    isolator[c] = malloc (MAX_CLAUSE_LENGTH * sizeof(int));
    for (int i = 0; i < MAX_CLAUSE_LENGTH; i++) {
      isolator[c][i] = 0;
    }
  }
  
  int iso_len = readIsolator(fname, n, isolator);

  for (int c = 0; c < iso_len; c++) {
    for (int v = 0; v < MAX_CLAUSE_LENGTH; v++) {
      int var = isolator[c][v];
      printf("%d ", var);
      if (var == 0) break;
    }
    printf("\n");
  }


  for (int c = 0; c < isolines; c++) {
    free(isolator[c]);
  }
  free(isolator);
}

int main (int argc, char** argv) {
  if (argc != 4) {
    printf("expected 3 arguments (inputfile, stnfile, stn_size), got %d", argc);
    return 1;
  }
  bool count_mode = true;

  int n = 33; //NOTE: change this for different-sized graphs
  stn_size = atoi (argv[3]);

  EMPTYGRAPH(stn, MAXM, stn_size);
  FILE * stnfile = fopen(argv[2], "r");
  char *fline = (char*) malloc (33);
  for (int r = 0; r < stn_size - 1; r++){
    size_t x = 0;
    if(getline(&fline, &x, stnfile) == -1 || x < stn_size) {
      fprintf(stderr, "not enough lines\n");
      return 1;
    }
    for (int c = r+1; c < stn_size; c++) {
      if (fline[c] == '0') {
        ADDONEARC(stn, c, r, MAXM);
      } else {
        if (fline[c] != '1') {
          fprintf(stderr, "found a nonbinary character in stn: %c\n", fline[c]);
          return 1;
        }
        ADDONEARC(stn, r, c, MAXM);
      }
    }
  }
  free(fline);
  fclose(stnfile);
  //printGraph(stn_size, stn);
  char fname[15]; // up to two digit isolators just in case
  sprintf(fname, "isolator%d.txt", n-stn_size);
  fname[13] = '\0'; // i'm paranoid
 
  int isolines = countLines(fname);
  int **isolator = (int **) (malloc (isolines * sizeof(int*)));
  for (int c = 0; c < isolines; c++) {
    isolator[c] = malloc (MAX_CLAUSE_LENGTH * sizeof(int));
    for (int i = 0; i < MAX_CLAUSE_LENGTH; i++) {
      isolator[c][i] = 0;
    }
  }
  int iso_len = readIsolator(fname, n, isolator);

  //int lab[MAXN],ptn[MAXN],orbits[MAXN];
  static DEFAULTOPTIONS_DIGRAPH(options);
  statsblk stats;
  options.getcanon=TRUE;
  options.writeautoms=FALSE;
  // compute canonical stn
  int st_lab[stn_size],ptn[stn_size],orbits[stn_size];
  densenauty(stn, st_lab, ptn, orbits, &options, &stats, MAXM, stn_size, canon_stn);
  //printGraph(stn_size, canon_stn);


  char * graphline = (char *) malloc (LINE_LENGTH * sizeof(char));
  FILE * input = fopen(argv[1], "r");

  graph g[MAXN*MAXM];
  int ct = 0;
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  //fprintf(stderr, "WS: %d\n", WORDSIZE);
  int num_non_st = 0;


  while(1)
  {
    size_t x = 0;
    if(getline(&graphline, &x, input) == -1)
    {
        fprintf(stderr, "breaking %lu\n", strlen(graphline));
        break;
    }
    ct ++;

    if(graphline[0] != '&')
        fprintf(stderr, "ERROR: non-& first char at line %d\n", ct);
    
    if (graphline[1] - 63 != n) {
        fprintf(stderr, "ERROR: graph size %d\n", graphline[1]);
        break;
    }

    EMPTYGRAPH(g,m,n);
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
                ADDONEARC(g,edge_ind/n, edge_ind%n, m);
                //fprintf(stderr,"verts: %d %d\n", edge_ind/n, edge_ind%n);
            }
            edge_ind++;
        }
    }

    //do stuff with g
    // think of g as an array of "set". think of a "set" as a set of vertices,
    // but its underlying representation is like a bunch of "setwords", i.e. long ints.
    //fprintf(stderr, "m: %d\n", m);

    set init_verts[n];
    EMPTYSET(init_verts, MAXM);
    for (int v = 0; v < n; v++) {
      ADDELEMENT(init_verts, v);
    }
    
    set used_verts[n];
    int lab[stn_size];
    EMPTYSET(used_verts, MAXM);

    if (count_mode) {
      int tmp_ct = countSTN(n, init_verts, lab, used_verts, g, 0); 
      fprintf(stderr, "graph: %d, count: %d, tot checked: %d\n",ct, tmp_ct, tot_checked);
      printf("graph: %d, count: %d, tot checked: %d\n",ct, tmp_ct, tot_checked);
    } else {
      bool res = findSTN(n, init_verts, lab, used_verts, g, 0); 
      fprintf(stderr, "graph: %d, res: %d, tot checked: %d\n",ct, res, tot_checked);
      if (!res) {
        num_non_st++;
        printf("X %d\n\n", ct);
        tot_checked = 0;
        continue;
      }

      //printSet(n, init_verts);
      int inv_st_lab[stn_size];
      invertPerm(stn_size, st_lab, inv_st_lab);



      //graph canon_stn_inv[MAXN*MAXM];
      //EMPTYGRAPH(canon_stn_inv, MAXM, stn_size);
      //graph canon_stn2[MAXN*MAXM];
      //EMPTYGRAPH(canon_stn2, MAXM, stn_size);
      graph g_ulstn[MAXN*MAXM];
      EMPTYGRAPH(g_ulstn, MAXM, n);
      graph g_ulstn_cut[MAXN*MAXM];
      EMPTYGRAPH(g_ulstn_cut, MAXM, stn_size);

      set all_stn_verts[stn_size];
      EMPTYSET(all_stn_verts, MAXM);
      for (int v = 0; v < stn_size; v++) {
        ADDELEMENT(all_stn_verts, v);
      }
      
      int inv_lab[stn_size];
      invertPerm(stn_size, lab, inv_lab);
      int ul_perm[n];
      int st_ext_perm[n];
      int st2ul_perm[n];
      int ctr = 0;
      int ctr2 = 0;
      for (int v = 0; v < n; v++) {
        ul_perm[v] = (ISELEMENT(init_verts, v))? ctr++ : stn_size + ctr2++;
        st_ext_perm[v] = (v < stn_size)? inv_lab[v] : v;
      }
      //printSet(n, init_verts);
      //printPerm(n, ul_perm);
      //printPerm(stn_size, inv_lab);
      

      composePerms(n, st_ext_perm, ul_perm, st2ul_perm);
      set all_n_verts[n];
      set first_stn_verts[n];
      set compl_stn_verts[n];
      EMPTYSET(all_n_verts, MAXM);
      EMPTYSET(first_stn_verts, MAXM);
      EMPTYSET(compl_stn_verts, MAXM);
      for (int v = 0; v < n; v++) {
        ADDELEMENT(all_n_verts, v);
        if (v < stn_size) {
          ADDELEMENT(first_stn_verts, v);
        } else {
          ADDELEMENT(compl_stn_verts, v);
        }
      }
      permuteSubGraph(n, g, all_n_verts, st2ul_perm, g_ulstn);
      //permuteSubGraph(stn_size, stn, all_stn_verts, inv_st_lab, canon_stn2);
      //permuteSubGraph(stn_size, canon_stn, all_stn_verts, st_lab, canon_stn_inv);

      //makeSubGraph(n, g_ulstn, first_stn_verts, g_ulstn_cut);

      
      //fprintf(stderr, "\ncardinality: %d\n", POPCOUNT(init_verts[0]));
      //char outname[

      
      //printf("graph isomorphic to original? %d\n", graphsIso(n, g, g_ulstn));

      int iso_units_perm[n - stn_size];
      findIsoPerm(n, g_ulstn, compl_stn_verts, iso_units_perm, isolator, iso_len);
      //printPerm(n - stn_size, iso_units_perm);
      int inv_iso_units_perm[n - stn_size];
      invertPerm(n - stn_size, iso_units_perm, inv_iso_units_perm);
      graph g_ulstn_iso[MAXN*MAXM];
      EMPTYGRAPH(g_ulstn_iso, MAXM, n);
      permuteSubGraph(n, g_ulstn, compl_stn_verts, inv_iso_units_perm, g_ulstn_iso);

      printGraph(n, g_ulstn_iso);
      printf("\n");
    }
    tot_checked = 0;
  }
  fprintf(stderr,"read %d graphs, %d did not have stn\n", ct, num_non_st);
  free(graphline);
  for (int c = 0; c < isolines; c++) {
    free(isolator[c]);
  }
  free(isolator);
  fclose(input);
}
