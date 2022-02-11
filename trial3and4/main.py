import Reader as r
import sys
import SLR
import dotter

def main(filename):
    r.readGrammar(filename)
    seed = SLR.Seed(r.STARTNONTERM,[],r.grammar.prods[r.STARTNONTERM][0])
    SLR.State(seed)
    t = SLR.table()
    with open("parser","w+") as file:
        file.write("#!/usr/bin/env python3\nimport parse as p\nimport sys\n\n")
        file.write("p.table = " + str(t) + "\np.start = list(p.table.keys())[0]\n")
        file.write("parser = p.Parser(sys.argv[1])\n")
        file.write("parser.parse()\n")
        
# if len(sys.argv) == 2:
#     main(sys.argv[1])
# elif len(sys.argv) == 3:
#     if sys.argv[1] == '-d':
#         main(sys.argv[2])
#         dotter.makedot()
#     else:
#         print("Your flag bad (-d to dot)")
# else:
#     print("Format: python3 main.py <-d> <.ag>")

main("/home/joemama/Documents/lastsem/compiler/dragontrials/trial3and4/logic.grammar")
dotter.makedot()

