from tablebuilder import Grammar

initcode = ""
actions = []

def readGrammar(filename):
    global initcode,actions
    currentNonterm = None
    terms = []
    nonterms = []
    ac = []
    prods = {}
    with open(filename, "r") as file:
        line = file.readline().rstrip()
        while list(line) != ['%','%']:
            line = line.lstrip().split(" ")
            while '' in line: line.remove('')
            if line[0] != '|':
                currentNonterm = line[0]
                if currentNonterm not in nonterms:
                    nonterms += [currentNonterm]
                    if currentNonterm in terms: terms.remove(currentNonterm)
                    prods.update({currentNonterm: []})
                prods[currentNonterm] += [line[2:]]
                for i in range(2,len(line)):
                    if line[i] not in nonterms:
                        if line[i][0] != '#' and line[i] not in terms:
                            terms += [line[i]]
                        elif line[i][0] == '#' and line[i] not in ac:
                            ac += [line[i]]
            else:
                prods[currentNonterm] += [line[1:]]
                for i in range(1,len(line)):
                    if line[i] not in nonterms:
                        if line[i][0] != '#' and line[i] not in terms:
                            terms += [line[i]]
                        elif line[i][0] == '#' and line[i] not in ac:
                            ac += [line[i]]
            line = file.readline().rstrip()

        line = file.readline()
        while list(line) != ['%','%'] and line != '':
            initcode += line + "\n"
            line = file.readline().rstrip()

        line = file.readline()
        while line != '':
            num = line.split(" ")[0][1:]
            actions += ["\tcase " + num + ": {\n\t\t" + ' '.join(line.split(" ")[2:-1]) + "\n\t\treturn;\n\t}\n"]
            line = file.readline().rstrip()

        return Grammar(terms, nonterms, prods, ac)