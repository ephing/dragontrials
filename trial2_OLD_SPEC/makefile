GRAMFILE := $(shell find . -name "*.ag" | head -n 1)
TOKENFILES := $(shell find . -name "*.tokens" | head -n 1)

.PHONY: build run test clean

build:
	echo "I used python, build is a no-op"

run: $(GRAMFILE)
	python3 main.py
	chmod +x translator

test: clean $(TOKENFILES) run
	./translator $(TOKENFILES)

clean:
	rm -rf __pycache__ translator