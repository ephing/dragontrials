import sys
from tablebuilder import EOF, STARTNONTERM

table = {}
tkstream = []
termLex = []
lookahead = 0
semlook = 0
stack = []
semstack = []
terminals = []
nonterminals = []
lamList = {}

def makeToken(line):
    if ':' in line: 
        s = line[:line.find(':')]
        l = eval(line[line.find(':')+1:line.find(' ')])
    else:
        s = line[:line.find(' ')]
        l = None
    return {'symbol':s, 
            'lexeme':l, 
            'pos':[int(line[line.rfind('[')+1:line.rfind(',')]),
                    int(line[line.rfind(',')+1:line.rfind(']')])] }

def readTokenStream(filename):
    tk = []
    with open(filename, "r") as file:
        line = file.readline().rstrip()
        while line != '':
            tk.append(makeToken(line))
            line = file.readline().rstrip()
    # add EOF token if there isn't one for whatever reason
    if tk[-1]['symbol'] != "EOF":
        eoftoken = {'symbol':EOF,'lexeme':None, 'pos':[tk[-1]['pos'][0]+1,1]}
        tk.append(eoftoken)
    else: tk[-1]['symbol'] = EOF
    return tk

def push(val):
    global stack
    stack = [val] + stack

def pop():
    global stack
    if len(stack) != 0: 
        val = stack[0]
        stack = stack[1:]
        return val

def sempush(val):
    global semstack
    semstack = [val] + semstack

def sempop():
    global semstack
    if len(semstack) != 0:
        val = semstack[0]
        semstack = semstack[1:]
        return val

def lexpush(val):
    global termLex
    termLex = [val] + termLex

def lexpop():
    global termLex
    if len(termLex) != 0:
        val = termLex[0]
        termLex = termLex[1:]
        return val

def getProdFromSemNum(num: int):
    for x in table:
        for y in table[x]:
            if table[x][y] != None:
                if num in table[x][y]: return table[x][y]
    return None

def start():
    if len(sys.argv) != 2:
        print("format: ./translator <.tokens file>")
    else:
        global tkstream, lookahead, semstack, termLex,stack
        tkstream = readTokenStream(sys.argv[1])
        # start with the start nonterm hur dur
        push(STARTNONTERM)
        # go until we're done or until the thing explodes
        while len(stack) != 0:
            # if terminal, check if TOS == lookahead and pop, explode if not
            #print(stack[0])
            if stack[0] in terminals:
                if stack[0] == tkstream[lookahead]['symbol']:
                    if tkstream[lookahead]['lexeme'] != None:
                        lexpush(tkstream[lookahead]['lexeme'])
                    lookahead += 1
                    pop()
                else:
                    print("rejected: non-matching terminals")
                    break
            # if nonterminal, pop and replace with value in table, explode if there isnt one
            elif stack[0] in nonterminals:
                val = pop()
                prod = table[val][tkstream[lookahead]['symbol']]
                if prod == None:
                    print("rejected")
                    break
                else:
                    for x in reversed(prod): push(x)
            #sem action
            else:
                num = pop()
                prod = getProdFromSemNum(num)
                combined = []
                if prod != None:
                    for x in reversed(prod):
                        if x in nonterminals:
                            combined += [sempop()]
                if callable(lamList[num]):
                    tmp = lamList[num]
                    for x in reversed(combined):
                        tmp = tmp(x)
                    combined = [tmp]
                elif lamList[num] != None: combined = [lamList[num]] + combined
                if len(combined) > 0: sempush(combined[0])
                print(semstack)

        # if no explosion, accept, otherwise don't
        else: 
            print("accepted: " + str(semstack[0]))
            sys.exit(0)
        sys.exit(1)