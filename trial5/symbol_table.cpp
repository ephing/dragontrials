#include "symbol_table.hpp"
#include "types.hpp"
namespace cshanty{

SymbolTable::SymbolTable(){
	scopeTableChain = new std::list<ScopeTable *>();
}

SymbolTable::~SymbolTable() {
	for (auto i : *scopeTableChain) delete i;
	delete scopeTableChain;
}

void SymbolTable::print() const{
	unsigned int len = 0;
	std::list<std::string> vars;
	for (auto sc : *scopeTableChain) {
		for (auto i : *(sc->getMap())) {
			std::stringstream temp;
			temp << i.first << ": " << i.second->toString();
			if (temp.str().size() > len) len = temp.str().size();
		}
	}
	for (auto sc : *scopeTableChain) {
		std::cout << "┌";
		for (unsigned int i = 0; i < len + 1; i++) std::cout << "─";
		std::cout << "┐\n";
		for (auto i : *(sc->getMap())) {
			std::cout << "│" << i.first << ": " << i.second->toString();
			for (unsigned int b = i.first.size() + i.second->toString().size() + 2; b < len; b++)
				std::cout << " ";
			std::cout << " │\n";
		}
		std::cout << "└";
		for (unsigned int i = 0; i < len + 1; i++) std::cout << "─";
		std::cout << "┘\n^\n";
	}
	std::cout << "────────────────────────────────\n";
}

ScopeTable * SymbolTable::enterScope(){
	ScopeTable * newScope = new ScopeTable();
	scopeTableChain->push_front(newScope);
	return newScope;
}

void SymbolTable::leaveScope(){
	if (scopeTableChain->empty()){
		throw new InternalError("Attempt to pop"
			"empty symbol table");
	}
	delete scopeTableChain->front();
	scopeTableChain->pop_front();
}

ScopeTable * SymbolTable::getCurrentScope(){
	return scopeTableChain->front();
}

bool SymbolTable::clash(std::string varName){
	bool hasClash = getCurrentScope()->clash(varName);
	return hasClash;
}

SemSymbol * SymbolTable::find(std::string varName){
	for (ScopeTable * scope : *scopeTableChain){
		SemSymbol * sym = scope->lookup(varName);
		if (sym != nullptr) { return sym; }
	}
	return nullptr;
}

bool SymbolTable::insert(SemSymbol * symbol){
	return scopeTableChain->front()->insert(symbol);
}

ScopeTable::ScopeTable(){
	symbols = new HashMap<std::string, SemSymbol *>();
}

ScopeTable::~ScopeTable() {
	delete symbols;
}

std::string ScopeTable::toString(){
	std::string result = "";
	for (auto entry : *symbols){
		result += entry.second->toString();
		result += "\n";
	}
	return result;
}

bool ScopeTable::clash(std::string varName){
	SemSymbol * found = lookup(varName);
	if (found != nullptr){
		return true;
	}
	return false;
}

SemSymbol * ScopeTable::lookup(std::string name){
	auto found = symbols->find(name);
	if (found == symbols->end()){
		return NULL;
	}
	return found->second;
}

bool ScopeTable::insert(SemSymbol * symbol){
	std::string symName = symbol->getName();
	bool alreadyInScope = (this->lookup(symName) != NULL);
	if (alreadyInScope){
		return false;
	}
	this->symbols->insert(std::make_pair(symName, symbol));
	return true;
}

std::string SemSymbol::toString(){
	std::string result = "";
	result += "name: " + this->getName();
	result += ", kind: " + kindToString(this->getKind());
	const DataType * type = this->getDataType();
	if (type == nullptr){
		result += ", type: NULL";
	} else {
		result += ", type: " + this->getDataType()->getString();
	}
	return result;
}

}
