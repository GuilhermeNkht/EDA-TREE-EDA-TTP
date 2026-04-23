all:
	(cd ttp && $(MAKE))

bench:
	(cd ttp && $(MAKE) MYFLAGS=-DGHOST_BENCH)

trace:
	(cd ttp && $(MAKE) MYFLAGS=-DGHOST_TRACE)

clean:
	(cd ttp && $(MAKE) clean)

hill:
	(cd ttp && $(MAKE) MYFLAGS=-DGHOST_HILL_CLIMBING)