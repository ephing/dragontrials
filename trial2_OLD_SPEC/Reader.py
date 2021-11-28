from tablebuilder import Grammar

lamList = {}

def buildLam(sem,i):
    vars = {}
    for x in sem:
        if x[0] == '$': vars.update({x: 'v' + str(len(vars))})
    lamstr = ''
    exp = ''
    for x in sem:
        if x in vars: 
            lamstr = lamstr + "lambda " + vars[x] + ":"
            exp = exp + vars[x]
        else: exp = exp + x
    lamstr = lamstr + exp
    lamList[i] = lamstr
    
def readGrammar(filename):
    currentNonterm = None
    terms = []
    nonterms = []
    prods = {}
    with open(filename,"r") as file:
        line = file.readline()
        while line != '':
            line = line.lstrip().rstrip().split(" ")
            while '' in line: line.remove('')
            if (line[-1] == '}'): 
                lamList.update({len(lamList)+1:line[line.index('{')+1:-1]})
                line = line[:line.index('{')]
            else: lamList.update({len(lamList)+1:None})
            if (line[0] != ''): 
                if (line[0] != '|'):
                    currentNonterm = line[0]
                    if currentNonterm not in nonterms:
                        nonterms.append(currentNonterm)
                        if currentNonterm in terms: terms.remove(currentNonterm)
                        prods.update({currentNonterm: []})
                    prods[currentNonterm].append(line[2:])
                    for i in range(2,len(line)):
                        if line[i] not in nonterms and line[i] not in terms:
                            terms.append(line[i])
                else:
                    prods[currentNonterm].append(line[1:])   
                    for i in range(1,len(line)):
                        if line[i] not in nonterms and line[i] not in terms:
                            terms.append(line[i])             
            line = file.readline()
    for x in lamList: 
        if lamList[x] != None: buildLam(lamList[x],x)
    lamList.update({len(lamList)+1:None})
    return Grammar(terms,nonterms,prods)