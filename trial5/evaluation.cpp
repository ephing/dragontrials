#include "ast.hpp"
#include "symbol_table.hpp"
#include <limits>

using namespace cshanty;

static const Eval* ret = nullptr;

template <typename N>
N input(std::istream& in);

const Eval* GlobalStmtNode::eval(Environment* env) {
    return myStmt->eval(env);
}

const Eval* GlobalExpNode::eval(Environment* env) {
    return myExp->eval(env);
}

const Eval* AssignStmtNode::eval(Environment* env) {
    return myExp->eval(env);
}

const Eval* ReceiveStmtNode::eval(Environment* env) {
    if (myDst->getType()->isInt()) {
        int in = input<int>(std::cin);
        myDst->set(env, new IntEval(in));
    } else if (myDst->getType()->isBool()) {
        std::string in = input<std::string>(std::cin);
        if (in == "true" || in == "aye") {
            myDst->set(env, new BoolEval(true));
        } else if (in == "false" || in == "nay") {
            myDst->set(env, new BoolEval(false));
        } else throw new EvaluationError("Attempt to receive invalid value");
    } else {
        std::string in = input<std::string>(std::cin);
        myDst->set(env, new StrEval(in));
    }
    return new VoidEval();
}

const Eval* ReportStmtNode::eval(Environment* env) {
    const Eval* expEval = mySrc->eval(env);
    expEval->printResult();
    delete expEval;
    return new VoidEval();
}

const Eval* PostDecStmtNode::eval(Environment* env) {
    const IntEval* e = myLVal->eval(env)->asInt();
    const IntEval* res = new IntEval(e->result - 1);
    myLVal->set(env, res);
    delete e;
    return new VoidEval();
}

const Eval* PostIncStmtNode::eval(Environment* env) {
    const IntEval* e = myLVal->eval(env)->asInt();
    const IntEval* res = new IntEval(e->result + 1);
    myLVal->set(env, res);
    delete e;
    return new VoidEval();
}

const Eval* IfStmtNode::eval(Environment* env) {
    const BoolEval* condEval = myCond->eval(env)->asBool();
    Environment* n = new Environment();
    Environment* o = env->connect(n);
    if (condEval->result) {
        for (auto i : *myBody) {
            const Eval* e = i->eval(n);
            delete e;
            if (ret) break;
        }
    }
    env->trunc(o);
    delete condEval;
    return new VoidEval();
}

const Eval* IfElseStmtNode::eval(Environment* env) {
    const BoolEval* condEval = myCond->eval(env)->asBool();
    Environment* n = new Environment();
    Environment* o = env->connect(n);
    if (condEval->result) {
        for (auto i : *myBodyTrue) {
            const Eval* e = i->eval(n);
            delete e;
            if (ret) break;
        }
    } else {
        for (auto i : *myBodyFalse) {
            const Eval* e = i->eval(n);
            delete e;
            if (ret) break;
        }
    }
    env->trunc(o);
    delete condEval;
    return new VoidEval();
}

const Eval* WhileStmtNode::eval(Environment* env) {
    const BoolEval* condEval = myCond->eval(env)->asBool();
    Environment* n = new Environment();
    Environment* o = env->connect(n);
    while (condEval->result) {
        for (auto i : *myBody) {
            const Eval* e = i->eval(n);
            delete e;
        }
        delete condEval;
        if (ret) break;
        condEval = myCond->eval(n)->asBool();
    }
    env->trunc(o);
    delete condEval;
    return new VoidEval();
}

const Eval* ReturnStmtNode::eval(Environment* env) {
    if (myExp == nullptr) ret = new VoidEval();
    else ret = myExp->eval(env);
    return new VoidEval();
}

const Eval* CallStmtNode::eval(Environment* env) {
    return myCallExp->eval(env);
}

const Eval* CallExpNode::eval(Environment* env) {
    const ClosureEval* e = myID->eval(env)->asClosure();
    Environment* n = new Environment();
    Environment* o = e->env->connect(n);
    for (size_t i = 0; i < e->result->getFormals()->size(); i++) {
        n->add(e->result->getFormals()->at(i)->ID()->getName(),myArgs->at(i)->eval(env));
    }
    const Eval* res = e->result->evalBody(n);
    e->env->trunc(o);
    delete e;
    return res;
}

const Eval* AssignExpNode::eval(Environment* env) {
    const Eval* res = mySrc->eval(env);
    myDst->set(env,res);
    return res->clone();
}

const Eval* IntLitNode::eval(Environment* env) {
    return new IntEval(myNum);
}

const Eval* StrLitNode::eval(Environment* env) {
    return new StrEval(myStr);
}

const Eval* TrueNode::eval(Environment* env) {
    return new BoolEval(true);
}

const Eval* FalseNode::eval(Environment* env) {
    return new BoolEval(false);
}

const Eval* IDNode::eval(Environment* env) {
    const Eval* e = env->find(name);
    if (e == nullptr) throw new EvaluationError("ERROR: Attempt to get value from uninitialized variable");
    return e->clone();
}

const Eval* IndexNode::eval(Environment* env) {
    const RecordEval* base = myBase->eval(env)->asRec();
    const Eval* res = base->result->at(myIdx->getName())->clone();
    if (res == nullptr) 
        throw new EvaluationError("ERROR: Attempt to get value from uninitialized variable");
    delete base;
    return res;
}

const Eval* NegNode::eval(Environment* env) {
    const IntEval* expEval = myExp->eval(env)->asInt();
    const IntEval* res = new IntEval(-expEval->result);
    delete expEval;
    return res;
}

const Eval* NotNode::eval(Environment* env) {
    const BoolEval* expEval = myExp->eval(env)->asBool();
    const BoolEval* res = new BoolEval(!expEval->result);
    delete expEval;
    return res;
}

const Eval* PlusNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    const IntEval* res = new IntEval(exp1Eval->result + exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* MinusNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    const IntEval* res = new IntEval(exp1Eval->result - exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* TimesNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    const IntEval* res = new IntEval(exp1Eval->result * exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* DivideNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    if (!exp2Eval->result) {
        delete exp1Eval;
        delete exp2Eval;
        throw new EvaluationError("Divide by zero error");
    }
    const IntEval* res = new IntEval(exp1Eval->result / exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* AndNode::eval(Environment* env) {
    const BoolEval* exp1Eval = myExp1->eval(env)->asBool();
    const BoolEval* exp2Eval = myExp2->eval(env)->asBool();
    const BoolEval* res = new BoolEval(exp1Eval->result && exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* OrNode::eval(Environment* env) {
    const BoolEval* exp1Eval = myExp1->eval(env)->asBool();
    const BoolEval* exp2Eval = myExp2->eval(env)->asBool();
    const BoolEval* res = new BoolEval(exp1Eval->result || exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* EqualsNode::eval(Environment* env) {
    const Eval* exp1Eval = myExp1->eval(env);
    const Eval* exp2Eval = myExp2->eval(env);
    const BoolEval* res = new BoolEval(!exp1Eval->compare(exp2Eval));
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* NotEqualsNode::eval(Environment* env) {
    const Eval* exp1Eval = myExp1->eval(env);
    const Eval* exp2Eval = myExp2->eval(env);
    const BoolEval* res = new BoolEval(exp1Eval->compare(exp2Eval));
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* LessNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    const BoolEval* res = new BoolEval(exp1Eval->result < exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* LessEqNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    const BoolEval* res = new BoolEval(exp1Eval->result <= exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* GreaterNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    const BoolEval* res = new BoolEval(exp1Eval->result > exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* GreaterEqNode::eval(Environment* env) {
    const IntEval* exp1Eval = myExp1->eval(env)->asInt();
    const IntEval* exp2Eval = myExp2->eval(env)->asInt();
    const BoolEval* res = new BoolEval(exp1Eval->result >= exp2Eval->result);
    delete exp1Eval;
    delete exp2Eval;
    return res;
}

const Eval* VarDeclNode::eval(Environment* env) {
    if (const RecordType* r = myID->getSymbol()->getDataType()->asRecord()) {
        HashMap<std::string, const Eval*>* f = new HashMap<std::string,const Eval*>();
        for (auto i : *r->getFields()) {
            f->insert(std::pair<std::string,const Eval*>(i.first, nullptr));
        }
        env->add(myID->getName(), new RecordEval(f));
    } else env->add(myID->getName(), nullptr);
    return new VoidEval();
}

const Eval* RecordTypeDeclNode::eval(Environment* env) {
    return new VoidEval();
}

const Eval* FnDeclNode::eval(Environment* env) {
    env->add(myID->getName(), new ClosureEval(this,env));
    return new VoidEval();
}

const Eval* FnDeclNode::evalBody(Environment* env) {
    for (auto i : *myBody) {
        const Eval* e = i->eval(env);
        delete e;
        if (ret) break;
    }
    if (!ret && !myRetType->getType()->isVoid())
        throw new EvaluationError("Reached end of non-void function but didn't return");
    const Eval* e = ret ? ret : new VoidEval();
    ret = nullptr;
    return e;
}

template <typename N>
N input(std::istream& in) {
    N res;
    in >> res;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(numeric_limits<streamsize>::max(),'\n');
        throw new EvaluationError("Attempt to receive invalid value");
    }
    return res;
}