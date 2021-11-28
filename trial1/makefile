SPECFILE := $(shell find . -name "*.spec" | head -n 1)
TESTFILES := $(wildcard *.txt)
TOKENFILES := $(TESTFILES:.txt=.tokens)

.PHONY: build run test clean

build:
	echo "I used python, build is a no-op"

run: $(SPECFILE)
	python3 main.py $(SPECFILE)
	chmod +x lexer

test: clean run $(TOKENFILES)

%.tokens: %.txt
	./lexer $<

clean:
	rm -rf lexer *.tokens *.err nfas __pycache__
