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
	virtual ~DataType() {}
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
		return error;
	}
	virtual const ErrorType * asError() const override { return this; }
	virtual std::string getString() const override { 
		return "ERROR";
	}
	virtual bool validVarType() const override { return false; }
	virtual size_t getSize() const override { return 0; }
	static void destroy() {
		delete error;
	}
private:
	ErrorType(){ 
		/* private constructor, can only 
		be called from produce */
	}
	size_t line;
	size_t col;
	static ErrorType * error;
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
	static BasicType * produce(BaseType base){
		for(BasicType * fly : BasicType::flyweights){
			if (fly->getBaseType() == base){
				return fly;
			}
		}
		BasicType * newType = new BasicType(base);
		BasicType::flyweights.push_back(newType);
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
		if (isBool()){ return 1; }
		else if (isString()){ throw new ToDoError("Implement me"); }
		else if (isVoid()){ return 8; }
		else if (isInt()){ return 8; }
		else { return 0; }
	}
	static void destroyPrimitives() {
		for (auto i : flyweights) delete i;
	}
private:
	BasicType(BaseType base) 
	: myBaseType(base){ }
	BaseType myBaseType;
	static std::list<BasicType *> flyweights;
};

class RecordType : public DataType{
public:
	//static RecordType * produce(std::list<DataType *>, std::string name){
	static RecordType * produce(std::string name, HashMap<std::string, const DataType *> * fields){
		RecordType * r;
		auto res = map.find(name);
		if (res == map.end()){
			RecordType * r = new RecordType(name, fields);
			map[name] = r;
			return r;
		} else {
			delete fields;
			return res->second;
		}
	};
	bool validVarType() const override { return true; }
	std::string getString() const override { return name; }
	size_t getSize() const override { return size; }
	const RecordType * asRecord() const override { return this; }
	bool isRecord() const override { return true; }

	const DataType * getField(std::string fieldName) const{
		auto res = fieldTypes->find(fieldName);
		if (res == fieldTypes->end()){ return nullptr; }
		return res->second;
	}

	HashMap<std::string, const DataType*>* getFields() const {
		return fieldTypes;
	}
	~RecordType() { delete fieldTypes; }
	static void destroyRecords() {
		for (auto i : map) delete i.second;
	}
private:
	RecordType(std::string nameIn, HashMap<std::string, const DataType *> * fieldsIn) 
	: name(nameIn), fieldTypes(fieldsIn), size(fieldsIn->size() * 8){ 
	}
	std::string name;
	HashMap<std::string, const DataType *> *fieldTypes;
	size_t size;
	static HashMap <std::string, RecordType *> map;
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
	~FnType() {
		delete myFormalTypes;
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
