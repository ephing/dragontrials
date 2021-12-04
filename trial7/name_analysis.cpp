#include "ast.hpp"
#include "symbol_table.hpp"
#include "errName.hpp"
#include "types.hpp"
#include "name_analysis.hpp"

namespace cshanty{

bool ProgramNode::nameAnalysis(SymbolTable * symTab){
	//Enter the global scope
	symTab->enterScope();
	bool res = true;
	for (auto decl : *myGlobals){
		res = decl->nameAnalysis(symTab) && res;
	}
	//Leave the global scope
	symTab->leaveScope();
	return res;
}

bool AssignStmtNode::nameAnalysis(SymbolTable * symTab){
	return myExp->nameAnalysis(symTab);
}

bool PostIncStmtNode::nameAnalysis(SymbolTable * symTab){
	return myLVal->nameAnalysis(symTab);
}

bool PostDecStmtNode::nameAnalysis(SymbolTable * symTab){
	return myLVal->nameAnalysis(symTab);
}

bool ReceiveStmtNode::nameAnalysis(SymbolTable * symTab){
	return myDst->nameAnalysis(symTab);
}

bool ReportStmtNode::nameAnalysis(SymbolTable * symTab){
	return mySrc->nameAnalysis(symTab);
}

bool IfStmtNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myCond->nameAnalysis(symTab) && result;
	symTab->enterScope();
	for (auto stmt : *myBody){
		result = stmt->nameAnalysis(symTab) && result;
	}	
	symTab->leaveScope();
	return result;
}

bool IfElseStmtNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myCond->nameAnalysis(symTab) && result;
	symTab->enterScope();
	for (auto stmt : *myBodyTrue){
		result = stmt->nameAnalysis(symTab) && result;
	}	
	symTab->leaveScope();
	symTab->enterScope();
	for (auto stmt : *myBodyFalse){
		result = stmt->nameAnalysis(symTab) && result;
	}	
	symTab->leaveScope();
	return result;
}

bool WhileStmtNode::nameAnalysis(SymbolTable * symTab){
	bool result = true;
	result = myCond->nameAnalysis(symTab) && result;
	symTab->enterScope();
	for (auto stmt : *myBody){
		result = stmt->nameAnalysis(symTab) && result;
	}	
	symTab->leaveScope();
	return result;
}

bool VarDeclNode::nameAnalysis(SymbolTable * symTab){
	bool checkType = myType->nameAnalysis(symTab);

	const DataType * dataType = getTypeNode()->getType();
	std::string varName = ID()->getName();

	bool validType = true;
	if (dataType == nullptr){
		validType = false;
	} else {
		validType = dataType->validVarType();
	}

	if (checkType && !validType){
		NameErr::badVarType(pos()); 
	}

	bool validName = !symTab->clash(varName);
	if (!validName){ NameErr::multiDecl(ID()->pos()); }

	if (!checkType || !validType || !validName){ 
		return false; 
	} else {
		symTab->insert(new VarSymbol(varName, dataType));
		SemSymbol * sym = symTab->find(varName);
		this->myID->attachSymbol(sym);
		return true;
	}
}

bool RecordTypeDeclNode::nameAnalysis(SymbolTable * symTab){
	std::string name = myID->getName();
	if (symTab->find(name) != nullptr){
		NameErr::multiDecl(this->pos());
		return false;
	}

	auto fields = new HashMap<std::string, const DataType *>();
	SymbolTable t;
	t.enterScope();
	for(auto elt : *myFields){
		std::string fieldName = elt->ID()->getName();
		SemSymbol * sym = t.find(fieldName);
		if (sym != nullptr){
			NameErr::multiDecl(elt->pos());
			return false;
		}

		bool res = elt->nameAnalysis(&t);
		sym = t.find(fieldName);
		if (!res){
			NameErr::badVarType(elt->pos()); 
			return false;
		}
		(*fields)[fieldName] = sym->getDataType();
		t.addVar(fieldName, sym->getDataType());
	}
	t.leaveScope();
	RecordType * r = RecordType::produce(myID->getName(), fields);
	symTab->insert(new RecordSymbol(name, r));

	return true;
}

bool FnDeclNode::nameAnalysis(SymbolTable * symTab){
	std::string fnName = this->ID()->getName();

	bool validRet = myRetType->nameAnalysis(symTab);

	// hold onto the scope of the function.
	ScopeTable * atFnScope = symTab->getCurrentScope();
	//Enter a new scope for "within" this function.
	ScopeTable * inFnScope = symTab->enterScope();

	/*Note that we check for a clash of the function 
	  name in it's declared scope (e.g. a global
	  scope for a global function)
	*/
	bool validName = true;
	if (atFnScope->clash(fnName)){
		NameErr::multiDecl(ID()->pos()); 
		validName = false;
	}

	bool validFormals = true;
	std::list<const DataType *> * formalTypes = 
		new std::list<const DataType *>();
	for (auto formal : *(this->myFormals)){
		validFormals = formal->nameAnalysis(symTab) && validFormals;
		TypeNode * typeNode = formal->getTypeNode();
		const DataType * formalType = typeNode->getType();
		formalTypes->push_back(formalType);
	}


	const DataType * retType = this->getRetTypeNode()->getType();
	FnType * dataType = new FnType(formalTypes, retType);
	//Make sure the fnSymbol is in the symbol table before 
	// analyzing the body, to allow for recursive calls
	if (validName){
		atFnScope->addFn(fnName, dataType);
		SemSymbol * sym = atFnScope->lookup(fnName);
		this->myID->attachSymbol(sym);
		
	}

	bool validBody = true;
	for (auto stmt : *myBody){
		validBody = stmt->nameAnalysis(symTab) && validBody;
	}

	symTab->leaveScope();
	return (validRet && validFormals && validName && validBody);
}

bool IndexNode::nameAnalysis(SymbolTable * symTab){
	bool res = myBase->nameAnalysis(symTab);
	if (!res){ return false; }
	return res;
}

bool BinaryExpNode::nameAnalysis(SymbolTable * symTab){
	bool resultLHS = myExp1->nameAnalysis(symTab);
	bool resultRHS = myExp2->nameAnalysis(symTab);
	return resultLHS && resultRHS;
}

bool CallExpNode::nameAnalysis(SymbolTable* symTab){
	bool result = true;
	result = myID->nameAnalysis(symTab) && result;
	for (auto arg : *myArgs){
		result = arg->nameAnalysis(symTab) && result;
	}
	return result;
}

bool NegNode::nameAnalysis(SymbolTable* symTab){
	return myExp->nameAnalysis(symTab);
}

bool NotNode::nameAnalysis(SymbolTable* symTab){
	return myExp->nameAnalysis(symTab);
}

bool AssignExpNode::nameAnalysis(SymbolTable* symTab){
	bool result = true;
	result = myDst->nameAnalysis(symTab) && result;
	result = mySrc->nameAnalysis(symTab) && result;
	return result;
}

bool ReturnStmtNode::nameAnalysis(SymbolTable * symTab){
	if (myExp == nullptr){ // May happen in void functions
		return true;
	}
	return myExp->nameAnalysis(symTab);
}

bool CallStmtNode::nameAnalysis(SymbolTable* symTab){
	return myCallExp->nameAnalysis(symTab);
}

bool TypeNode::nameAnalysis(SymbolTable * symTab){
	return true;
}

bool RecordTypeNode::nameAnalysis(SymbolTable * symTab){
	SemSymbol * sym = symTab->find(myID->getName());
	if (sym == nullptr){
		NameErr::badVarType(this->pos());
		return false;
	}
	const DataType * type = sym->getDataType();

	const RecordType * recordType = type->asRecord();
	if (recordType == nullptr){
		NameErr::badVarType(this->pos());
		return false;
	}
	this->myType = recordType;
	return true;
}

bool IntLitNode::nameAnalysis(SymbolTable * symTab){
	return true;
}

bool StrLitNode::nameAnalysis(SymbolTable * symTab){
	return true;
}

bool TrueNode::nameAnalysis(SymbolTable * symTab){
	return true;
}

bool FalseNode::nameAnalysis(SymbolTable * symTab){
	return true;
}

bool IDNode::nameAnalysis(SymbolTable* symTab){
	std::string myName = this->getName();
	SemSymbol * sym = symTab->find(myName);
	if (sym == nullptr){
		return NameErr::undeclID(pos());
	}
	this->attachSymbol(sym);
	return true;
}

void IDNode::attachSymbol(SemSymbol * symbolIn){
	this->mySymbol = symbolIn;
}

}
