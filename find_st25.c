#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <assert.h>
#define MAXN 33
#include "nautinv.h"
#include "naututil.h"


graph stn[MAXN*MAXM];
graph canon_stn[MAXN*MAXM];
int tot_checked = 0;
int stn_size;

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

#define LINE_LENGTH 256

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

void makeSubGraph(int n, graph *g, set *verts, graph *output) {
  int vmap[n];
  int ctr = 0;
  for (int v = 0; v < n; v++) {
    vmap[v] = (ISELEMENT(verts, v))? ctr++ : -1;
  }

  for (int v = 0; v < n-1; v++) {
    set *row = GRAPHROW(g, v, MAXM);
    for (int v2 = v + 1; v2 < n; v2++) {
      if (ISELEMENT(verts,v) && ISELEMENT(verts, v2)) {
        if (ISELEMENT(row, v2)) {
          ADDONEARC(output,vmap[v],vmap[v2],MAXM);
        } else {
          ADDONEARC(output,vmap[v2],vmap[v],MAXM);
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
  int ctr = 0;
  for (int v = 0; v < n; v++) {
    vmap[v] = (ISELEMENT(verts, v))? ctr++ : -1;
    vmap_inv[ctr-1] = v; 
  }
  /*
  printf("subg perms: \n");
  for (int v = 0; v < n; v++) {
    printf("%d ", vmap[v]);
  }
  printf("\n");
  for (int v = 0; v < n; v++) {
    printf("%d ", vmap_inv[v]);
  }
  printf("\n");
  */
  // vmap goes from full graph vertex to permutation ind

  for (int v = 0; v < n-1; v++) {
    int v_new = (ISELEMENT(verts, v))? vmap_inv[lab[vmap[v]]] : v;
    set *row = GRAPHROW(g, v, MAXM);
    for (int v2 = v + 1; v2 < n; v2++) {
      int v2_new = (ISELEMENT(verts, v2))? vmap_inv[lab[vmap[v2]]] : v2;
      if (ISELEMENT(row, v2)) {
        ADDONEARC(output,v_new,v2_new,MAXM);
      } else {
        ADDONEARC(output,v2_new,v_new,MAXM);
      }
    }
  }
}

bool graphEqual(int n, graph *g1, graph *g2) {
    for (int v = 0; v < n; v++) {
      if (g1[v] != g2[v]) {
        return false;
      }
    }
    
    return true;
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

int factorial(int x) {
  int ret = 1;
  for (int i = 1; i <= x; i++) {
    ret = ret * i;
  }
  return ret;
}

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
// vert2deg contains the IN degree for each of the n verts
// used is a set of vertices for which removal has been exhaustively tried, i.e. it should not be a removal candidate at this depth.
bool findSTN(int n, set *verts, int* lab,  set *used, graph* g, int depth) {
  if (tot_checked > 13884156) {
    return false;
  }
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
int main (int argc, char** argv) {
  if (argc != 4) {
    printf("expected 3 arguments (inputfile, stnfile, stn_size), got %d", argc);
    return 1;
  }

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
  //printGraph(stn_size, stn);

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
  int n = 33;
  int m = SETWORDSNEEDED(n);
  nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
  //fprintf(stderr, "WS: %d\n", WORDSIZE);
  int num_non_st = 0;
  while(1)
  {
        size_t x;
        if(getline(&graphline, &x, input) == -1)
        {
            fprintf(stderr, "breaking %lu\n", strlen(graphline));
            break;
        }
        ct ++;
        /*
        if (ct <= 2091) {
          continue;
        }*/
        //fprintf(stderr, "%s\n", graphline);  

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
        bool res = findSTN(n, init_verts, lab, used_verts, g, 0); 
        fprintf(stderr, "graph: %d, res: %d, tot checked: %d\n",ct, res, tot_checked);
        if (!res) {
          num_non_st++;
          printf("X %d\n\n", ct);
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

        //printGraph(n, g_ulstn);
        //printf("\n");

        //makeSubGraph(n, g_ulstn, first_stn_verts, g_ulstn_cut);

        //printPerm(stn_size, lab);
        //printPerm(stn_size, st_lab);
        //printPerm(stn_size, inv_st_lab);
        //printPerm(stn_size, inv2_st_lab);
        //printGraph(stn_size, stn);
        //printf("\n");
        //printGraph(stn_size, canon_stn_inv);
        //printf("\n");
        //printf("\n");
        //printGraph(stn_size, canon_stn);
        //printf("\n");
        //printGraph(n, g_ulstn);
        //printf("canon_stn = stn2canon(stn)? %d\n", graphEqual(stn_size, canon_stn, canon_stn2));
        //printf("stn = canon2stn(canon_stn))? %d\n", graphEqual(stn_size, stn, canon_stn_inv));
        //printf("canon_stn = cut permuted g? %d\n", graphEqual(stn_size, canon_stn, g_ulstn_cut));
        tot_checked = 0;
        //fprintf(stderr, "\ncardinality: %d\n", POPCOUNT(init_verts[0]));
        //char outname[

        
        //printf("graph isomorphic to original? %d\n", graphsIso(n, g, g_ulstn));

        int iso_units_perm[n - stn_size];
        findIsoUnitsPerm(n, g_ulstn, compl_stn_verts, iso_units_perm);
        //printPerm(n - stn_size, iso_units_perm);
        int inv_iso_units_perm[n - stn_size];
        invertPerm(n - stn_size, iso_units_perm, inv_iso_units_perm);
        graph g_ulstn_iso[MAXN*MAXM];
        EMPTYGRAPH(g_ulstn_iso, MAXM, n);
        permuteSubGraph(n, g_ulstn, compl_stn_verts, inv_iso_units_perm, g_ulstn_iso);

        printGraph(n, g_ulstn_iso);
        printf("\n");
        
        /*
        int pn = 4;
        int myperm[pn];
        for (int i = 0; i < factorial(pn); i++) {
          getPerm(pn, i, myperm);
        }
        break;
        */



  }
  fprintf(stderr,"read %d graphs, %d did not have stn\n", ct, num_non_st);
  free(graphline);
  fclose(input);
}
