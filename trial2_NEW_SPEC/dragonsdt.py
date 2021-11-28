import reader
import tablebuilder
import cppcode
import sys

cfile = "translator.cpp"

def main(filename):
    table = tablebuilder.table(reader.readGrammar(filename))
    for x in table: print(x,table[x])
    with open(cfile, "w") as file:
        file.write("#include <map>\n#include <vector>\n#include <fstream>\n#include <algorithm>\n#include <stack>\nconst char* look = \"\";\n")
        file.write(reader.initcode)
        file.write("\nvoid runActions(int act) {\n\tswitch (act) {\n")
        for x in reader.actions:
            file.write(x)
        file.write("\n\t}\n}\n\n")
        file.write("std::vector<std::string> terms, nonterms;\nint lookahead = 0;\n")
        file.write(cppcode.selectable(table))
        file.write(cppcode.token)
        file.write(cppcode.llparser)
        file.write(cppcode.cmain)

main(sys.argv[1])