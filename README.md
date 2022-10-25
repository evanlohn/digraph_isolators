Repository for the FMCAD '22 paper [Compact Symmetry Breaking for Tournaments](https://www.cs.cmu.edu/~mheule/publications/FMCAD22.pdf)

`isolators` contains our tournament isolators from n=3 to n=8
`map_files` contains the files mapping tournaments of a given size to an equivalence class label
`tt7free_experiments` is a folder full of a bunch of cluttered files used for the ramsey experiments
If you'd like to replicate our ramsey experiment results, `src/find_st25.c` is a good starting point. running `./setup.sh` from `src` will download nauty and link the files
the `find_st25` script needs. The previously known 5303 tt7free n=33 graphs can be found [here](https://users.cecs.anu.edu.au/~bdm/data/digraphs.html)

The following script solves for the existence of a 2-clause SBP (Symmetry Breaking Predicate) for complete digraphs on 3 vertices.
Running `decode_clauses.py` as shown below outputs a full interpretation of all truth assignments made by the SAT solver, as well as the corresponding SBP when the result was SAT.
Example usage:
```bash
python3 src/gen_map_files.py 3
python src/tocnf.py 3 2 --filename di_sbp_3_2
cadical di_sbp_3_2.cnf > di_sbp_3_2.out
python decode_clauses di_sbp_3_2
```

I cleaned up the repo a lot to make it easier to look through, but in doing so probably broke some hardcoded paths in my scripts. 
If anything isn't easy to fix, let me know via opening an issue or emailing me.
