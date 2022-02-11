CC=gcc

all: filter probe

probe: probe.c
	$(CC) probe.c -o probe

filter: filter.c
	$(CC) filter.c -o filter
.PHONY: clean

clean: 
	rm -f probe filter
