CC=gcc

all: filter probe find_st25 

find_st25: find_st25.c nauty_util_evan.c nauty_util_evan.h
	$(CC) find_st25.c nauty_util_evan.c nauty.a -o find_st25

probe: probe.c
	$(CC) probe.c -o probe

filter: filter.c
	$(CC) filter.c -o filter
.PHONY: clean

clean: 
	rm -f probe filter find_st25
