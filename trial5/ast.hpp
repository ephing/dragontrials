#ifndef CSHANTY_AST_HPP
#define CSHANTY_AST_HPP

#include <ostream>
#include <sstream>
#include <string.h>
#include <vector>
#include "tokens.hpp"
#include "types.hpp"
#include "evaluation.hpp"
#include "environment.hpp"
#include "symbol_table.hpp"

namespace cshanty {

class TypeAnalysis;

class Opd;

class SymbolTable;
class SemSymbol;

class DeclNode;
class VarDeclNode;
class StmtNode;
class AssignExpNode;
class FormalDeclNode;
class TypeNode;
class ExpNode;
class LValNode;
class IDNode;

class ASTNode{
public:
	ASTNode(Position * pos) : myPos(pos){ }
	virtual ~ASTNode() {}
	Position * pos() { return myPos; };
	std::string posStr(){ return pos()->span(); }
	virtual bool isGlobEval() const { return false; }
	virtual bool isVarDecl() const { return false; }
	virtual bool nameAnalysis(SymbolTable *) = 0;
protected:
	Position * myPos = nullptr;
};

class StmtNode : public ASTNode{
public:
	StmtNode(Position * p) : ASTNode(p){ }
	virtual ~StmtNode() {}
	virtual void typeAnalysis(TypeAnalysis *) = 0;
	virtual const Eval* eval(Environment*) = 0;
};

class DeclNode : public StmtNode{
public:
	DeclNode(Position * p) : StmtNode(p){ }
	virtual ~DeclNode() {}
	virtual void typeAnalysis(TypeAnalysis *) override = 0;
};

class ProgramNode : public ASTNode{
public:
	ProgramNode(std::vector<DeclNode *> * globalsIn);
	virtual ~ProgramNode() {
		for (auto i : *myGlobals) delete i;
		delete myGlobals;
		delete myPos;
	}
	std::vector<DeclNode*>** globs() { return &myGlobals; }
	virtual bool nameAnalysis(SymbolTable *) override;
	virtual void typeAnalysis(TypeAnalysis *);
private:
	std::vector<DeclNode *> * myGlobals;
};

class ExpNode : public ASTNode{
protected:
	ExpNode(Position * p) : ASTNode(p){ }
public:
	virtual ~ExpNode() {}
	virtual bool nameAnalysis(SymbolTable * symTab) override = 0;
	virtual void typeAnalysis(TypeAnalysis *) = 0;
	virtual const Eval* eval(Environment*) = 0;
};

class LValNode : public ExpNode{
public:
	LValNode(Position * p) : ExpNode(p){}
	virtual ~LValNode() { delete myPos; }
	bool nameAnalysis(SymbolTable * symTab) override { return false; }
	virtual void typeAnalysis(TypeAnalysis *) override {} 
	virtual const DataType* getType() const { return nullptr; }
	virtual void set(Environment*,const Eval*) = 0;
};

class IDNode : public LValNode{
public:
	IDNode(Position * p, std::string nameIn)
	: LValNode(p), name(nameIn), mySymbol(nullptr){}
	std::string getName(){ return name; }
	void attachSymbol(SemSymbol * symbolIn);
	SemSymbol * getSymbol() const { return mySymbol; }
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
	const DataType* getType() const override { return getSymbol()->getDataType(); }
	void set(Environment* env,const Eval* res) override {
		env->set(name,res);
	}
	void desSymbol() {
		delete mySymbol;
	}
private:
	std::string name;
	SemSymbol * mySymbol;
};

class IndexNode : public LValNode{
public:
	IndexNode(Position * p, IDNode * base, IDNode * idx)
	: LValNode(p), myBase(base), myIdx(idx){ }
	~IndexNode() {
		delete myBase;
		delete myIdx;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
	const DataType* getType() const override { 
		return myBase->getSymbol()->getDataType()->asRecord()->getField(myIdx->getName());
	}
	void set(Environment* env,const Eval* res) override {
		const RecordEval* type = env->find(myBase->getName())->asRec();
		if (type->result->at(myIdx->getName())) delete type->result->at(myIdx->getName());
		type->result->at(myIdx->getName()) = res;
	}
private:
	IDNode * myBase;
	IDNode * myIdx;
};

class TypeNode : public ASTNode{
public:
	TypeNode(Position * p) : ASTNode(p){ }
	virtual ~TypeNode() {}
	virtual const DataType * getType() = 0;
	virtual bool nameAnalysis(SymbolTable *) override;
	virtual void typeAnalysis(TypeAnalysis *);
};

class RecordTypeNode : public TypeNode{
public:
	RecordTypeNode(Position * p, IDNode * IDin)
	:TypeNode(p), myID(IDin) { }
	~RecordTypeNode() {
		delete myID;
	}
	virtual const DataType * getType() override { return myType; }
	virtual bool nameAnalysis(SymbolTable *) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
private:
	IDNode * myID;
	const RecordType * myType;
};

class VarDeclNode : public DeclNode{
public:
	VarDeclNode(Position * p, TypeNode * typeIn, IDNode * IDIn)
	: DeclNode(p), myType(typeIn), myID(IDIn){ }
	~VarDeclNode() {
		myID->desSymbol();
		delete myType;
		delete myID;
		delete myPos;
	}
	IDNode * ID(){ return myID; }
	TypeNode * getTypeNode(){ return myType; }
	bool isVarDecl() const override { return true; }
	bool nameAnalysis(SymbolTable * symTab) override;
	void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	TypeNode * myType;
	IDNode * myID;
};

class RecordTypeDeclNode : public DeclNode{
public:
	RecordTypeDeclNode(Position *p, IDNode *id, std::vector<VarDeclNode*> *body)
	: DeclNode(p), myID(id), myFields(body){ }
	~RecordTypeDeclNode() {
		myID->desSymbol();
		delete myID;
		for (auto i : *myFields) delete i;
		delete myFields;
		delete myPos;
	}
	TypeNode * getTypeNode(){ return nullptr; }
	bool nameAnalysis(SymbolTable * symTab) override;
	void typeAnalysis(TypeAnalysis * typing) override;
	const Eval* eval(Environment*) override;
private:
	IDNode * myID;
	std::vector<VarDeclNode *> * myFields;
};

class FormalDeclNode : public VarDeclNode{
public:
	FormalDeclNode(Position * p, TypeNode * type, IDNode * id) 
	: VarDeclNode(p, type, id){ }
};

class FnDeclNode : public DeclNode{
public:
	FnDeclNode(Position * p, 
	  TypeNode * retTypeIn, IDNode * idIn,
	  std::vector<FormalDeclNode *> * formalsIn,
	  std::vector<StmtNode *> * bodyIn)
	: DeclNode(p), myRetType(retTypeIn), myID(idIn),
	  myFormals(formalsIn), myBody(bodyIn){ 
	}
	~FnDeclNode() {
		myID->desSymbol();
		delete myRetType;
		delete myID;
		for (auto i : *myFormals) delete i;
		delete myFormals;
		for (auto i : *myBody) delete i;
		delete myBody;
		delete myPos;
	}
	IDNode * ID() const { return myID; }
	std::vector<FormalDeclNode *> * getFormals() const{
		return myFormals;
	}
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	virtual TypeNode * getRetTypeNode() { 
		return myRetType;
	}
	const Eval* eval(Environment*) override;
	const Eval* evalBody(Environment*);
private:
	TypeNode * myRetType;
	IDNode * myID;
	std::vector<FormalDeclNode *> * myFormals;
	std::vector<StmtNode *> * myBody;
};

class AssignExpNode : public ExpNode{
public:
	AssignExpNode(Position * p, LValNode * dstIn, ExpNode * srcIn)
	: ExpNode(p), myDst(dstIn), mySrc(srcIn){ }
	~AssignExpNode() {
		delete myDst;
		delete mySrc;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	LValNode * myDst;
	ExpNode * mySrc;
};

class AssignStmtNode : public StmtNode{
public:
	AssignStmtNode(Position * p, AssignExpNode * expIn)
	: StmtNode(p), myExp(expIn){ }
	~AssignStmtNode() { 
		delete myExp;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	AssignExpNode * myExp;
};

class ReceiveStmtNode : public StmtNode{
public:
	ReceiveStmtNode(Position * p, LValNode * dstIn)
	: StmtNode(p), myDst(dstIn){ }
	~ReceiveStmtNode() {
		delete myDst;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	LValNode * myDst;
};

class ReportStmtNode : public StmtNode{
public:
	ReportStmtNode(Position * p, ExpNode * srcIn)
	: StmtNode(p), mySrc(srcIn){ }
	~ReportStmtNode() { 
		delete mySrc;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	ExpNode * mySrc;
};

class PostDecStmtNode : public StmtNode{
public:
	PostDecStmtNode(Position * p, LValNode * lvalIn)
	: StmtNode(p), myLVal(lvalIn){ }
	~PostDecStmtNode() { 
		delete myLVal;
		delete myPos;
	}
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	LValNode * myLVal;
};

class PostIncStmtNode : public StmtNode{
public:
	PostIncStmtNode(Position * p, LValNode * lvalIn)
	: StmtNode(p), myLVal(lvalIn){ }
	~PostIncStmtNode() { 
		delete myLVal;
		delete myPos;
	}
	virtual bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	LValNode * myLVal;
};

class IfStmtNode : public StmtNode{
public:
	IfStmtNode(Position * p, ExpNode * condIn,
	  std::vector<StmtNode *> * bodyIn)
	: StmtNode(p), myCond(condIn), myBody(bodyIn){ }
	~IfStmtNode() {
		delete myCond;
		for (auto i : *myBody) delete i;
		delete myBody;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	ExpNode * myCond;
	std::vector<StmtNode *> * myBody;
};

class IfElseStmtNode : public StmtNode{
public:
	IfElseStmtNode(Position * p, ExpNode * condIn, 
	  std::vector<StmtNode *> * bodyTrueIn,
	  std::vector<StmtNode *> * bodyFalseIn)
	: StmtNode(p), myCond(condIn),
	  myBodyTrue(bodyTrueIn), myBodyFalse(bodyFalseIn) { }
	~IfElseStmtNode() {
		delete myCond;
		for (auto i : *myBodyFalse) delete i;
		for (auto i : *myBodyTrue) delete i;
		delete myBodyTrue;
		delete myBodyFalse;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	ExpNode * myCond;
	std::vector<StmtNode *> * myBodyTrue;
	std::vector<StmtNode *> * myBodyFalse;
};

class WhileStmtNode : public StmtNode{
public:
	WhileStmtNode(Position * p, ExpNode * condIn, 
	  std::vector<StmtNode *> * bodyIn)
	: StmtNode(p), myCond(condIn), myBody(bodyIn){ }
	~WhileStmtNode() {
		delete myCond;
		for (auto i : *myBody) delete i;
		delete myBody;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	ExpNode * myCond;
	std::vector<StmtNode *> * myBody;
};

class ReturnStmtNode : public StmtNode{
public:
	ReturnStmtNode(Position * p, ExpNode * exp)
	: StmtNode(p), myExp(exp){ }
	~ReturnStmtNode() { 
		delete myExp;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	ExpNode * myExp;
};

class CallExpNode : public ExpNode{
public:
	CallExpNode(Position * p, IDNode * id,
	  std::vector<ExpNode *> * argsIn)
	: ExpNode(p), myID(id), myArgs(argsIn){ }
	~CallExpNode() {
		delete myID;
		for (auto i : *myArgs) delete i;
		delete myArgs;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	void typeAnalysis(TypeAnalysis *) override;
	DataType * getRetType();
	const Eval* eval(Environment*) override;
private:
	IDNode * myID;
	std::vector<ExpNode *> * myArgs;
};

class BinaryExpNode : public ExpNode{
public:
	BinaryExpNode(Position * p, ExpNode * lhs, ExpNode * rhs)
	: ExpNode(p), myExp1(lhs), myExp2(rhs) { }
	virtual ~BinaryExpNode() {
		delete myExp1;
		delete myExp2;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override = 0;
protected:
	ExpNode * myExp1;
	ExpNode * myExp2;
	void binaryLogicTyping(TypeAnalysis * typing);
	void binaryEqTyping(TypeAnalysis * typing);
	void binaryRelTyping(TypeAnalysis * typing);
	void binaryMathTyping(TypeAnalysis * typing);
};

class PlusNode : public BinaryExpNode{
public:
	PlusNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class MinusNode : public BinaryExpNode{
public:
	MinusNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class TimesNode : public BinaryExpNode{
public:
	TimesNode(Position * p, ExpNode * e1In, ExpNode * e2In)
	: BinaryExpNode(p, e1In, e2In){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class DivideNode : public BinaryExpNode{
public:
	DivideNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class AndNode : public BinaryExpNode{
public:
	AndNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class OrNode : public BinaryExpNode{
public:
	OrNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class EqualsNode : public BinaryExpNode{
public:
	EqualsNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
	
};

class NotEqualsNode : public BinaryExpNode{
public:
	NotEqualsNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
	
};

class LessNode : public BinaryExpNode{
public:
	LessNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class LessEqNode : public BinaryExpNode{
public:
	LessEqNode(Position * pos, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(pos, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class GreaterNode : public BinaryExpNode{
public:
	GreaterNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class GreaterEqNode : public BinaryExpNode{
public:
	GreaterEqNode(Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class UnaryExpNode : public ExpNode {
public:
	UnaryExpNode(Position * p, ExpNode * expIn) 
	: ExpNode(p){
		this->myExp = expIn;
	}
	virtual ~UnaryExpNode() { 
		delete myExp;
		delete myPos;
	}
	virtual bool nameAnalysis(SymbolTable * symTab) override = 0;
	virtual void typeAnalysis(TypeAnalysis *) override = 0;
protected:
	ExpNode * myExp;
};

class NegNode : public UnaryExpNode{
public:
	NegNode(Position * p, ExpNode * exp)
	: UnaryExpNode(p, exp){ }
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class NotNode : public UnaryExpNode{
public:
	NotNode(Position * p, ExpNode * exp)
	: UnaryExpNode(p, exp){ }
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class VoidTypeNode : public TypeNode{
public:
	VoidTypeNode(Position * p) : TypeNode(p){}
	~VoidTypeNode() { delete myPos; }
	virtual const DataType * getType()override { 
		return BasicType::VOID(); 
	}
};

class IntTypeNode : public TypeNode{
public:
	IntTypeNode(Position * p): TypeNode(p){}
	~IntTypeNode() { delete myPos; }
	virtual const DataType * getType() override;
};

class BoolTypeNode : public TypeNode{
public:
	BoolTypeNode(Position * p): TypeNode(p) { }
	~BoolTypeNode() { delete myPos; }
	virtual const DataType * getType() override;
};

class StringTypeNode : public TypeNode{
public:
	StringTypeNode(Position * p): TypeNode(p) { }
	~StringTypeNode() { delete myPos; }
	virtual const DataType * getType() override;
};

class IntLitNode : public ExpNode{
public:
	IntLitNode(Position * p, const int numIn)
	: ExpNode(p), myNum(numIn){ }
	~IntLitNode() { delete myPos; }
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	const int myNum;
};

class StrLitNode : public ExpNode{
public:
	StrLitNode(Position * p, const std::string strIn)
	: ExpNode(p), myStr(strIn){ }
	~StrLitNode() { delete myPos; }
	bool nameAnalysis(SymbolTable *) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	 const std::string myStr;
};

class TrueNode : public ExpNode{
public:
	TrueNode(Position * p): ExpNode(p){ }
	~TrueNode() { delete myPos; }
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class FalseNode : public ExpNode{
public:
	FalseNode(Position * p): ExpNode(p){ }
	~FalseNode() { delete myPos; }
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
};

class CallStmtNode : public StmtNode{
public:
	CallStmtNode(Position * p, CallExpNode * expIn)
	: StmtNode(p), myCallExp(expIn){ }
	~CallStmtNode() { 
		delete myCallExp;
		delete myPos;
	}
	bool nameAnalysis(SymbolTable * symTab) override;
	virtual void typeAnalysis(TypeAnalysis *) override;
	const Eval* eval(Environment*) override;
private:
	CallExpNode * myCallExp;
};

class GlobalStmtNode : public DeclNode {
public:
	GlobalStmtNode(Position* p,StmtNode* in) : DeclNode(p), myStmt(in) {}
	~GlobalStmtNode() { delete myStmt; }
	bool isGlobEval() const override { return !myStmt->isVarDecl(); }
	virtual bool nameAnalysis(SymbolTable *);
	virtual void typeAnalysis(TypeAnalysis *);
	virtual const Eval* eval(Environment*);
private:
	StmtNode* myStmt;
};

class GlobalExpNode : public DeclNode {
public:
	GlobalExpNode(Position* p,ExpNode* in) : DeclNode(p), myExp(in) {}
	~GlobalExpNode() { delete myExp; }
	bool isGlobEval() const override { return true; }
	virtual bool nameAnalysis(SymbolTable *);
	virtual void typeAnalysis(TypeAnalysis *);
	virtual const Eval* eval(Environment*);
private:
	ExpNode* myExp;
};

} //End namespace cshanty

#endif

