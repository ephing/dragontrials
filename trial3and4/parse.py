from Reader import EOF

start = ""
table = {}

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

class Parser:
    def __init__(self,tks):
        self.stack = [start]
        self.lookahead = 0
        self.tkstream = readTokenStream(tks)

    def parse(self):
        while len(self.stack) != 0:
            next = table[self.stack[0]][self.tkstream[self.lookahead]['symbol']]
            #accept
            if next == "accept":
                print("SUCCESS!")
                return
            #failure
            elif next == None:
                print("rejected")
                exit(1)
            #shift/goto
            elif isinstance(next,str):
                self.stack = [next] + self.stack
                self.lookahead += 1
            #reduce
            else:
                self.stack = [table[self.stack[0]][next[0]]] + self.stack[next[1]:]
        print("rejected")
        exit(1)

        