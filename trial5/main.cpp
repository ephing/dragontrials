#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "errors.hpp"
#include "scanner.hpp"
#include "symbol_table.hpp"
#include "type_analysis.hpp"
#include "evaluation.hpp"
#include "environment.hpp"

using namespace cshanty;
int deferParse(std::stringstream&, int);
int getDepth(std::string);

static std::ifstream inStream;

static void interp(std::stringstream& s,ProgramNode * root, SymbolTable* symTab, Environment* env){
	if (s.str() == "" || s.str().substr(0,2) == "//") return;
	cshanty::Scanner scanner(&s);
	cshanty::Parser parser(scanner, root->globs());
	int errCode = parser.parse();
	bool success;
	if (!errCode) {
		success = root->nameAnalysis(symTab);
		success = success && TypeAnalysis::build(root);
		if (success) {
			try {
				const Eval* e = (*root->globs())->back()->eval(env);
				if (!inStream.is_open()) e->printResult();
				delete e;
			} catch (EvaluationError* e) {
				std::cout << e->msg() << "\n";
			}
			if ((*root->globs())->back()->isGlobEval()) {
				delete (*root->globs())->back();
				(*root->globs())->pop_back();
			}
		} else {
			delete (*root->globs())->back();
			(*root->globs())->pop_back();
		}
	}
}

int deferParse(std::stringstream& s, int depth) {
	int count = depth;
	while (count > 0) {
		count = depth;
		std::string temp;
		if (inStream.is_open()) {
			getline(inStream,temp);
		} else {
			for (int i = 0; i < depth; i++) {
				std::cout << ". ";
			}
			getline(std::cin,temp);
		}
		s << temp;
		count += getDepth(temp);
		while (count > depth) {
			count = deferParse(s,count);
		}
		if (count < depth) break;
	}
	return count;
}

int getDepth(std::string s) {
	int count = 0;
	for (char c : s) {
		if (c == '}') count--;
		else if (c == '{') count++;
	}
	return count;
}

int main( const int argc, const char **argv )
{
	cshanty::ProgramNode * root = new ProgramNode(new std::vector<DeclNode*>());
	SymbolTable* symTab = new SymbolTable();
	Environment* env = new Environment();
	std::string input;

	if (argc == 1) 
		std::cout << "> Welcome to dragoninterp! Enter C-Shanty code to be interpreted...\n";
	if (argc > 2) {
		std::cout << "Format: ./dragoninterp <optional .cshanty>\n";
		delete env;
		delete symTab;
		delete root;
		return 1;
	}
	else if (argc == 2) inStream.open(argv[1]);
	if (inStream.bad()) {
		std::cout << "Input file not found!\n";
		delete env;
		delete symTab;
		delete root;
		return 1;
	}
	while (!inStream.is_open() || (!inStream.eof())) {
		input = "";
		std::stringstream strstr;
		if (argc == 2) getline(inStream,input);
		else getline(std::cin,input);
		//special commands
		if (!inStream.is_open() && input == ":exit") break;
		if (!inStream.is_open() && input == ":clear") {
			std::cout << "\x1b[2J\x1b[1;1H> Welcome to dragoninterp! Enter C-Shanty code to be interpreted...\n";
			continue;
		}
		if (!inStream.is_open() && input == ":env") {
			env->print();
			continue;
		}

		strstr << input;
		int d = getDepth(input);
		while (d > 0) d = deferParse(strstr, d);

		//parse
		try {
			interp(strstr,root, symTab, env);
		} catch (cshanty::ToDoError * e){
			std::cerr << "ToDoError: " << e->msg() << "\n";
			delete env;
			delete symTab;
			delete root;
			BasicType::destroyPrimitives();
			RecordType::destroyRecords();
			ErrorType::destroy();
			return 1;
		} catch (cshanty::InternalError * e){
			std::cerr << "InternalError: " << e->msg() << "\n";
			delete env;
			delete symTab;
			delete root;
			BasicType::destroyPrimitives();
			RecordType::destroyRecords();
			ErrorType::destroy();
			return 1;
		}
		strstr.clear();
	}
	if (argc == 2) inStream.close();
	delete env;
	delete symTab;
	delete root;
	BasicType::destroyPrimitives();
	RecordType::destroyRecords();
	ErrorType::destroy();
	return 0;
}
