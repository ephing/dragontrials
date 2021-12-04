#include <assert.h>

#include "name_analysis.hpp"
#include "type_analysis.hpp"

namespace cshanty {

TypeAnalysis * TypeAnalysis::build(NameAnalysis * nameAnalysis){
	TypeAnalysis * typeAnalysis = new TypeAnalysis();
	auto ast = nameAnalysis->ast;	
	typeAnalysis->ast = ast;

	ast->typeAnalysis(typeAnalysis);
	if (typeAnalysis->hasError){
		return nullptr;
	}

	return typeAnalysis;

}

void ProgramNode::typeAnalysis(TypeAnalysis * typing){
	for (auto decl : *myGlobals){
		decl->typeAnalysis(typing);
	}
	typing->nodeType(this, BasicType::VOID());
}

void IDNode::typeAnalysis(TypeAnalysis * typing){
	if (mySymbol == nullptr){
		typing->nodeType(this, nullptr);
	} else {
		const DataType * type = mySymbol->getDataType();
		typing->nodeType(this, type);
	}
}

void VarDeclNode::typeAnalysis(TypeAnalysis * typing){
	myType->typeAnalysis(typing);
	const DataType * declaredType = typing->nodeType(myType);
	//We assume that the type that comes back is valid,
	// otherwise we wouldn't have passed nameAnalysis
	typing->nodeType(this, declaredType);
}

void RecordTypeDeclNode::typeAnalysis(TypeAnalysis * typing){
	return;
}

void FnDeclNode::typeAnalysis(TypeAnalysis * typing){
	myRetType->typeAnalysis(typing);
	const DataType * retDataType = typing->nodeType(myRetType);

	std::list<const DataType *> * formalTypes = 
		new std::list<const DataType *>();
	for (auto formal : *myFormals){
		formal->typeAnalysis(typing);
		formalTypes->push_back(typing->nodeType(formal));
	}	

	
	typing->nodeType(this, new FnType(formalTypes, retDataType));

	typing->setCurrentFnType(typing->nodeType(this)->asFn());
	for (auto stmt : *myBody){
		stmt->typeAnalysis(typing);
	}
	typing->setCurrentFnType(nullptr);
}

static bool validAssignOpd(const DataType * type){
	if (type == nullptr){
		return false;
	} 

	if (type->isBool() || type->isInt()){ 
		return true; 
	}
	if (type->asError()){ 
		return true; 
	}
	return false;
}

static bool type_RecordVar(DataType * type){
	if (type == nullptr){
		return false;
	} else {
		return type->asRecord();
	}
}

static bool type_RecordName(const DataType * type){
	return type == nullptr;
}

static bool type_isError(const DataType * type){
	return type != nullptr && type->asError();
}

void AssignExpNode::typeAnalysis(TypeAnalysis * typing){
	myDst->typeAnalysis(typing);
	mySrc->typeAnalysis(typing);
	const DataType * dstType = typing->nodeType(myDst);
	const DataType * srcType = typing->nodeType(mySrc);

	if (type_RecordName(dstType) || type_RecordName(srcType)){
		typing->nodeType(this, ErrorType::produce());
		if (dstType == nullptr && srcType == nullptr){
			typing->errAssignRecName(this->pos());
			return;
		} 
		if (dstType == nullptr){
			typing->errAssignOpd(myDst->pos());
			if (!validAssignOpd(srcType)){ 
				typing->errAssignOpd(mySrc->pos());
			}
		} else if (srcType == nullptr){
			if (!validAssignOpd(dstType)){ 
				typing->errAssignOpd(myDst->pos());
			}
			typing->errAssignOpd(mySrc->pos());
		}
		return;
	}
	if (srcType->asRecord() && dstType->asRecord()){
		typing->nodeType(this, ErrorType::produce());
		typing->errAssignRecVar(this->pos());
		return;
	}
	/*
	if (dstType->asFn() && srcType->asFn()){
		typing->nodeType(this, ErrorType::produce());
		typing->errAssignFn(this->pos());
		return;
	}
	*/


	bool validOperands = true;
	bool knownError = type_isError(dstType) || type_isError(srcType);
	if (!validAssignOpd(dstType)){
		typing->errAssignOpd(myDst->pos());
		validOperands = false;
	}
	if (!validAssignOpd(srcType)){
		typing->errAssignOpd(mySrc->pos());
		validOperands = false;
	}
	if (!validOperands || knownError){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	if (dstType->asError() || srcType->asError()){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	if (dstType == srcType){
		typing->nodeType(this, dstType);
		return;
	}

	typing->nodeType(this, ErrorType::produce());
	typing->errAssignOpr(this->pos());
	return;
}

void CallExpNode::typeAnalysis(TypeAnalysis * typing){

	std::list<const DataType *> * aList = new std::list<const DataType *>();
	for (auto actual : *myArgs){
		actual->typeAnalysis(typing);
		aList->push_back(typing->nodeType(actual));
	}

	SemSymbol * calleeSym = myID->getSymbol();
	assert(calleeSym != nullptr);
	const DataType * calleeType = calleeSym->getDataType();
	const FnType * fnType = calleeType->asFn();
	if (fnType == nullptr){
		typing->errCallee(myID->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	const std::list<const DataType *>* fList = fnType->getFormalTypes();
	if (aList->size() != fList->size()){
		typing->errArgCount(pos());
		//Note: we still consider the call to return the 
		// return type
	} else {
		auto actualTypesItr = aList->begin();
		auto formalTypesItr = fList->begin();
		auto actualsItr = myArgs->begin();
		while(actualTypesItr != aList->end()){
			const DataType * actualType = *actualTypesItr;
			const DataType * formalType = *formalTypesItr;
			ExpNode * actual = *actualsItr;
			auto actualsItrOld = actualsItr;
			actualTypesItr++;
			formalTypesItr++;
			actualsItr++;

			//Matching to error is ignored
			if (actualType->asError()){ continue; }
			if (formalType->asError()){ continue; }

			//Ok match
			if (formalType == actualType){ continue; }

			const RecordType * formalRec = formalType->asRecord();
			const RecordType * actualRec = actualType->asRecord();
			if (formalRec && actualRec){
				if (formalRec == actualRec){
					continue;
				}
			}

			//Bad match
			typing->errArgMatch(actual->pos());
			typing->nodeType(this, ErrorType::produce());
		}
	}

	typing->nodeType(this, fnType->getReturnType());
	return;
}

void NegNode::typeAnalysis(TypeAnalysis * typing){
	myExp->typeAnalysis(typing);
	const DataType * subType = typing->nodeType(myExp);

	//Propagate error, don't re-report
	if (subType->asError()){
		typing->nodeType(this, subType);
		return;
	} else if (subType->isInt()){
		typing->nodeType(this, BasicType::INT());
	} else {
		typing->errMathOpd(myExp->pos());
		typing->nodeType(this, ErrorType::produce());
	}
}

void NotNode::typeAnalysis(TypeAnalysis * typing){
	myExp->typeAnalysis(typing);
	const DataType * childType = typing->nodeType(myExp);

	if (childType->asError() != nullptr){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	if (childType->isBool()){
		typing->nodeType(this, childType);
		return;
	} else {
		typing->errLogicOpd(myExp->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}
}

void TypeNode::typeAnalysis(TypeAnalysis * typing){
	typing->nodeType(this, this->getType());
}

void RecordTypeNode::typeAnalysis(TypeAnalysis * typing){
	assert(myType != nullptr);
	typing->nodeType(this, nullptr);
	return;
}

static bool typeMathOpd(TypeAnalysis * typing, ExpNode * opd){
	opd->typeAnalysis(typing);
	const DataType * type = typing->nodeType(opd);
	if (type->isInt()){ return true; }
	//if (type->isByte()){ return true; }
	if (type->asError()){
		//Don't re-report an error, but don't check for
		// incompatibility
		return false;
	}

	typing->errMathOpd(opd->pos());
	return false;
}

/*
static const DataType * getEltType(const ArrayType * arrType){
	if (arrType == nullptr){
		return ErrorType::produce();
	}
	return arrType->baseType();
}
*/

void IndexNode::typeAnalysis(TypeAnalysis * typing){
	myBase->typeAnalysis(typing);
	//myIdx->typeAnalysis(typing);

	const DataType * baseType = typing->nodeType(myBase);
	//const DataType * offType = typing->nodeType(myIdx);

	if (baseType->asError()){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	const RecordType * asRec = baseType->asRecord();
	if (asRec == nullptr){
		typing->errRecordID(myBase->pos());
		return;
	}
	
	const DataType * fieldType = asRec->getField(myIdx->getName());
	if (fieldType == nullptr){
		TODO(No such field!)
	}
	typing->nodeType(this, fieldType);
	
	/*
	const ArrayType * asArray = baseType->asArray();
	if (asArray == nullptr){
		throw new ToDoError("Implment me")
	}
	*/

	/*
	if (offType->isByte()){
		typing->nodeType(this, eltType);
		return;
	}
	*/

	//typing->errArrayIndex(myOffset->pos();

	/*
	if (const PtrType * asPtr = baseType->asPtr()){
		typing->nodeType(this, asPtr->decLevel());
	} else {
		typing->badPtrBase(myBase->pos());
	}
	*/
}

void BinaryExpNode::binaryMathTyping(
	TypeAnalysis * typing
){
	bool lhsValid = typeMathOpd(typing, myExp1);
	bool rhsValid = typeMathOpd(typing, myExp2);
	if (!lhsValid || !rhsValid){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	//Get the valid operand types, check operator
	const DataType * lhsType = typing->nodeType(myExp1);
	const DataType * rhsType = typing->nodeType(myExp2);

	if (lhsType->isInt() && rhsType->isInt()){
		typing->nodeType(this, BasicType::INT());
		return;
	}

	typing->nodeType(this, ErrorType::produce());
	return;
}

static const DataType * typeLogicOpd(
	TypeAnalysis * typing, ExpNode * opd
){
	opd->typeAnalysis(typing);
	const DataType * type = typing->nodeType(opd);

	//Return type if it's valid
	if (type->isBool()){ return type; }

	//Don't re-report an error, but return null to
	// indicate incompatibility
	if (type->asError()){ return nullptr; }

	//If type isn't an error, but is incompatible,
	// report and indicate incompatibility
	typing->errLogicOpd(opd->pos());
	return NULL;
}

void BinaryExpNode::binaryLogicTyping(TypeAnalysis * typing){
	const DataType * lhsType = typeLogicOpd(typing, myExp1);
	const DataType * rhsType = typeLogicOpd(typing, myExp2);
	if (!lhsType || !rhsType){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	//Given valid operand types, check operator
	if (lhsType->isBool() && rhsType->isBool()){
		typing->nodeType(this, BasicType::BOOL());
		return;
	}

	//We never expect to get here, so we'll consider it
	// an error with the compiler itself
	throw new InternalError("Incomplete typing");
	typing->nodeType(this, ErrorType::produce());
	return;
}

void PlusNode::typeAnalysis(TypeAnalysis * typing){
	binaryMathTyping(typing);
}

void MinusNode::typeAnalysis(TypeAnalysis * typing){
	binaryMathTyping(typing);
}

void TimesNode::typeAnalysis(TypeAnalysis * typing){
	binaryMathTyping(typing);
}

void DivideNode::typeAnalysis(TypeAnalysis * typing){
	binaryMathTyping(typing);
}

void AndNode::typeAnalysis(TypeAnalysis * typing){
	binaryLogicTyping(typing);
}

void OrNode::typeAnalysis(TypeAnalysis * typing){
	binaryLogicTyping(typing);
}

static const DataType * typeEqOpd(
	TypeAnalysis * typing, ExpNode * opd
){
	assert(opd != nullptr || "opd is null!");

	opd->typeAnalysis(typing);
	const DataType * type = typing->nodeType(opd);
	if (type == nullptr){ return nullptr; } // Record name

	if (type->isInt()){ return type; }
	if (type->isBool()){ return type; }

	// Bad record, but handled outside
	if (type->asRecord()){ 
		return type; 
	} 

	//Errors are invalid, but don't cause re-reports
	if (type->asError()){ return ErrorType::produce(); }

	typing->errEqOpd(opd->pos());
	return ErrorType::produce();
}

void BinaryExpNode::binaryEqTyping(TypeAnalysis * typing){
	const DataType * lhsType = typeEqOpd(typing, myExp1);
	const DataType * rhsType = typeEqOpd(typing, myExp2);

	if (lhsType == nullptr || rhsType == nullptr){
		typing->nodeType(this, ErrorType::produce());
		if (lhsType == nullptr && rhsType == nullptr){
			typing->errEqRecNames(this->pos());
		} else if (lhsType == nullptr){
			typing->errEqOpd(myExp1->pos());
		} else {
			typing->errEqRecNames(myExp2->pos());
		}
		return;
	}
	if (lhsType->asRecord() || rhsType->asRecord()){
		typing->nodeType(this, ErrorType::produce());
		if (lhsType->asRecord() && rhsType->asRecord()){
			typing->errEqRecVars(this->pos());
		} else if (lhsType->asRecord()){
			typing->errEqOpd(myExp1->pos());
		} else {
			typing->errEqRecNames(myExp2->pos());
		}
		return;
	}

	if (lhsType->asError() || rhsType->asError()){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	if (lhsType == rhsType){
		typing->nodeType(this, BasicType::BOOL());
		return;
	}

	const RecordType * lhsRec = lhsType->asRecord();
	const RecordType * rhsRec = rhsType->asRecord();
	if (lhsRec != nullptr || rhsRec != nullptr){
		typing->errEqRecVars(myExp1->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	typing->errEqOpr(pos());
	typing->nodeType(this, ErrorType::produce());
	return;
}

void EqualsNode::typeAnalysis(TypeAnalysis * typing){
	binaryEqTyping(typing);
	assert(typing->nodeType(this) != nullptr);
}

void NotEqualsNode::typeAnalysis(TypeAnalysis * typing){
	binaryEqTyping(typing);
}

static const DataType * typeRelOpd(
	TypeAnalysis * typing, ExpNode * opd
){
	opd->typeAnalysis(typing);
	const DataType * type = typing->nodeType(opd);

	if (type->isInt()){ return type; }
	//if (type->isByte()){ return type; }

	//Errors are invalid, but don't cause re-reports
	if (type->asError()){ return nullptr; }

	typing->errRelOpd(opd->pos());
	typing->nodeType(opd, ErrorType::produce());
	return nullptr;
}

void BinaryExpNode::binaryRelTyping(TypeAnalysis * typing){
	const DataType * lhsType = typeRelOpd(typing, myExp1);
	const DataType * rhsType = typeRelOpd(typing, myExp2);

	if (!lhsType || !rhsType){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	if (lhsType->isInt() && rhsType->isInt()){
		typing->nodeType(this, BasicType::BOOL());
		return;
	}

	//There is no bad relational operator, so we never 
	// expect to get here
	return;
}

void GreaterNode::typeAnalysis(TypeAnalysis * typing){
	binaryRelTyping(typing);
}

void GreaterEqNode::typeAnalysis(TypeAnalysis * typing){
	binaryRelTyping(typing);
}

void LessNode::typeAnalysis(TypeAnalysis * typing){
	binaryRelTyping(typing);
}

void LessEqNode::typeAnalysis(TypeAnalysis * typing){
	binaryRelTyping(typing);
}

void AssignStmtNode::typeAnalysis(TypeAnalysis * typing){
	myExp->typeAnalysis(typing);
	const DataType * childType = typing->nodeType(myExp);
	if (childType->asError()){
		typing->nodeType(this, ErrorType::produce());
	} else {
		typing->nodeType(this, BasicType::VOID());
	}
}

void PostDecStmtNode::typeAnalysis(TypeAnalysis * typing){
	myLVal->typeAnalysis(typing);
	const DataType * childType = typing->nodeType(myLVal);

	if (childType->asError()){ return; }
	if (childType->isInt()){ return; }
	//if (childType->isByte()){ return; }

	//Any other unary math is an error
	typing->errMathOpd(myLVal->pos());
}

void PostIncStmtNode::typeAnalysis(TypeAnalysis * typing){
	myLVal->typeAnalysis(typing);
	const DataType * childType = typing->nodeType(myLVal);

	if (childType->asError()){ return; }
	if (childType->isInt()){ return; }
	//if (childType->isByte()){ return; }

	//Any other unary math is an error
	typing->errMathOpd(myLVal->pos());
}

void ReceiveStmtNode::typeAnalysis(TypeAnalysis * typing){
	myDst->typeAnalysis(typing);
	const DataType * childType = typing->nodeType(myDst);

	if (childType == nullptr){ // record name
		typing->errReceiveRecName(myDst->pos());
		return;
	}

	if (childType->isBool()){
		return;
	} else if (childType->isInt()){
		return;
	} else if (childType->isRecord()){
		typing->errReceiveRecVar(myDst->pos());
		return;
	/*
	} else if (childType->isArray()){
		const ArrayType * arr = childType->asArray();
		if (arr->baseType()->isByte()){
			return;
		}
		typing->errReadOther(myDst->pos());
	*/
	} else if (childType->asFn()){
		typing->errReadFn(myDst->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	} else if (childType->asError()){
		typing->nodeType(this, ErrorType::produce());
		return;
	} else {
		typing->errReadOther(myDst->pos());
	}
	typing->nodeType(this, BasicType::VOID());
}

void ReportStmtNode::typeAnalysis(TypeAnalysis * typing){
	mySrc->typeAnalysis(typing);
	const DataType * childType = typing->nodeType(mySrc);

	//Record name
	if (type_RecordName(childType)){
		typing->errReportRecName(mySrc->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	//Mark error, but don't re-report
	if (childType->asError()){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	//Check for invalid type
	if (childType->isVoid()){
		typing->errWriteVoid(mySrc->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	} else if (childType->asFn()){
		typing->errWriteFn(mySrc->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	} else if (childType->asBasic()){
		//Can write to a var of any other type
		return;
	} else if (childType->asRecord()){
		typing->errReportRecVar(mySrc->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}
	/*
	} else if (const ArrayType * arr = childType->asArray()){
		if(arr->baseType()->isByte()){
			return;
		} else {
			typing->errWriteArray(mySrc->pos());
			return;
		}
	}
	*/

	/*
	if (const PtrType * asPtr = childType->asPtr()){
		//TODO: FIXME
		const DataType * deref = PtrType::derefType(asPtr);
		const BasicType * base = deref->asBasic();
		assert(base != nullptr);
			
		if (base->isByte()){
			typing->nodeType(this, BasicType::VOID());
		} else {
			typing->badWritePtr(pos);
		}
		return;
	}
	*/

	typing->nodeType(this, BasicType::VOID());
}

void IfStmtNode::typeAnalysis(TypeAnalysis * typing){
	//Start off the typing as void, but may update to error
	typing->nodeType(this, BasicType::VOID());

	myCond->typeAnalysis(typing);
	const DataType * condType = typing->nodeType(myCond);
	bool goodCond = true;
	if (condType == nullptr){
		typing->nodeType(this, ErrorType::produce());
		goodCond = false;
	} else if (condType->asError()){
		typing->nodeType(this, ErrorType::produce());
		goodCond = false;
	} else if (!condType->isBool()){
		goodCond = false;
		typing->errIfCond(myCond->pos());
		typing->nodeType(this, 
			ErrorType::produce());
	}

	for (auto stmt : *myBody){
		stmt->typeAnalysis(typing);
	}

	if (goodCond){
		typing->nodeType(this, BasicType::produce(VOID));
	} else {
		typing->nodeType(this, ErrorType::produce());
	}
}

void IfElseStmtNode::typeAnalysis(TypeAnalysis * typing){
	myCond->typeAnalysis(typing);
	const DataType * condType = typing->nodeType(myCond);

	bool goodCond = true;
	if (condType->asError()){
		goodCond = false;
		typing->nodeType(this, ErrorType::produce());
	} else if (!condType->isBool()){
		typing->errIfCond(myCond->pos());
		goodCond = false;
	}
	for (auto stmt : *myBodyTrue){
		stmt->typeAnalysis(typing);
	}
	for (auto stmt : *myBodyFalse){
		stmt->typeAnalysis(typing);
	}
	
	if (goodCond){
		typing->nodeType(this, BasicType::produce(VOID));
	} else {
		typing->nodeType(this, ErrorType::produce());
	}
}

void WhileStmtNode::typeAnalysis(TypeAnalysis * typing){
	myCond->typeAnalysis(typing);
	const DataType * condType = typing->nodeType(myCond);

	if (condType->asError()){
		typing->nodeType(this, ErrorType::produce());
	} else if (!condType->isBool()){
		typing->errWhileCond(myCond->pos());
	}

	for (auto stmt : *myBody){
		stmt->typeAnalysis(typing);
	}

	typing->nodeType(this, BasicType::VOID());
}

void CallStmtNode::typeAnalysis(TypeAnalysis * typing){
	myCallExp->typeAnalysis(typing);
	typing->nodeType(this, BasicType::VOID());
}

void ReturnStmtNode::typeAnalysis(TypeAnalysis * typing){
	const FnType * fnType = typing->getCurrentFnType();
	const DataType * fnRet = fnType->getReturnType();

	//Check: shouldn't return anything
	if (fnRet == BasicType::VOID()){
		if (myExp != nullptr) {
			myExp->typeAnalysis(typing);
			typing->extraRetValue(myExp->pos());
			typing->nodeType(this, ErrorType::produce());
		} else {
			typing->nodeType(this, BasicType::VOID());
		}
		return;
	}

	//Check: returns nothing, but should
	if (myExp == nullptr){
		typing->errRetEmpty(pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	//Attempt to return a record name 
	if (fnRet == nullptr){
		typing->errRetWrong(myExp->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	myExp->typeAnalysis(typing);
	const DataType * childType = typing->nodeType(myExp);

	if (childType->asError()){
		typing->nodeType(this, ErrorType::produce());
		return;
	}

	if (childType != fnRet){
		typing->errRetWrong(myExp->pos());
		typing->nodeType(this, ErrorType::produce());
		return;
	}
	typing->nodeType(this, ErrorType::produce());
	return;
}

void StrLitNode::typeAnalysis(TypeAnalysis * typing){
	BasicType * basic = BasicType::STRING();
	//ArrayType * asArr = ArrayType::produce(basic, 0);
	typing->nodeType(this, basic);
}

void FalseNode::typeAnalysis(TypeAnalysis * typing){
	typing->nodeType(this, BasicType::BOOL());
}

void TrueNode::typeAnalysis(TypeAnalysis * typing){
	typing->nodeType(this, BasicType::BOOL());
}

void IntLitNode::typeAnalysis(TypeAnalysis * typing){
	typing->nodeType(this, BasicType::INT());
}

}
