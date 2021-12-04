LEXER_TOOL := flex
CXX ?= g++ # Set the C++ compiler to g++ iff it hasn't already been set
CPP_SRCS := $(wildcard *.cpp) 
OBJ_SRCS := parser.o lexer.o $(CPP_SRCS:.cpp=.o)
DEPS := $(OBJ_SRCS:.o=.d)
FLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Wuninitialized -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wundef -Werror -Wno-unused -Wno-unused-parameter

.PHONY: all clean test t7

TESTPROGS := $(wildcard tests/*.tnc)
TESTS := $(TESTPROGS:.tnc=)

.PHONY: all clean test cleantest

all: 
	make cshantyc

clean:
	rm -rf *.output *.o *.cc *.hh $(DEPS) cshantyc parser.dot parser.png
	$(MAKE) -C t7_tests/ clean

-include $(DEPS)

cshantyc: $(OBJ_SRCS) stdcshanty.o
	$(CXX) $(FLAGS) -g -std=c++14 -o $@ $(OBJ_SRCS)

stdcshanty.o: stdcshanty.c
	gcc -c stdcshanty.c

%.o: %.cpp 
	$(CXX) $(FLAGS) -g -std=c++14 -MMD -MP -c -o $@ $<

parser.o: parser.cc
	$(CXX) $(FLAGS) -Wno-sign-compare -Wno-sign-conversion -Wno-switch-default -g -std=c++14 -MMD -MP -c -o $@ $<

parser.cc: cshanty.yy
	bison -Wnone -Wno-deprecated --defines=grammar.hh -v $<

lexer.yy.cc: cshanty.l
	$(LEXER_TOOL) --outfile=lexer.yy.cc $<

lexer.o: lexer.yy.cc
	$(CXX) $(FLAGS) -Wno-sign-compare -Wno-sign-conversion -Wno-old-style-cast -Wno-switch-default -g -std=c++14 -c lexer.yy.cc -o lexer.o

test: t7

t7: all
	$(MAKE) -C t7_tests/
