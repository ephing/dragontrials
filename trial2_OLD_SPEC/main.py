#!/usr/bin/env python3
import Reader as r
import tablebuilder as tb

cfg = r.readGrammar("grammar.ag")
cfgdata = {'t':cfg.terms,'n':cfg.nonterms}

table = tb.table(cfg)
if table == []: exit(2)

with open("translator","w") as file:
    file.write("#!/usr/bin/env python3\nimport parser as p\n\np.table =")
    file.write(str(table))
    file.write("\np.terminals = ")
    file.write(str(cfgdata))
    file.write("[\'t\']\np.nonterminals = ")
    file.write(str(cfgdata))
    file.write("[\'n\']\np.lamList = {")
    for x in r.lamList: file.write(str(x) + ": " + str(r.lamList[x]) +", ")
    file.write("}\np.start()")