import math
import matplotlib.pyplot as plt

def known_ramsey_nums_cond(n_verts):
	if n_verts >= 64: # nothing known for k>7
		return naive_ramsey_cond(n_verts)
	for k, Rk in zip([7,6,5,4,3,2], [54,28,14,8,4,2]):
		if n_verts >= Rk:
			return k
	return 1

def naive_ramsey_cond(n_verts):
	return int(math.log(n_verts, 2)) + 1

def produce_table(ramsey_cond, upto=100):
	ret = {}
	for i in range(1, upto):
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

def plot_unit_counts(upto=100000):

	tab_naive = produce_table(naive_ramsey_cond, upto=upto)
	xs = list(range(2,upto))
	ys = [tab_naive[x] for x in xs]
	
	met1 = lambda x: x * (math.log(x,2) - 2)/2
	met2 = lambda x: tab_naive[x-1] + math.log(x,2) - 7

	sampler = lambda lst: [x for i, x in enumerate(lst) if i %(len(lst)//1000) == 0]

	if upto >= 100000:
		xs = sampler(xs)
		ys = sampler(ys)

	plt.plot(xs, ys)
	#plt.plot(xs, [4*x for x in xs])
	plt.plot(xs, [met1(x) for x in xs])
	plt.plot(xs, [met2(x) for x in xs])

	plt.show()

def compare_bounds(upto=1000000):
	tab_naive = produce_table(naive_ramsey_cond, upto=upto)
	#tab_known = produce_table(known_ramsey_nums_cond, upto=upto)
	at = True
	avg_diff = 0
	pct_correct = 0
	tab = tab_naive
	for i in range(2, upto):
		lb = tab[i-1] + math.log(i,4) -5 # doesn't actually work: need to increase 5 as upto increases
		#lb = i * (math.log(i,2) - 2)/2 # works up to 8,000,000
		at = at and lb <= tab[i]
		pct_correct += 1 if lb <= tab[i] else 0
		avg_diff += tab[i] - lb
		print(i, tab[i], lb, lb <= tab[i])
		#print(i, tab_naive[i], tab_known[i], tab_known[i] >= tab_naive[i])
		
	print(at)
	print(f'avg diff: {avg_diff/(upto - 2)}')
	print(f'pct satisfying lower bound: {pct_correct/(upto-2)}')

if __name__ == '__main__':
	compare_bounds(100000)
	#plot_unit_counts(20000)
