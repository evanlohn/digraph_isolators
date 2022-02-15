
# read in a .cnf.txt from shatter and produce symmetry breaking clauses that do NOT introduce new solutions
import re
import sys

def read_symms(fname, max_var):
    with open(fname, 'r') as f:
        lines = f.readlines()[1:-1] # cut off [ and ]

    regex = r'\(((\d+,)*\d+)\)'
    symms = []
    new_symm = True
    for line in lines:
        matches = re.findall(regex, line)
        matches = [x[0] for x in matches]
        if new_symm:
            var_occur = {}
            all_nums = []
        for match in matches:
            nums = [int(x) if int(x) <= max_var else -(int(x) - max_var) for x in match.split(',') if int(x) <= 2*max_var]
            if len(nums) <= 1:
                continue
            match_ind = len(all_nums)
            for i, num in enumerate(nums):
                if abs(num) not in var_occur:
                    var_occur[abs(num)] = [(match_ind, i)]
                else:
                    var_occur[abs(num)].append((match_ind, i))
            all_nums.append(nums)
        new_symm = line[-2] == ','
        if new_symm:
            symms.append((all_nums, var_occur))
    symms.append((all_nums, var_occur))
    return symms

def uf_find(uf, x):
    x = abs(x)
    curr = x
    if x not in uf:
        return -1
    while uf[curr] != curr:
        curr = uf[curr]
    uf[x] = curr # path compression ;)
    return curr

def uf_add(uf, x, y):
    x = abs(x)
    y = abs(y)
    if x not in uf:
        if y not in uf:
            uf[x] = x
            uf[y] = x
        else:
            uf[x] = uf_find(uf, y)
    else:
        # could do union by rank, but its like 3 extra LOC
        uf[y] = uf_find(uf, x)

def uf_would_form_cycle(uf, x, y):
    tmp = uf_find(uf, x)
    return tmp == uf_find(uf, y) and tmp != -1

def generate_symm_break_pairs(symms, max_var):
    # each tuple of symms represents an independent symmetry
    symm_pairs = []
    for all_nums, var_occur in symms:
        used = set()
        def seen_before(a, b):
            return (abs(a), abs(b)) in used or (abs(b),abs(a)) in used
        tmp = []
        uf = {} # union-find data structure
        for var in range(1, max_var+1):
            if var in var_occur: # variable is part of some symmetries and symm not yet broken on it.
                ind_lst = var_occur[var]
                for match_ind, var_ind in ind_lst:
                    signed_var = all_nums[match_ind][var_ind]
                    assert abs(signed_var) == var
                    next_ind = (var_ind + 1) % len(all_nums[match_ind])
                    next_var = all_nums[match_ind][next_ind]
                    if signed_var < 0:
                        signed_var, next_var = -signed_var, -next_var

                    #if abs(next_var) not in used:
                    if not seen_before(signed_var, next_var) and (next_var < 0 or signed_var < next_var) and not uf_would_form_cycle(uf, signed_var, next_var):
                        used.add((var, abs(next_var)))
                        uf_add(uf, var, next_var)
                        #used.add(abs(next_var))
                        tmp.append((signed_var, next_var))
                        break
                if abs(var) == abs(next_var):
                    break

        symm_pairs.append(tmp)
        #print(''.join([f'({v1},{v2})' for v1, v2 in tmp]))
    return symm_pairs

def generate_symm_break_clauses(symm_pairs, fresh_var):
    # special first symm:
    v1, v2 = symm_pairs[0]
    clauses = [[-v1, v2]]

    #special first defn + symm break
    fv = fresh_var()
    clauses += [[fv, -v1], [fv, v2], [-fv, v1, -v2]] #defn
    v1, v2 = symm_pairs[1]
    clauses += [[-fv, -v1, v2]] # symm break

    for new_vs in symm_pairs[2:]: # general pattern to break remaining symms
        new_fv = fresh_var()
        clauses += [[new_fv, -fv, -v1], [new_fv, -fv, v2], [-new_fv, fv], [-new_fv, v1, -v2]] #defn
        v1, v2 = new_vs
        fv = new_fv
        clauses += [[-fv, -v1, v2]] # symm break
    return clauses

def output_clauses(clauses):
    for clause in clauses:
        print(' '.join([str(x) for x in clause]) + ' 0')


if __name__ == '__main__':
    fname = sys.argv[1]
    max_var = int(sys.argv[2]) if len(sys.argv) > 2 else None
    if max_var is None:
        n = int(fname.split('_')[1])
        max_var = n * (n-1) // 2
    symms = read_symms(fname, max_var)
    if max_var is None:
        max_var = max([max(var_occur) for _, var_occur in symms])
    all_symm_pairs = generate_symm_break_pairs(symms, max_var)
    def fresh_var_maker():
        next_var = max_var
        def fresh_var():
                nonlocal next_var
                next_var += 1
                return next_var
        return fresh_var
    fresh_var = fresh_var_maker()
    clauses = []
    for symm_pairs in all_symm_pairs:
        if len(symm_pairs) == 0:
            continue
        clauses += generate_symm_break_clauses(symm_pairs, fresh_var)

    if len(clauses) == 0:
        print('no symms found')
    else:
        output_clauses(clauses)



