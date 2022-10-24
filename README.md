repository for generating optimal SBPs for digraphs (for now, only complete digraphs)

The following script solves for the existence of a 2-clause SBP for complete digraphs on 3 vertices.
Running `decode_clauses.py` as shown below outputs a full interpretation of all truth assignments made by the SAT solver, as well as the corresponding SBP when the result was SAT.
Example usage:
```bash
python gen_map_files.py 3
python tocnf.py 3 2 --filename di_sbp_3_2
cadical di_sbp_3_2.cnf > di_sbp_3_2.out
python decode_clauses di_sbp_3_2
```

`permuted_ST26_exc_all.txt` is all the different permutations with ST26 in the top left of graphs without ST27.
`tt7free_iso7_all_negation_patterns_autom2.txt` is clauses denying all known permutations of any tt7free33some.d6 graph that put ST26 in the top left up to automorphism of ST26, but NOT up to automorphism of the isolated graph
                                                however, it uses the wrong clauses to deny the ST26->ST27 transition
`tt7free_iso7_all_negation_patterns_autom3.txt` is the same as `tt7free_iso7_all_negation_patterns_autom2.txt`, but fixes the `ST26->ST27` transition clause issue.
`tt7free_iso7_all_negation_patterns_autom4.txt` denies everything up to both automorphisms but only for graphs with ST27
`tt7free_iso7_all_negation_patterns_autom5.txt` denies everything up to both automorphisms but only for graphs without ST27
`tt7free_iso7_all_negation_patterns_autom5b.txt` same as above, but ignores the "new" solution we found (first in my modified tt7free33some.d6 file).
`tt7free_iso7_all_negation_patterns_autom6.txt` concatenates 4 and 5 :)
`tt7free_iso7_all_negation_patterns_autom6b.txt` concatenates 4 and 5b :)
