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

