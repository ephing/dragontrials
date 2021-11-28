#ifndef CSHANTY_DATA_TYPES
#define CSHANTY_DATA_TYPES

#include <list>
#include <sstream>
#include "errors.hpp"

#include <unordered_map>

#ifndef CSHANTY_HASH_MAP_ALIAS
// Use an alias template so that we can use
// "HashMap" and it means "std::unordered_map"
template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;
#endif


namespace cshanty{

class ASTNode;

class BasicType;
class FnType;
class ErrorType;
class RecordType;

enum BaseType{
	INT, VOID, STRING, BOOL
};

//This class is the superclass for all cshanty types. You
// can get information about which type is implemented
// concretely using the as<X> functions, or query information
// using the is<X> functions.
class DataType{
public:
	virtual std::string getString() const = 0;
	virtual const BasicType * asBasic() const { return nullptr; }
	virtual const RecordType * asRecord() const { return nullptr; }
	virtual const FnType * asFn() const { return nullptr; }
	virtual const ErrorType * asError() const { return nullptr; }
	virtual bool isVoid() const { return false; }
	virtual bool isInt() const { return false; }
	virtual bool isBool() const { return false; }
	virtual bool isString() const { return false; }
	virtual bool isRecord() const { return false; }
	virtual bool validVarType() const = 0 ;
	virtual size_t getSize() const = 0;
protected:
};

//This DataType subclass is the superclass for all cshanty types. 
// Note that there is exactly one instance of this 
class ErrorType : public DataType{
public:
	static ErrorType * produce(){
		//Note: this static member will only ever be initialized 
		// ONCE, no matter how many times the function is called.
		// That means there will only ever be 1 instance of errorType
		// in the entire codebase.
		static ErrorType * error = new ErrorType();
		
		return error;
	}
	virtual const ErrorType * asError() const override { return this; }
	virtual std::string getString() const override { 
		return "ERROR";
	}
	virtual bool validVarType() const override { return false; }
	virtual size_t getSize() const override { return 0; }
private:
	ErrorType(){ 
		/* private constructor, can only 
		be called from produce */
	}
	size_t line;
	size_t col;
};

//DataType subclass for all scalar types 
class BasicType : public DataType{
public:
	static BasicType * VOID(){
		return produce(BaseType::VOID);
	}
	static BasicType * BOOL(){
		return produce(BaseType::BOOL);
	}
	static BasicType * STRING(){
		return produce(BaseType::STRING);
	}
	static BasicType * INT(){
		return produce(BaseType::INT);
	}

	//Create a scalar type. If that type already exists,
	// return the known instance of that type. Making sure
	// there is only 1 instance of a class for a given set
	// of fields is known as the "flyweight" design pattern
	// and ensures that the memory needs of a program are kept
	// down: rather than having a distinct type for every base
	// INT (for example), only one is constructed and kept in
	// the flyweights list. That type is then re-used anywhere
	// it's needed. 

	//Note the use of the static function declaration, which 
	// means that no instance of BasicType is needed to call
	// the function.
	static BasicType * produce(BaseType base){
		//Note the use of the static local variable, which
		//means that the flyweights variable persists between
		// multiple calls to this function (it is essentially
		// a global variable that can only be accessed
		// in this function).
		static std::list<BasicType *> flyweights;
		for(BasicType * fly : flyweights){
			if (fly->getBaseType() == base){
				return fly;
			}
		}
		BasicType * newType = new BasicType(base);
		flyweights.push_back(newType);
		return newType;
	}
	const BasicType * asBasic() const override {
		return this;
	}
	BasicType * asBasic(){
		return this;
	}
	bool isInt() const override {
		return myBaseType == BaseType::INT;
	}
	bool isString() const override {
		return myBaseType == BaseType::STRING;
	}
	bool isBool() const override {
		return myBaseType == BaseType::BOOL;
	}
	virtual bool isVoid() const override { 
		return myBaseType == BaseType::VOID; 
	}
	virtual bool validVarType() const override {
		return !isVoid();
	}
	virtual BaseType getBaseType() const { return myBaseType; }
	virtual std::string getString() const override;
	virtual size_t getSize() const override { 
		if (isBool()){ return 8; }
		else if (isString()){ return 8; }
		else if (isVoid()){ return 8; }
		else if (isInt()){ return 8; }
		else { return 0; }
	}
private:
	BasicType(BaseType base) 
	: myBaseType(base){ }
	BaseType myBaseType;
};

class RecordType : public DataType{
public:
	//static RecordType * produce(std::list<DataType *>, std::string name){
	static RecordType * produce(std::string name, HashMap<std::string, const DataType *> * fields){
		static HashMap <std::string, RecordType *> map;

		//TODO: find a node
		RecordType * r;
		auto res = map.find(name);
		if (res == map.end()){
			RecordType * r = new RecordType(name, fields);
			map[name] = r;
			return r;
		} else {
			return res->second;
		}
	};
	bool validVarType() const override { return true; }
	std::string getString() const override { return name; }
	size_t getSize() const override { 
		size_t size = 0;
		for (auto field : *fieldTypes){
			const DataType * fieldType = field.second;
			size += fieldType->getSize();
		}
		return size;
	}
	const RecordType * asRecord() const override { return this; }
	bool isRecord() const override { return true; }

	const DataType * getField(std::string fieldName) const{
		auto res = fieldTypes->find(fieldName);
		if (res == fieldTypes->end()){ return nullptr; }
		return res->second;
	}
	size_t getOffset(std::string name) const {
		auto itr = offset.find(name);
		const size_t fieldOff = itr->second;
		return fieldOff;
	}
private:
	RecordType(std::string nameIn, HashMap<std::string, const DataType *> * fieldsIn) 
	: name(nameIn), fieldTypes(fieldsIn){ 
		size_t usedOffset = 0;
		for (auto field : *fieldTypes){
			std::string name = field.first;
			const DataType * fieldType = field.second;
			offset[name] = usedOffset;
			usedOffset += fieldType->getSize();
		}
	}
	std::string name;
	HashMap<std::string, const DataType *> *fieldTypes;
	HashMap<std::string, size_t> offset;
};

//DataType subclass to represent the type of a function. It will
// have a list of argument types and a return type. 
class FnType : public DataType{
public:
	FnType(const std::list<const DataType *>* formalsIn, const DataType * retTypeIn) 
	: DataType(),
	  myFormalTypes(formalsIn),
	  myRetType(retTypeIn)
	{
	}
	std::string getString() const override{
		std::string result = "";
		bool first = true;
		for (auto elt : *myFormalTypes){
			if (first) { first = false; }
			else { result += ","; }
			result += elt->getString();
		}
		result += "->";
		result += myRetType->getString();
		return result;
	}
	virtual const FnType * asFn() const override { return this; }

	const DataType * getReturnType() const {
		return myRetType;
	}
	const std::list<const DataType *> * getFormalTypes() const {
		return myFormalTypes;
	}
	virtual bool validVarType() const override { return false; }
	virtual size_t getSize() const override { return 0; }
private:
	const std::list<const DataType *> * myFormalTypes;
	const DataType * myRetType;
};

}

#endif
