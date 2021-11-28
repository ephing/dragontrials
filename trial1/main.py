#!/usr/bin/env python3
import RegexEater as RE
import Automata as AM
import DFAPrinter as DP
import sys

# stored info from spec file
def buildRule(regex: str, action: str):
    out = {}
    out["regex"] = regex
    out["action"] = action
    r = RE.RegexEater()
    tree = r.parse(regex)
    d = AM.buildAutomata(tree)
    out["dfa"] = AM.toDict(AM.toDFA3(AM.epsremove(d)))
    return out

if len(sys.argv) != 2:
    print("Format: python3 main.py <spec>")
    print(sys.argv)
else:
    # read and parse spec file
    with open(sys.argv[1],"r") as file:
        with open("lexer","w") as outfile:
            outfile.write("#!/usr/bin/env python3\nimport lex\n\nlex.rules = {\n")
        count = 0
        line = file.readline().rstrip()
        while line:
            # (ERR)
            if line[-1] == '\"':
                action = "(ERR) " + line[line[:len(line)-1].rfind('\"'):]
                regex = line[:line[:len(line)-1].rfind('\"') - 7]
                with open("lexer", "a") as outfile:
                    outfile.write("    " + str(count) + ": " + str(buildRule(regex,action)) + ",\n")
            # (SKIP)
            elif line[-1] == ')':
                regex = line[:line.rfind('(') - 1]
                action = line[line.rfind('('):]
                with open("lexer", "a") as outfile:
                    outfile.write("    " + str(count) + ": " + str(buildRule(regex,action)) + ",\n")
            else:
                ind = line[:line.rfind(' ')].rfind(' ')
                with open("lexer", "a") as outfile:
                    outfile.write("    " + str(count) + ": " + str(buildRule(line[:ind],line[ind + 1:])) + ",\n")
            count += 1
            line = file.readline().rstrip()
        with open("lexer", "a") as outfile:
            outfile.write("}\n\n")
            outfile.write("lex.start()")
            
