#include "ast.hpp"

cshanty::ProgramNode::ProgramNode(std::list<DeclNode *> * globalsIn)
: ASTNode(new Position(0,0,0,0)), myGlobals(globalsIn){
	if (!globalsIn->empty()){
		myPos->expand(
			myGlobals->front()->pos(),
			myGlobals->back()->pos()
		);
	}
}
