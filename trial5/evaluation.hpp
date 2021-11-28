#ifndef CSHANTY_AST_EVALUATION
#define CSHANTY_AST_EVALUATION

#include <string>
#include <unordered_map>
#include "errors.hpp"

template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;

namespace cshanty {

class Eval;
class IntEval;
class BoolEval;
class StrEval;
class VoidEval;
class ClosureEval;
class RecordEval;
class FnDeclNode;
class Environment;

class Eval {
public:
    Eval() {}
    virtual ~Eval() {}
    virtual void printResult() const = 0;
    virtual const IntEval* asInt() const { return nullptr; }
    virtual const StrEval* asStr() const { return nullptr; }
    virtual const BoolEval* asBool() const { return nullptr; }
    virtual const VoidEval* asVoid() const { return nullptr; }
    virtual const ClosureEval* asClosure() const { return nullptr; }
    virtual const RecordEval* asRec() const { return nullptr; }
    virtual int compare(const Eval*) const = 0;
    virtual const Eval* clone() const = 0;
};

class StrEval : public Eval {
public:
    StrEval(std::string r) : Eval(), result(r) {}
    void printResult() const override { std::cout << result << "\n"; }
    const StrEval* asStr() const { return this; }
    int compare(const Eval* e) const override {
        if (const StrEval* s = e->asStr()) {
            if (result == s->result) return 0;
            else if (result.size() > s->result.size()) return 1;
            else if (result.size() < s->result.size()) return -1;
            else {
                for (size_t i = 0; i < result.size(); i++) {
                    if (result[i] - s->result[i]) return result[i] - s->result[i];
                }
            }
        }
        throw new InternalError("Attempt to compare string to non-string but after type analysis succeeded somehow");
    }
    const Eval* clone() const override { return new StrEval(result); }
    std::string result;
};

class IntEval : public Eval {
public:
    IntEval(int r) : Eval(), result(r) {}
    void printResult() const override { std::cout << result << "\n"; }
    const IntEval* asInt() const { return this; }
    int compare(const Eval* e) const override {
        if (const IntEval* s = e->asInt()) {
            return result - s->result;
        }
        throw new InternalError("Attempt to compare int to non-int but after type analysis succeeded somehow");
    }
    const Eval* clone() const override { return new IntEval(result); }
    int result;
};

class BoolEval : public Eval {
public:
    BoolEval(bool r) : Eval(), result(r) {}
    void printResult() const override { std::cout << (result ? "aye\n" : "nay\n"); }
    const BoolEval* asBool() const { return this; }
    int compare(const Eval* e) const override {
        if (const BoolEval* s = e->asBool()) {
            if (result == s->result) return 0;
            else return 1;
        }
        throw new InternalError("Attempt to compare bool to non-bool but after type analysis succeeded somehow");
        return 0;
    }
    const Eval* clone() const override { return new BoolEval(result); }
    bool result;
};

class VoidEval : public Eval {
public:
    VoidEval() : Eval() {}
    void printResult() const override {}
    int compare(const Eval* e) const override {
        throw new InternalError("Attempt to compare voids what are you even doing");
        return 0;
    }
    const Eval* clone() const override { return new VoidEval(); }
    const VoidEval* asVoid() const { return this; }
};

class ClosureEval : public Eval {
public:
    ClosureEval(FnDeclNode* f, Environment* e) : Eval(), result(f), env(e) {}
    void printResult() const override { std::cout << result << "\n"; }
    const ClosureEval* asClosure() const override { return this; }
    int compare(const Eval* e) const override {
        throw new InternalError("Attempt to compare functions what are you even doing");
        return 0;
    }
    const Eval* clone() const override { return new ClosureEval(result, env); }
    FnDeclNode* result;
    Environment* env;
};

class RecordEval : public Eval {
public:
    RecordEval(HashMap<std::string,const Eval*>* r) : Eval(), result(r) {}
    virtual ~RecordEval() { 
        for (auto i : *result) delete i.second;
        delete result; 
    }
    void printResult() const override {
        std::cout << "\n";
        for (auto i : *result) {
            std::cout << "\t" << i.first << ": ";
            if (i.second == nullptr) std::cout << "nullptr\n";
            else i.second->printResult();
        }
    }
    const RecordEval* asRec() const { return this; }
    int compare(const Eval* e) const override {
        throw new InternalError("Attempt to compare records what are you even doing");
        return 0;
    }
    const Eval* clone() const override { 
        HashMap<std::string,const Eval*>* f = new HashMap<std::string,const Eval*>();
        for (auto i : *result) f->insert(std::pair<std::string,const Eval*>(i.first,i.second->clone()));
        return new RecordEval(f); 
    }
    HashMap<std::string,const Eval*>* result;
};

}

#endif