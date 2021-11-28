GRAMFILE := $(shell find . -name "*.ag" | head -n 1)
TOKENFILES := $(shell find . -name "*.tokens" | head -n 1)

.PHONY: build run test clean

build:
	echo "used python for .ag reading"

run: $(GRAMFILE)
	python3 dragonsdt.py $<
	g++ -std=c++14 translator.cpp -o parser

test: $(TOKENFILES) run
	./parser $<

clean:
	rm -rf translator.cpp parser __pycache__