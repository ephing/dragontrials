TESTFILES := $(wildcard *.cshanty)
TESTS := $(TESTFILES:.cshanty=.test)

.PHONY: all

all: $(TESTS)

%.test:
	@rm -f $*.out
	@touch $*.out
	@echo "TEST $*"
	@../dragoninterp $*.cshanty > $*.out;\
	PROG_EXIT_CODE=$$?;\
	diff -B --ignore-all-space $*.out $*.out.expected;\
	TAC_DIFF_EXIT=$$?;\
	exit $$TAC_DIFF_EXIT

clean:
	rm -f *.out