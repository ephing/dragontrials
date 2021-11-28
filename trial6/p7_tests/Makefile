TESTFILES := $(wildcard *.cshanty)
TESTS := $(TESTFILES:.cshanty=.test)
LIBLINUX := -dynamic-linker /lib64/ld-linux-x86-64.so.2

.PHONY: all

all: $(TESTS)

%.test:
	@rm -f $*.err $*.s
	@touch $*.err $*X64.s $*MIPS.s
	@echo "TEST $*"
	@../cshantyc $*.cshanty -o $*X64.s -m $*MIPS.s ;\
	COMP_EXIT_CODE=$$?;
	@as -o $*.o $*X64.s
	@ld $(LIBLINUX) \
		/usr/lib/x86_64-linux-gnu/crt1.o \
		/usr/lib/x86_64-linux-gnu/crti.o \
		-lc \
		$*.o \
		../stdcshanty.o \
		/usr/lib/x86_64-linux-gnu/crtn.o \
		-o  $*.prog
	@./$*.prog < $*.in > $*.out; \
	diff -B --ignore-all-space $*.out $*.out.expected;\
	RUN_DIFF_EXIT=$$?;\
	exit $$RUN_DIFF_EXIT

clean:
	rm -f *.out *.err *.o *.s *.prog
