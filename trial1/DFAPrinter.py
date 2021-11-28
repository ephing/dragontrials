import os

def makeDotFile(sig: list,delt: list,fin: list,stst: int,regex: str):
    if not os.path.exists('nfas/dots'):
        os.makedirs('nfas/dots')
    with open("nfas/dots/" + regex + ".dot","w+") as file:
        file.write("digraph dfa {\n    \"\" [shape=none]\n")
        for st in range(len(delt)):
            file.write("    \"" + str(st) + "\" [label=" + str(st))
            if st in fin: file.write(", shape=doublecircle]\n")
            else: file.write(", shape=circle]\n")
        
        file.write("    \"\" -> \"" + str(stst) + "\"\n")

        for st in range(len(delt)):
            for l in range(len(sig)):
                if delt[st][l] != []:
                    for d in delt[st][l]:
                        file.write("    \"" + str(st) + "\" -> \"" + str(d) + "\" [label=\"" + sig[l] + "\"]\n")

        file.write("}")
