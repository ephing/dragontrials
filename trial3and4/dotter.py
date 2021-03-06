from Reader import STARTNONTERM,EOF
import SLR

def makedot():
    with open("parser.dot","w+") as file:
        file.write("digraph dfa {\n    \"\" [shape=none]\n")
        t = 0
        for s in SLR.states:
            file.write("    \"" + str(t) + "\" [shape=box,label=\"" + getStuff(SLR.states[s]) + "\"]\n")
            t += 1
        t = 0
        file.write("    \"\" -> \"0\"")
        for s in SLR.states:
            for z in SLR.states[s].outTokens:
                file.write("    \"" + str(t) + "\" -> \"" + str(SLR.states[s].outTokens[z]) + "\" [label=\""+z+"\"]\n")
            t += 1
        file.write("}")

def getStuff(s: SLR.State):
    out = ""
    for i in s.items:
        out += i.asStr().replace(STARTNONTERM, "S'").replace(EOF,"") + "\\n"
    return out