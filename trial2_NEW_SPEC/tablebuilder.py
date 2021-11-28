# These two are used to add an extra nonterm, term, and production to the grammar
# \31 ::= X \30
# where X is the first nonterminal listed in the .ag file
# I use this to force the EOF token at the end of the grammar, for use in set generation
EOF = "\30"
STARTNONTERM = "\31"

class Grammar:
    def __init__(self,terms,nonterms,prods,ac):
        self.terms = terms + [EOF]
        self.nonterms = nonterms + [STARTNONTERM]
        self.trueprods = prods
        self.prods = {}
        for n in prods:
            if n not in self.prods.keys(): self.prods.update({n:[]})
            for p in prods[n]:
                self.prods[n] += [[]]
                for t in p:
                    if t[0] != '#': self.prods[n][-1] += [t]
        self.actions = ac
        self.trueprods.update({STARTNONTERM: [[self.nonterms[0], EOF]]})
        self.prods.update({STARTNONTERM: [[self.nonterms[0], EOF]]})
        self.firstSets = {'':[None]}
        self.followSets = {EOF: [None], STARTNONTERM: [None]}

        #term first sets
        for i in self.terms: self.firstSets.update({i: [i]})
        #nonterm first sets
        for n in self.nonterms: self.FIRST_sym(n)
        #symbol string first sets
        for p in self.prods:
            for x in self.prods[p]:
                if x == []: continue
                self.FIRST_prod(x)
        # follow sets for terms and nonterms
        for t in self.terms: 
            self.FOLLOW(t)
            #print(t, self.followSets)
        for n in self.nonterms: self.FOLLOW(n)

    def print(self) -> None:
        print("Term:",self.terms,"\nNonterm:",self.nonterms,"\nProds",self.prods)

    # get FIRST set of a single symbol
    def FIRST_sym(self, symbol: str) -> dict:
        if symbol in self.firstSets: return self.firstSets[symbol]
        self.firstSets.update({ symbol: [] })
        
        for p in self.prods[symbol]:
            # production is epsilon, add epsilon to FIRST set
            if p == [] and None not in self.firstSets[symbol]:
                self.firstSets[symbol] += [None]
            else:
                # check symbols in production sequentially
                for i in p:
                    #get FIRST set of current symbol in production
                    recursionBabyWOOOOO = self.FIRST_sym(i)

                    # if we're at the end of the production or i is not nullable, add FIRST(i) including epsilon if its there
                    # do not continue through symbol string
                    if None not in recursionBabyWOOOOO or p.index(i) == len(p) - 1:
                        for x in recursionBabyWOOOOO:
                            if x not in self.firstSets[symbol]:
                                self.firstSets[symbol] += [x]
                        break
                    # if we get here, i was nullable, add FIRST(i) excluding epsilon and continue through symbol string
                    for x in recursionBabyWOOOOO:
                        if x != None and x not in self.firstSets[symbol]:
                            self.firstSets[symbol] += [x]
        return self.firstSets[symbol]

    # get FIRST set of a symbol string
    def FIRST_prod(self, prod: list) -> None:
        prodStr = ' '.join(prod)
        if prodStr in self.firstSets: return
        self.firstSets.update({prodStr: []})
        for s in prod:
            # add FIRST(s)
            for x in self.firstSets[s]:
                if x not in self.firstSets[prodStr]: self.firstSets[prodStr] += [x]
            # if s is nullable and there are still more symbols in the production
            # then remove epsilon from the current first set and add first set of the production excluding the first symbol
            #
            # if there is only one symbol left and epsilon in FIRST(s), then the entire production is nullable
            # we keep epsilon if its there and dont recurse
            if None in self.firstSets[s] and len(prod) > 1:
                #remove epsilon and recurse
                self.firstSets[prodStr].remove(None)
                self.FIRST_prod(prod[1:])
                for x in self.firstSets[' '.join(prod[1:])]:
                    if x not in self.firstSets[prodStr]:
                        self.firstSets[prodStr] += [x]
            else: break

    # get FOLLOW set of a single symbol
    def FOLLOW(self,symbol: str, prev = []) -> dict:
        #print(symbol)
        if symbol not in self.followSets: self.followSets.update({symbol: []})

        for n in self.prods:
            for p in self.prods[n]:
                if symbol in p:
                    # get location of symbol in this production
                    index = p.index(symbol)
                    followZ = None
                    # Beta is empty, calculate FOLLOW(Z)
                    if index == len(p) - 1 and n not in prev:
                        followZ = self.FOLLOW(n, prev + [n])
                    else:
                        # get FIRST(Beta) and add it to FOLLOW(symbol)
                        self.FIRST_prod(p[index+1:])
                        bStr = ' '.join(p[index+1:])
                        for f in self.firstSets[bStr]:
                            if f != None and f not in self.followSets[symbol]: 
                                self.followSets[symbol] += [f]
                        # Beta is nullable, calculate FOLLOW(Z)
                        if None in self.firstSets[bStr] and n not in prev: 
                            followZ = self.FOLLOW(n, prev + [n])
                    # add FOLLOW(Z) if it was calculated
                    if followZ != None:
                        for z in followZ:
                            if z not in self.followSets[symbol]: self.followSets[symbol] += [z]

        return self.followSets[symbol]

def removeAction(p):
    if p == None: return p
    temp = []
    for y in p:
        if y[0] != '#': temp += [y]
    return temp

def addToTable(gr: Grammar, t, n: str, z, p) -> None:
    for x in gr.trueprods[n]:
        temp = removeAction(x)
        if temp == p:
            t[n][z] = x
            break
    return t

# generate selector table for a grammar
def table(gr: Grammar):
    # build empty table
    res = {n:{t:None for t in gr.terms} for n in gr.nonterms}
    for n in gr.nonterms:
        for p in gr.prods[n]:
            if len(p) > 0 and p[-1][0] == '#': pFirst = gr.firstSets[' '.join(p[:len(p)-1])]
            else: pFirst = gr.firstSets[' '.join(p)]
            placed = False
            for x in pFirst:
                if x in gr.terms:
                    # multiple symbols in same table index means not LL(1)
                    if res[n][x] != None and removeAction(res[n][x]) != p:
                        print("Bad Grammar")
                        exit(1)
                    res = addToTable(gr, res, n, x, p)
                    placed = True
            if not placed and None in pFirst:
                pFollow = gr.followSets[n]
                for x in pFollow:
                    if x in gr.terms:
                        # multiple symbols in same table index means not LL(1)
                        if res[n][x] != None and removeAction(res[n][x]) != p:
                            print("Bad Grammar")
                            exit(1)
                            return {}
                        res = addToTable(gr, res, n, x, p)
    return res
