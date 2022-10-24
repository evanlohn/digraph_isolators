import math
import matplotlib.pyplot as plt

def optimistic_ramsey_nums_cond(n_verts):
	if n_verts >= 64: # nothing known for k>7
		return naive_ramsey_cond(n_verts/34) + 6
	for k, Rk in zip([7,6,5,4,3,2], [34,28,14,8,4,2]):
		if n_verts >= Rk:
			return k
	return 1

def known_ramsey_nums_cond(n_verts):
	if n_verts >= 64: # nothing known for k>7
		return naive_ramsey_cond(n_verts/47) + 6
	for k, Rk in zip([7,6,5,4,3,2], [47,28,14,8,4,2]):
		if n_verts >= Rk:
			return k
	return 1

def naive_ramsey_cond(n_verts):
	return int(math.log(n_verts, 2)) + 1

def produce_table(ramsey_cond, upto=100):
	ret = {}
	for i in range(2, upto):
		verts_remaining = i
		units_added = 0
		while verts_remaining > 0:
			if verts_remaining in ret:
				units_added += ret[verts_remaining] #dynamic programming ;)
				break
			verts_used = ramsey_cond(verts_remaining)
			verts_remaining -= verts_used
			units_added += verts_used * (verts_used - 1)//2
		ret[i] = units_added
	return ret

def print_for_latex(xs, y_pairs):
	for y_name, ys in y_pairs:
		print(y_name)
		print(''.join([f'({x},{y})' for x, y in zip(xs, ys)]))
		print()


def plot_unit_counts(upto=100000):

	tab_naive = produce_table(naive_ramsey_cond, upto=upto)
	tab_known = produce_table(known_ramsey_nums_cond, upto=upto)
	tab_hope = produce_table(optimistic_ramsey_nums_cond, upto=upto)
	xs = list(range(2,upto))
	ys = [tab_naive[x] for x in xs]
	ys_known = [tab_known[x] for x in xs]
	ys_hope = [tab_hope[x] for x in xs]
	
	met1 = lambda x: x * (math.log(x,2) - 2)/2
	curr = 0
	curr_ind = 0
	def met2(x):
		nonlocal curr, curr_ind
		while curr_ind < x:
			curr_ind += 1
			curr = curr + int(math.log(curr_ind,2))/2
		return curr
	#met2 = lambda x: tab_naive[x-1] + math.log(x,2) - 7

	sampler = lambda lst: [x for i, x in enumerate(lst) if i %(len(lst)//1000) == 0]

	if upto >= 100000:
		xs = sampler(xs)
		ys = sampler(ys)
		ys_known = sampler(ys_known)
		ys_hope = sampler(ys_hope)

	met2ys = [met2(x) for x in xs]
	plt.plot(xs, ys_hope, label='units_hope(n)')
	plt.plot(xs, ys, label='units_naive(n)')
	plt.plot(xs, ys_known, label='units(n)')
	#plt.plot(xs, [4*x for x in xs])
	#plt.plot(xs, [met1(x) for x in xs])
	plt.plot(xs, met2ys, label='LB(n)')
	plt.legend()
	plt.xlabel('n')

	plt.show()
	print_for_latex(xs,[('units_naive(n)', ys), ('units_hope(n)', ys_hope), ('units_known(n)', ys_known), ('LB(n)', met2ys)])
	#print('units(n)')
	#print(''.join([f'({x},{y})' for x, y in zip(xs, ys)]))
	#print()
	#print()
	#print('LB(n)')
	#print(''.join([f'({x},{y})' for x, y in zip(xs, met2ys)]))

def compare_bounds(upto=1000000):
	tab_naive = produce_table(naive_ramsey_cond, upto=upto)
	#tab_known = produce_table(known_ramsey_nums_cond, upto=upto)
	at = True
	avg_diff = 0
	pct_correct = 0
	tab = tab_naive
	log_upto_i = 0
	mdiff = 0
	for i in range(2, upto):
		log_upto_i += int(math.log(i,2))/2
		lb = log_upto_i
		#lb = tab[i-1] + math.log(i,4) -5 # doesn't actually work: need to increase 5 as upto increases
		#lb = i * (math.log(i,2) - 2)/2 # works up to 8,000,000
		at = at and lb <= tab[i]
		pct_correct += 1 if lb <= tab[i] else 0
		avg_diff += tab[i] - lb
		mdiff = max(mdiff, tab[i] - lb)
		print(i, tab[i], lb, lb <= tab[i])
		#print(i, tab_naive[i], tab_known[i], tab_known[i] >= tab_naive[i])
		
	print(at)
	print(f'avg diff: {avg_diff/(upto - 2)}')
	print(f'max diff: {mdiff}')
	print(f'pct satisfying lower bound: {pct_correct/(upto-2)}')

if __name__ == '__main__':
	compare_bounds(18)
	#plot_unit_counts(200)
	#plot_unit_counts(100000)
