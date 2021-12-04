#include "x64opt.hpp"
#include "3ac.hpp"
#include <algorithm>

using namespace cshanty;

IRProgram* QuadScanner::scanQuads(IRProgram* prog) {
    QuadScanner s;
    s.myProg = prog;

    s.sequenceQuads(prog->getProcs());

    s.removeNops();
    bool changed = true;
    while (changed) changed = s.removeJumps();

    return s.myProg;
}

void QuadScanner::sequenceQuads(std::list<Procedure*>* procs) {
    Quad * trail = nullptr;
    for (auto p : *procs) {
        EnterQuad * enter = p->getEnter();
        LeaveQuad * leave = p->getLeave();

        if (p->getQuads()->empty()){
            nextQuad.insert({enter,leave});
            prevQuad.insert({leave,enter});
            trail = leave;
            continue;
        }
        for (auto lead : *(p->getQuads())){
            if (trail == nullptr){
                trail = lead;
                nextQuad.insert({enter,lead});
                prevQuad.insert({lead,enter});
                continue;
            }

            nextQuad.insert({trail,lead});
            prevQuad.insert({lead,trail});
            trail = lead;
        }
        nextQuad.insert({trail,leave});
        prevQuad.insert({leave,trail});
        trail = leave;
    }
}

void QuadScanner::removeNops() {
    Quad* current = myProg->getProcs()->front()->getEnter();

    std::set<Quad*> nops;

    while (current != myProg->getProcs()->back()->getLeave()) {
        if (auto q = dynamic_cast<NopQuad*>(current)) {
            nops.insert(current);
        }
        current = nextQuad[current];
    }

    for (Quad* q : nops) removeQuad(q);
}

bool QuadScanner::removeJumps() {
    Quad* current = myProg->getProcs()->front()->getEnter();

    std::set<Quad*> badjmps;
    bool didSomething = false;

    while (current != myProg->getProcs()->back()->getLeave()) {
        if (auto q = dynamic_cast<GotoQuad*>(current)) {
            if (nextQuad[current]->hasLabel(q->getTarget())) {
                badjmps.insert(current);
                didSomething = true;
            }
        }
        else if (auto q = dynamic_cast<IfzQuad*>(current)) {
            if (nextQuad[current]->hasLabel(q->getTarget())) {
                badjmps.insert(current);
                didSomething = true;
            }
        }
        current = nextQuad[current];
    }
    for (Quad* q : badjmps) removeQuad(q);

    return didSomething;
}

void QuadScanner::removeQuad(Quad* q) {
    for (auto i : *myProg->getProcs()) {
        auto t = std::find(i->getQuads()->begin(),i->getQuads()->end(),q);
        if (t != i->getQuads()->end()) {
            (*t)->transferLabels(nextQuad[q]);
            i->getQuads()->remove(q);
            break;
        }
    }

    Quad* theQuad = q;
    nextQuad[prevQuad[q]] = nextQuad[q];
    prevQuad[nextQuad[q]] = prevQuad[q];

    delete theQuad;
}