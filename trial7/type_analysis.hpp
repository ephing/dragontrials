#ifndef XXLANG_TYPE_ANALYSIS
#define XXLANG_TYPE_ANALYSIS

#include "ast.hpp"
#include "symbol_table.hpp"
#include "types.hpp"

class NameAnalysis;

namespace cshanty{

// An instance of this class will be passed over the entire
// AST. Rather than attaching types to each node, the 
// TypeAnalysis class contains a map from each ASTNode to it's
// DataType. Thus, instead of attaching a type field to most nodes,
// one can instead map the node to it's type, or lookup the node
// in the map.
class TypeAnalysis {

private:
	//The private constructor here means that the type analysis
	// can only be created via the static build function
	TypeAnalysis(){
		hasError = false;
	}

public:
	static TypeAnalysis * build(NameAnalysis * astRoot);
	//static TypeAnalysis * build();

	//The type analysis has an instance variable to say whether
	// the analysis failed or not. Setting this variable is much
	// less of a pain than passing a boolean all the way up to the
	// root during the TypeAnalysis pass. 
	bool passed(){
		return !hasError;
	}

	void setCurrentFnType(const FnType * type){
		currentFnType = type;
	}

	const FnType * getCurrentFnType(){
		return currentFnType;
	}

	
	//Set the type of a node. Note that the function name is 
	// overloaded: this 2-argument nodeType puts a value into the
	// map with a given type. 
	void nodeType(const ASTNode * node, const DataType * type){
		nodeToType[node] = type;
	}

	//Gets the type of a node already placed in the map. Note
	// that this function name is overloaded: the 1-argument nodeType
	// gets the type of the given node out of the map.
	const DataType * nodeType(const ASTNode * node){
		auto res = nodeToType.find(node);
		assert(res != nodeToType.end() || "No type for node");
		
		//Note: this actually could be nullptr
		return res->second;
	}

	//The following functions all report and error and 
	// tell the object that the analysis has failed. 
	void errWriteFn(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attempt to output a function");
	}
	void errWriteVoid(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Attempt to write void");
	}
	void errReportRecName(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attempt to report record name");
	}
	void errReportRecVar(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attempt to output a record variable");
	}
	void errReceiveRecVar(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attempt to read a record variable");
	}
	void errReceiveRecName(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attempt to read a record name");
	}
	void errReadFn(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attemp to assign input to function");
	}
	void errReadOther(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attempt to read to illegal type");
	}
	void errCallee(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Attempt to call a "
			"non-function");
	}
	void errArgCount(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Function call with wrong"
			" number of args");
	}
	void errArgMatch(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Type of actual does not match"
			" type of formal");
	}
	void errRetEmpty(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Missing return value");
	}
	void extraRetValue(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Return with a value in void"
			" function");
	}
	void errRetWrong(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Bad return value");
	}
	void errMathOpd(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Arithmetic operator applied"
			" to invalid operand");
	}
	void errRelOpd(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Relational operator applied to"
			" non-numeric operand");
	}
	void errLogicOpd(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Logical operator applied to"
			" non-bool operand");
	}
	void errIfCond(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Non-bool expression used as"
			" an if condition");
	}
	void errWhileCond(Position * pos){
		hasError = true;
		Report::fatal(pos,
			"Non-bool expression used as"
			" a while condition");
	}
	void errEqOpd(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Invalid equality operand");
	}
	void errEqOpr(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Invalid equality operation");
	}
	void errEqRecVars(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Equality operator applied to record variables");
	}
	void errEqRecNames(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Equality operator applied to record names");
	}
	void errAssignOpd(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Invalid assignment operand");
	}
	void errAssignOpr(Position * pos){
		hasError = true;
		Report::fatal(pos, 
			"Invalid assignment operation");
	}
	void errAssignRecVar(Position * pos){
		hasError = true;
		Report::fatal(pos, "Record variable assignment");
	}
	void errAssignRecName(Position * pos){
		hasError = true;
		Report::fatal(pos, "Record name assignment");
	}
	void errAssignFn(Position * pos){
		hasError = true;
		Report::fatal(pos, "Invalid assignment operation");
	}
	void errRecordID(Position * pos){
		hasError = true;
		Report::fatal(pos, "Attempt to index a non-record");
	}

	void errRecordIndex(Position * pos){
		hasError = true;
		Report::fatal(pos, "Bad index");
	}
private:
	HashMap<const ASTNode *, const DataType *> nodeToType;
	const FnType * currentFnType;
	bool hasError;
public:
	ProgramNode * ast;
};

}
#endif
