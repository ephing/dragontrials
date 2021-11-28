import RegexTree as RT

class RegexEater:
    def __init__(self):
        self.index = 0
        self.input = RT.Tree("")

    def parse(self,regex: str):
        self.input = RT.Tree(regex)
        self.index = 0
        out = self.regex()
        return out

    # peek at next character without consuming
    def peek(self):
        if (self.index < len(self.input.regex)): return self.input.regex[self.index]
        else: raise Exception("Uh oh bad index")

    # EAT
    def consume(self, c: str):
        if (self.peek() == c): self.index += 1
        else: raise Exception("Food bad")

    # peek 'n consume
    def pnc(self):
        c = self.peek()
        self.consume(c)
        return c

    # attempts to get a term consisting only of continuous concatenated primitives
    def getTerm(self):
        conc = None

        while (self.index < len(self.input.regex) and self.peek() != ')' and self.peek() != '|'):
            next = self.kleenes()
            if (conc == None): conc = next
            else: conc = RT.Concat(conc, next)

        return conc

    # gets the thing to be operated on (b) and handles *, +, and ? operaters on that base
    def kleenes(self):
        b = self.base()

        while (self.index < len(self.input.regex)):
            c = self.peek()
            if (c == '*'):
                self.consume('*')
                b = RT.MeanKleene(b)
            elif c == '+':
                self.consume('+')
                b = RT.Concat(b, RT.MeanKleene(b))
            elif c == '?':
                self.consume('?')
                b = RT.Split(b,None)
            else: break

        return b

    # handles grouping () and [], as well as escape sequences
    def base(self):
        c = self.peek()
        if c == '(':
            self.consume('(')
            r = self.regex()
            self.consume(')')
            return r
        elif c == '[':
            self.consume('[')
            r = self.charSel()
            self.consume(']')
            return r
        elif c == '\\':
            self.consume('\\')
            esc = self.pnc()
            if esc == 't': return RT.Prim('\t', False)
            elif esc == 'n': return RT.Prim('\n', False)
            elif esc == '\'': return RT.Prim('\'', False)
            elif esc == '\"': return RT.Prim('\"', False)
            elif esc == '\\': return RT.Prim('\\', False)
            elif esc == '_': return RT.Prim(' ', False)
            else: raise Exception("Invalid escape sequence in regex: " + esc)
        elif c == '.':
            return RT.Prim(self.pnc(), True)
        else:
            return RT.Prim(self.pnc(), False)

    # handles [], and its inner operators - and ^
    def charSel(self):
        t = None
        carrot = self.peek() == '^'
        chars = []
        while self.index < len(self.input.regex) and self.peek() != ']':
            c = self.pnc()
            if c == '\\':
                esc = self.pnc()
                if esc == 't': c = '\t'
                elif esc == 'n': c = '\n'
                elif esc == '\'': c = '\''
                elif esc == '\"': c = '\"'
                elif esc == '\\': c = '\\'
                elif esc == '_': c = ' '
                else: raise Exception("Invalid escape sequence in regex: " + esc)
            chars.append(c)
            if self.peek() == '-':
                self.consume('-')
                if self.peek() == ']': chars.append('-')
                else:
                    if c <= self.peek():
                        for i in range(ord(c) + 1, ord(self.peek()) + 1):
                            chars.append(chr(i))
                    self.consume(self.peek())
        for i in ['\t','\n'] + [chr(x) for x in range(32,127)]:
            if carrot != (i in chars):
                if t == None: t = RT.Prim(i, False)
                else: t = RT.Split(t,RT.Prim(i, False))
        return t

    # gets a term, handles alternation with next term if needed
    def regex(self):
        t = self.getTerm()

        if self.index < len(self.input.regex) and self.peek() == '|':
            self.consume('|')
            reg = self.regex()
            return RT.Split(t,reg)
        return t
