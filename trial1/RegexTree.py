
class Tree:
    def __init__(self, reg: str):
        self.regex = reg

    def print(self):
        return self.regex

class Split(Tree):
    def __init__(self, t1: Tree, t2: Tree):
        self.ch1 = t1
        self.ch2 = t2

    def print(self):
        out1 = "None"
        out2 = "None"
        if self.ch1 != None: out1 = self.ch1.print()
        if self.ch2 != None: out2 = self.ch2.print()
        return "Split(" + out1 + ", " + out2 + ")"

class Concat(Tree):
    def __init__(self, t1: Tree, t2: Tree):
        self.t1 = t1
        self.t2 = t2

    def print(self):
        out1 = "None"
        out2 = "None"
        if self.t1 != None: out1 = self.t1.print()
        if self.t2 != None: out2 = self.t2.print()
        return "Concat(" + out1 + ", " + out2 + ")"

class MeanKleene(Tree):
    def __init__(self,t: str):
        self.rep = t

    def print(self):
        out = "None"
        if self.rep != None: out = self.rep.print()
        return "Rep(" + self.rep.print() + ")"

class Prim(Tree):
    def __init__(self,c: str, isW: bool):
        self.regex = c
        self.isWildcard = isW

    def print(self):
        return self.regex
