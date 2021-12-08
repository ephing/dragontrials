TESTFILES := $(wildcard *.cshanty)
TESTS := $(TESTFILES:.cshanty=.test)
LIBLINUX := -dynamic-linker /lib64/ld-linux-x86-64.so.2

.PHONY: all

all: $(TESTS)

%.test:
	@rm -f $*.err $*.out
	@touch $*.err $*.out
	@echo "TEST $*"
	@../cshantyc $*.cshanty -l $*.ll ;\
	COMP_EXIT_CODE=$$?;
	@llvm-as-9 -o $*.o $*.ll
	@lli $*.ll < $*.in > $*.out; \
	diff -B --ignore-all-space $*.out $*.out.expected;\
	RUN_DIFF_EXIT=$$?;\
	exit $$RUN_DIFF_EXIT

clean:
	rm -f *.ll *.out *.err *.c
