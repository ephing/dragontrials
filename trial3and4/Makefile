
GRAMFILE := $(shell find . -name "*.grammar" | head -n 1)
TOKENFILE := $(shell find . -name "*.tokens" | head -n 1)

.PHONY: build run test dot clean

build:
	echo "used python"

# added the -d flag here so you don;t accidentally miss it but it technically should only be in
# the make dot one
run: $(GRAMFILE)
	python3 main.py $<
	chmod +x parser

test: $(TOKENFILE) run
	./parser $<

# if this is trial 3 submission, ignore everything to do with dot,
# else dont ignore
dot: $(GRAMFILE)
	python3 main.py -d $<
	dot -Tpng parser.dot -o parser.png

clean:
	rm -rf *.dot *.png __pycache__ parser