#include "ast.hpp"
#include "errors.hpp"

namespace cshanty{

static void doIndent(std::ostream& out, int indent){
	for (int k = 0 ; k < indent; k++){ out << "\t"; }
}

void ProgramNode::transpileToC(std::ostream& out, int indent){
    out << "#include \"stdio.h\"\n\n";
	for (DeclNode * decl : *myGlobals){
		decl->transpileToC(out, indent);
	}
}

void VarDeclNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent); 
	myType->transpileToC(out, 0);
	out << " ";
	myID->transpileToC(out, 0);
	out << ";\n";
}

void RecordTypeDeclNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent); 
	out << "typedef struct ";
	myID->transpileToC(out, 0);
	out << "{\n";
	for(auto field : *myFields){
		field->transpileToC(out, 1);
	}
	out << "} ";
    myID->transpileToC(out,0);
    out << ";\n";
}

void FormalDeclNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent); 
	getTypeNode()->transpileToC(out, 0);
	out << " ";
	ID()->transpileToC(out, 0);
}

void FnDeclNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent); 
	myRetType->transpileToC(out, 0); 
	out << " ";
	myID->transpileToC(out, 0);
	out << "(";
    bool firstFormal = true;
    for(auto formal : *myFormals){
        if (firstFormal) { firstFormal = false; }
        else { out << ", "; }
        formal->transpileToC(out, 0);
    }
	out << "){\n";
	for(auto stmt : *myBody){
		stmt->transpileToC(out, indent+1);
	}
	doIndent(out, indent);
	out << "}\n";
}

void AssignStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp->transpileToC(out,0);
	out << ";\n";
}

void ReceiveStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
    std::string type;
    if (auto t = dynamic_cast<IDNode*>(myDst)) {
        if (t->getSymbol()->getDataType()->isString()) type = "%s";
        else type = "%d";
    } else if (auto t = dynamic_cast<IndexNode*>(myDst)) {
        if (t->isString()) type = "%s";
        else type = "%d";
    }
	out << "scanf(\""<<type<<"\",&";
	myDst->transpileToC(out,0);
	out << ");\n";
}

void ReportStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
    std::string type;
    if (auto s = dynamic_cast<StrLitNode*>(mySrc)) type = "%s";
    else if (auto s = dynamic_cast<CallExpNode*>(mySrc)) {
        if (s->isString()) type = "%s";
        else type = "%d";
    }
    else type = "%d";
	out << "printf(\""<<type<<"\",";
	mySrc->transpileToC(out,0);
	out << ");\n";
}

void PostIncStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myLVal->transpileToC(out,0);
	out << "++;\n";
}

void PostDecStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myLVal->transpileToC(out,0);
	out << "--;\n";
}

void IfStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "if (";
	myCond->transpileToC(out, 0);
	out << "){\n";
	for (auto stmt : *myBody){
		stmt->transpileToC(out, indent + 1);
	}
	doIndent(out, indent);
	out << "}\n";
}

void IfElseStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "if (";
	myCond->transpileToC(out, 0);
	out << "){\n";
	for (auto stmt : *myBodyTrue){
		stmt->transpileToC(out, indent + 1);
	}
	doIndent(out, indent);
	out << "} else {\n";
	for (auto stmt : *myBodyFalse){
		stmt->transpileToC(out, indent + 1);
	}
	doIndent(out, indent);
	out << "}\n";
}

void WhileStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "while (";
	myCond->transpileToC(out, 0);
	out << "){\n";
	for (auto stmt : *myBody){
		stmt->transpileToC(out, indent + 1);
	}
	doIndent(out, indent);
	out << "}\n";
}

void ReturnStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "return";
	if (myExp != nullptr){
		out << " ";
		myExp->transpileToC(out, 0);
	}
	out << ";\n";
}

void CallStmtNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myCallExp->transpileToC(out, 0);
	out << ";\n";
}

void ExpNode::transpileToCNested(std::ostream& out){
	out << "(";
	transpileToC(out, 0);
	out << ")";
}

void CallExpNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myID->transpileToC(out, 0);
	out << "(";
	
	bool firstArg = true;
	for(auto arg : *myArgs){
		if (firstArg) { firstArg = false; }
		else { out << ", "; }
		arg->transpileToC(out, 0);
	}
	out << ")";
}
void CallExpNode::transpileToCNested(std::ostream& out){
	transpileToC(out, 0);
}

void IndexNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myBase->transpileToCNested(out);
	out << ".";
	myIdx->transpileToC(out, 0);
}

void MinusNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " - ";
	myExp2->transpileToCNested(out);
}

void PlusNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " + ";
	myExp2->transpileToCNested(out);
}

void TimesNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " * ";
	myExp2->transpileToCNested(out);
}

void DivideNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " / ";
	myExp2->transpileToCNested(out);
}

void AndNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " && ";
	myExp2->transpileToCNested(out);
}

void OrNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " || ";
	myExp2->transpileToCNested(out);
}

void EqualsNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " == ";
	myExp2->transpileToCNested(out);
}

void NotEqualsNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " != ";
	myExp2->transpileToCNested(out);
}

void GreaterNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " > ";
	myExp2->transpileToCNested(out);
}

void GreaterEqNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " >= ";
	myExp2->transpileToCNested(out);
}

void LessNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " < ";
	myExp2->transpileToCNested(out);
}

void LessEqNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myExp1->transpileToCNested(out); 
	out << " <= ";
	myExp2->transpileToCNested(out);
}

void NotNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "!";
	myExp->transpileToCNested(out); 
}

void NegNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "-";
	myExp->transpileToCNested(out); 
}

void VoidTypeNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "void";
}

void IntTypeNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "int";
}

void StringTypeNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "const char*";
}

void RecordTypeNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << myID->getName();
}

void BoolTypeNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "int";
}

void AssignExpNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	myDst->transpileToCNested(out);
	out << " = ";
	mySrc->transpileToCNested(out);
}

void LValNode::transpileToCNested(std::ostream& out){
	transpileToC(out, 0);
}

void IDNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << name;
}

void IntLitNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << myNum;
}

void StrLitNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << myStr;
}

void FalseNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "0";
}

void TrueNode::transpileToC(std::ostream& out, int indent){
	doIndent(out, indent);
	out << "1";
}

} //End namespace cshanty
