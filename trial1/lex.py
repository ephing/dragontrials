# this code is the majority of the tokenizer. Only code that gets generated is DFA stuff
import sys
rules = {}
# keeps track of current and previous state, and whether a machine is accepted
class AMTracker:
    def __init__(self):
        self.reset()

    # delete search history
    def reset(self):
        self.states = [[rules[x]['dfa']['startState']] for x in rules]

    # transition machines based on input character
    def delta(self,c):
        for x in rules:
            try:
                cindex = rules[x]['dfa']['sigma'].index(c)
                self.states[x] = [rules[x]['dfa']['deltaT'][self.states[x][0]][cindex][0]] + self.states[x]
            except:
                self.states[x] = [None] + self.states[x]

    # returns non-dead machine index, or None if they're all dead
    def death(self):
        for st in range(len(self.states)):
            if self.states[st][0] != None:
                return st
        return None

    # checks all previous states and returns which machine was the winner as well as
    # how many characters we actually consume with this token
    def revert(self):
        for x in range(len(self.states[0])):
            for r in rules:
                try:
                    if self.states[r][x] in rules[r]['dfa']['finStates']:
                        return (r,len(self.states[0]) - x - 1)
                except:
                    pass            
        raise Exception("No matching tokens!")

def peek(f):
    p = f.tell()
    out = f.read(1)
    f.seek(p)
    return out

def start():
    if len(sys.argv) != 2:
        print("Format: ./lexer <input file>")
    else:
        amt = AMTracker()
        with open(sys.argv[1],"r") as file:
            with open(sys.argv[1][:sys.argv[1].rfind('.')] + ".tokens", "w") as outfile:
                with open(sys.argv[1][:sys.argv[1].rfind('.')] + ".err", "a+") as errfile:
                    errfile.truncate(0)
                    lineNum = 1
                    colNum = 1
                    tsLine = 1
                    tsCol = 1
                    token = ""
                    # while not EOF
                    c = peek(file)
                    while len(c) != 0:
                        # move all machines
                        amt.delta(c)
                        # if they all dead
                        if amt.death() == None:
                            (rev, tk) = amt.revert()
                            act = rules[rev]['action']
                            if act[0:5] == "(ERR)":
                                errfile.write(act[5:] + " [" + str(tsLine) + ", " + str(tsCol) + "]\n")
                            else:
                                if act[0] != '(':
                                    if act[-2] == 'u': act = act = act[:act.rfind(' ')] + ":" + token[:tk]
                                    else: act = act = act[:act.rfind(' ')]
                                    outfile.write(act + " [" + str(tsLine) + ", " + str(tsCol) + "]\n")
                            amt.reset()
                            # reset file reader to read from tk, in case revert() told us to consume fewer characters
                            # then we already did
                            file.seek(file.tell() + tk - len(token))
                            tsLine = lineNum
                            tsCol = colNum - len(token) + tk
                            token = ""
                        else:
                            file.read(1)
                            token += c
                            colNum += 1
                            if c == "\n":
                                lineNum += 1
                                colNum = 1
                            # handle no \n before EOF
                            if len(peek(file)) == 0:
                                (rev, tk) = amt.revert()
                                act = rules[rev]['action']
                                if act[0:5] == "(ERR)":
                                    errfile.write(act[5:] + " [" + str(tsLine) + ", " + str(tsCol) + "]\n")
                                else:
                                    if act[0] != '(':
                                        if act[-2] == 'u': act = act = act[:act.rfind(' ')] + ":" + token[:tk]
                                        else: act = act = act[:act.rfind(' ')]
                                        outfile.write(act + " [" + str(tsLine) + ", " + str(tsCol) + "]\n")
                        c = peek(file)
                    outfile.write("EOF [" + str(lineNum) + ", " + str(colNum) + "]\n")