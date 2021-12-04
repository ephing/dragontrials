#ifndef CSHANTY_X64OPT_H
#define CSHANTY_X64OPT_H

#include <unordered_map>
#include <list>

namespace cshanty {

class Quad;
class Procedure;
class IRProgram;

class QuadScanner {
public:
    static IRProgram* scanQuads(IRProgram*);
private:
    void sequenceQuads(std::list<Procedure*>*);
    void removeNops();
    bool removeJumps();
    void removeQuad(Quad*);

    std::unordered_map<Quad *, Quad *> prevQuad;
	std::unordered_map<Quad *, Quad *> nextQuad;
    IRProgram* myProg;
};

}

#endif