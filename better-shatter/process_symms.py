
# read in a .cnf.txt from shatter and produce symmetry breaking clauses that do NOT introduce new solutions
import re
import sys

def read_symms(fname, max_var):
	with open(fname, 'r') as f:
		lines = f.readlines()[1:-1] # cut off [ and ]

	regex = '\(((\d+,)*\d+)\)'
	symms = []
	for line in lines:
		matches = re.findall(line, regex)
		var_occur = {}
		all_nums = []
		for match_ind, match in enumerate(matches):
			nums = [int(x) if int(x) <= max_var else -(int(x) - max_var) for x in match[1:-1].split(',') if int(x) <= 2*max_var]
			for i, num in enumerate(nums):
				if abs(num) not in var_occur:
					var_occur[abs(num)] = [(match_ind, i)]
				else:
					var_occur[abs(num)].append((match_ind, i))
			all_nums.append(nums)
		symms.append((all_nums, var_occur))
	return symms

def generate_symm_break_pairs(symms, max_var):
	# each tuple of symms represents an independent symmetry
	symm_pairs = []
	for all_nums, var_occur in symms:
		used = set()
		for var in range(1, max_var+1):
			if var in var_occur and var not in used: # variable is part of some symmetries and symm not yet broken on it.
				ind_lst = var_occur[var]
				for match_ind, var_ind in ind_lst:
					signed_var = all_nums[match_ind][var_ind]
					next_ind = (var_ind + 1) % len(all_nums[match_ind])
					next_var = all_nums[match_ind][next_ind]
					if next_var not in used:
						used.add(var)
						used.add(abs(next_var))
						symm_pairs.append((signed_var, next_var))
						break
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
	max_var = None if len(sys.argv) <= 2 else int(sys.argv[2])
	symms = read_symms(fname)
	if max_var is None:
		max_var = max([max(var_occur) for _, var_occur in symms])
	symm_pairs = generate_symm_break_pairs(symms, max_var)
        def fresh_var_maker():
            next_var = max_var
            def fresh_var():
                    nonlocal next_var
                    next_var += 1
                    return next_var
            return fresh_var
	clauses = generate_symm_break_clauses(symm_pairs, fresh_var_maker())
	output_clauses(clauses)



