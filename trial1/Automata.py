from copy import deepcopy
import RegexTree as RT

class NFA:
    def __init__(self,c: chr):
        self.startState = 0
        self.finStates = [1]
        self.sigma = [c, None]
        self.deltaT = [[[1],[]],[[],[]]]

    def print(self):
        print("stst: " + str(self.startState))
        print("finst: " + str(self.finStates))
        print(self.sigma)
        for x in range(len(self.deltaT)):
            if x < 10: print(str(x) + " : " + str(self.deltaT[x]))
            else: print(str(x) + ": " + str(self.deltaT[x]))
        print("\n")

def mergeSigma(fa1: NFA,fa2: NFA):
    if fa1.sigma == fa2.sigma: return [fa1,fa2]
    fa1 = deepcopy(fa1)
    fa2 = deepcopy(fa2)
    #add missing letters to fa1, and missing transitions for those letters
    for x in fa2.sigma:
        if x not in fa1.sigma:
            fa1.sigma.insert(len(fa1.sigma) - 1, x)
            #need to keep epsilon at end of sigma for ease, hence this doodoo line
            fa1.deltaT = [y[:len(y) - 1] + [[]] + [y[len(y) - 1]] for y in fa1.deltaT]
    
    # add missing letters to fa2 and transitions, while maintaining same order as fa1.sigma
    temp = [[] for _ in fa2.deltaT]
    for x in fa1.sigma:
        if x in fa2.sigma:
            temp = [temp[y] + [fa2.deltaT[y][fa2.sigma.index(x)]] for y in range(len(temp))]
        else:
            temp = [y + [[]] for y in temp]

    fa2.deltaT = temp
    fa2.sigma = fa1.sigma
    return [fa1,fa2]

def concat(nfa1: NFA,nfa2: NFA):
    res = NFA(' ')
    #make both machines have same sigma
    nfa1, nfa2 = mergeSigma(deepcopy(nfa1),deepcopy(nfa2))
    res.sigma = nfa1.sigma
    delt = deepcopy(nfa2.deltaT)
    #change numeric rep of nfa2 states so they don't overlap with nfa1
    for state in range(len(delt)):
        for let in range(len(delt[state])):
            for let2 in range(len(delt[state][let])):
                if delt[state][let][let2] != None:
                    delt[state][let][let2] += len(nfa1.deltaT)
    res.deltaT = deepcopy(nfa1.deltaT) + delt
    # add epsilon transition from final states of nfa1 to start state of nfa2
    for st in nfa1.finStates:
        res.deltaT[st][len(nfa1.sigma) - 1] += [nfa2.startState + len(nfa1.deltaT)]
    res.startState = nfa1.startState
    res.finStates = [st + len(nfa1.deltaT) for st in nfa2.finStates]
    return epsremove(res)

def altern(nfa1: NFA, nfa2: NFA):
    res = NFA(' ')
    nfa1, nfa2 = mergeSigma(deepcopy(nfa1), deepcopy(nfa2))
    res.sigma = nfa1.sigma
    res.deltaT = [[[] for l in res.sigma]]
    delt = deepcopy(nfa1.deltaT)
    # change numeric rep of nfa1 states to not overlap with new start state
    for state in range(len(delt)):
        for let in range(len(delt[state])):
            for let2 in range(len(delt[state][let])):
                delt[state][let][let2] += 1
    res.deltaT += delt
    delt = deepcopy(nfa2.deltaT)
    #change num rep of nfa2 states to not overlap yadda yadda
    for state in range(len(delt)):
        for let in range(len(delt[state])):
            for let2 in range(len(delt[state][let])):
                delt[state][let][let2] += 1 + len(nfa1.deltaT)
    res.deltaT += delt
    # add new final state
    res.deltaT += [[[] for l in res.sigma]]
    res.finStates = [len(res.deltaT) - 1]
    #add a bunch of epsilon transitions
    res.deltaT[0][len(res.sigma) - 1] = [nfa1.startState + 1, nfa2.startState + 1 + len(nfa1.deltaT)]
    for st in nfa1.finStates:
        res.deltaT[st + 1][len(res.sigma) - 1] += [len(res.deltaT) - 1]
    
    for st in nfa2.finStates:
        res.deltaT[st + 1 + len(nfa1.deltaT)][len(res.sigma) - 1] += [len(res.deltaT) - 1]
    return epsremove(res)

def kleene(nfa: NFA):
    #copy paste
    res = NFA(' ')
    res.sigma = nfa.sigma
    res.startState = nfa.startState
    res.finStates = deepcopy(nfa.finStates)
    res.deltaT = deepcopy(nfa.deltaT)
    #add epsilon transitions
    for st in res.finStates:
        res.deltaT[st][len(res.sigma) - 1] += [res.startState]
        res.deltaT[res.startState][len(res.sigma) - 1] += [st]
    return res

def getVisited(fa: NFA, vs: list, curr: int):
    if curr in vs: return
    else:
        vs.append(curr)
        for x in range(len(fa.deltaT[curr])):
            for y in fa.deltaT[curr][x]:
                getVisited(fa,vs,y)

def reduce(fa: NFA):
    visitedstates = []
    getVisited(fa, visitedstates, fa.startState)
    visitedstates.sort()
    tempTFunc = []
    tempFinal = []
    # create deltaT without unvisited states
    for x in range(len(fa.deltaT)):
        if x in visitedstates: tempTFunc.append(fa.deltaT[x])
    #do something funcky idk I wrote this a long time ago
    for x in visitedstates:
        for y in range(len(tempTFunc)):
            for z in range(len(tempTFunc[y])):
                if x in tempTFunc[y][z]: tempTFunc[y][z] = [visitedstates.index(x)]
        if x in fa.finStates:
            tempFinal.append(visitedstates.index(x))
    fa.finStates = tempFinal
    fa.deltaT = tempTFunc
    return fa

def epsClosure(nfa: NFA, state: int, res: list):
    if state not in res and state != None: 
        res.append(state)
        next = nfa.deltaT[state][len(nfa.sigma) - 1]
        for st in next:
            if st != state:
                res = epsClosure(nfa,st,res)
    return res

def epsremove(nfa: NFA):
    res = deepcopy(nfa)
    #get epsclose for each state
    epsclose = [epsClosure(res, st,[]) for st in range(len(res.deltaT))]
    #remove epsilon transitions entirely
    for st in range(len(res.deltaT)):
        res.deltaT[st][len(res.sigma) - 1] = []
    
    # add new transitions and final states based on the closure using lots of for loops
    for st in range(len(res.deltaT)):
        for eps in epsclose[st]:
            for l in range(len(res.sigma) - 1):
                if res.deltaT[eps][l] == []: continue
                for nst in res.deltaT[eps][l]:
                    if nst not in res.deltaT[st][l]:
                        res.deltaT[st][l] += [nst]
            if st not in res.finStates and eps in res.finStates:
                res.finStates.append(st)
    # call reduce to get rid of excess states to make further calculation only partially obliterate cpu
    return reduce(res)

def toDFAHelp(nfa: NFA, res: NFA, curr: int, states: list, ind: dict):
    if curr in states: return
    states.append(curr)
    ind.update({ len(res.deltaT): curr })
    res.deltaT.append([])
    stindex = len(res.deltaT) - 1

    #determine if final state
    for x in curr:
        if x in nfa.finStates:
            res.finStates.append(stindex)
            break

    #recursively build new states and transitions as we traverse to them
    for l in range(len(res.sigma)):
        next = []
        for st in curr:
            for x in nfa.deltaT[st][l]:
                if x not in next:
                    next.append(x)
        next.sort()
        res.deltaT[stindex].append(next)
        if next != []:
            toDFAHelp(nfa,res,next,states,ind)

def toDFA3(nfa: NFA):
    res = NFA(' ')
    res.sigma = nfa.sigma[:len(nfa.sigma) - 1]
    res.deltaT = []
    res.startState = 0
    res.finStates = []

    ind = {}
    toDFAHelp(nfa,res,[res.startState],[], ind)
    ndelta = [[[] for _ in res.sigma] for _ in res.deltaT]

    for x in ind:
        for st in range(len(res.deltaT)):
            for l in range(len(res.sigma)):
                if res.deltaT[st][l] == ind[x]:
                    ndelta[st][l] = [x]
    res.deltaT = ndelta
    return res

def toDict(nfa: NFA):
    out = {}
    out["startState"] = nfa.startState
    out["sigma"] = nfa.sigma
    out["finStates"] = nfa.finStates
    out["deltaT"] = nfa.deltaT
    return out

#basic recursive traversal to build from bottom up
def buildAutomata(tree):
    mach = None
    if isinstance(tree, RT.Prim):
        if (tree.regex == "." and tree.isWildcard): 
            mach = NFA('\t')
            for x in range(32,127):
                mach.sigma.insert(1,chr(x))
                mach.deltaT[0].insert(1,[1])
                mach.deltaT[1].insert(1,[])
        else:
            mach = NFA(tree.regex)
    elif isinstance(tree, RT.Concat):
        mach = concat(buildAutomata(tree.t1), buildAutomata(tree.t2))
    elif isinstance(tree, RT.Split):
        mach = altern(buildAutomata(tree.ch1), buildAutomata(tree.ch2))
    elif isinstance(tree, RT.MeanKleene):
        mach = kleene(buildAutomata(tree.rep))
    elif tree == None:
        # this case should only be true if we did a ? regex operator, just praying I'm right haha
        mach = NFA('')
        mach.sigma = [None]
        mach.deltaT = [[[1]],[[]]]
    else:
        print(tree)
        raise Exception("Invalid tree")
    return mach
