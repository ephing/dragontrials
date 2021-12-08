#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string.h>
#include "unistd.h"
#include "errors.hpp"
#include "scanner.hpp"
#include "name_analysis.hpp"
#include "type_analysis.hpp"

using namespace cshanty;

static void usageAndDie(){
	std::cerr << "Usage: cshantyc <infile>\n"
	<< " [-t <tokensFile>]: Output tokens to <tokensFile>\n"
	<< " [-p]: Parse the input to check syntax\n"
	<< " [-u <unparseFile>]: Output canonical program text to <unparseFile>\n"
	<< " [-n <nameFile>]: Output name analysis to <nameFile>\n"
	<< " [-c]: Do type checking\n"
	<< " [-a <3ACFile>]: Output program as 3-address code\n"
	<< " [-o <ASMFile>]: Output x64 assembly to <ASMFile>\n"
	<< " [-l <LLVMFile>]: Output LLVM Bitcode to <LLVMFile>\n"
	;
	std::cout << std::flush;
	std::cerr << std::flush;
	exit(1);
}

static void writeTokenStream(const char * inPath, const char * outPath){
	std::ifstream inStream(inPath);
	if (!inStream.good()){
		std::string msg = "Bad input stream";
		msg += inPath;
		throw new cshanty::InternalError(msg.c_str());
	}
	if (outPath == nullptr){
		std::string msg = "No tokens output file given";
		throw new cshanty::InternalError(msg.c_str());
	}

	Scanner scanner(&inStream);
	if (strcmp(outPath, "--") == 0){
		scanner.outputTokens(std::cout);
	} else {
		std::ofstream outStream(outPath);
		if (!outStream.good()){
			std::string msg = "Bad output file ";
			msg += outPath;
			throw new InternalError(msg.c_str());
		}
		scanner.outputTokens(outStream);
		outStream.close();
	}
}

static cshanty::ProgramNode * parse(const char * inFile){
	std::ifstream inStream(inFile);
	if (!inStream.good()){
		std::string msg = "Bad input stream ";
		msg += inFile;
		throw new InternalError(msg.c_str());
	}

	//This pointer will be set to the root of the
	// AST after parsing
	cshanty::ProgramNode * root = nullptr;

	cshanty::Scanner scanner(&inStream);
	cshanty::Parser parser(scanner, &root);

	int errCode = parser.parse();
	if (errCode != 0){ return nullptr; }

	return root;
}

static void outputAST(ASTNode * ast, const char * outPath){
	if (strcmp(outPath, "--") == 0){
		ast->unparse(std::cout, 0);
	} else {
		std::ofstream outStream(outPath);
		if (!outStream.good()){
			std::string msg = "Bad output file ";
			msg += outPath;
			throw new cshanty::InternalError(msg.c_str());
		}
		ast->unparse(outStream, 0);
	}
}

static cshanty::NameAnalysis * doNameAnalysis(const char * inputPath){
	cshanty::ProgramNode * ast = parse(inputPath);
	if (ast == nullptr){ return nullptr; }
	
	return cshanty::NameAnalysis::build(ast);
}

static bool doUnparsing(const char * inputPath, const char * outPath){
	cshanty::ProgramNode * ast = parse(inputPath);
	if (ast == nullptr){ 
		std::cerr << "No AST built\n";
		return false;
	}

	outputAST(ast, outPath);
	return true;
}

static cshanty::TypeAnalysis * doTypeAnalysis(const char * inputPath){
	cshanty::NameAnalysis * nameAnalysis = doNameAnalysis(inputPath);
	if (nameAnalysis == nullptr){ return nullptr; }
	return TypeAnalysis::build(nameAnalysis);
}

static void write3AC(cshanty::IRProgram * prog, const char * outPath){
	if (outPath == nullptr){
		throw new InternalError("Null 3AC flat file given");
	}
	std::string flatProg = prog->toString();
	if (strcmp(outPath, "--") == 0){
		std::cout << flatProg << std::endl;
	} else {
		std::ofstream outStream(outPath);
		outStream << flatProg << std::endl;
		outStream.close();
	}
}


static IRProgram * do3AC(const char * inputPath){
	cshanty::TypeAnalysis * typeAnalysis = doTypeAnalysis(inputPath);
	if (typeAnalysis == nullptr){ return nullptr; }
	
	IRProgram * prog = typeAnalysis->ast->to3AC(typeAnalysis);
	return prog;
}

static int writeX64(cshanty::IRProgram * prog, const char * outPath){
	if (outPath == nullptr){
		throw new InternalError("Null codegen file given");
	}
	if (strcmp(outPath, "--") == 0){
		prog->toX64(std::cout);
	} else {
		std::ofstream outStream(outPath);
		prog->toX64(outStream);
		outStream.close();
	}
	return 0;
}

static void outputCPP(ASTNode* ast, const char* outPath) {
	if (strcmp(outPath, "--") == 0){
		ast->transpileToC(std::cout, 0);
	} else {
		std::ofstream outStream(outPath);
		if (!outStream.good()){
			std::string msg = "Bad output file ";
			msg += outPath;
			throw new cshanty::InternalError(msg.c_str());
		}
		ast->transpileToC(outStream, 0);
	}
}

static std::string doTranspiling(const char* inputPath, const char* outPath) {
	std::string cfile(outPath);
	cfile += ".c";
	if (outPath == nullptr){
		throw new InternalError("Null codegen file given");
	}
	cshanty::ProgramNode * ast = doTypeAnalysis(inputPath)->ast;
	if (ast == nullptr){ 
		std::cerr << "No AST built\n";
	} else {
		outputCPP(ast, cfile.c_str());
	}
	return cfile;
}

int main( const int argc, const char **argv )
{
	if (argc <= 1){ usageAndDie(); }
	std::ifstream * input = new std::ifstream(argv[1]);
	if (input == nullptr){ usageAndDie(); }
	if (!input->good()){
		std::cerr << "Bad path " << argv[1] << std::endl;
		usageAndDie();
	}

	const char * inFile = NULL;
	const char * tokensFile = NULL;
	bool checkParse = false;
	const char * unparseFile = NULL;
	const char * namesFile = NULL;
	bool checkTypes = false;
	const char * threeACFile = NULL;
	const char * asmFile = NULL;
	const char * llvmFile = NULL;

	bool useful = false;
	int i = 1;
	for (int i = 1 ; i < argc ; i++){
		if (argv[i][0] == '-'){
			if (argv[i][1] == 't'){
				i++;
				tokensFile = argv[i];
				useful = true;
			} else if (argv[i][1] == 'p'){
				checkParse = true;
				useful = true;
			} else if (argv[i][1] == 'u'){
				i++;
				if (i >= argc){ usageAndDie(); }
				unparseFile = argv[i];
				useful = true;
			} else if (argv[i][1] == 'n'){
				i++;
				namesFile = argv[i];
				useful = true;
			} else if (argv[i][1] == 'c'){
				checkTypes = true;
				useful = true;
			} else if (argv[i][1] == 'a'){
				i++;
				if (i >= argc){ usageAndDie(); }
				threeACFile = argv[i];
				useful = true;
			} else if (argv[i][1] == 'o') {
				i++;
				if (i >= argc){ usageAndDie(); }
				asmFile = argv[i];
				useful = true;
			} else if (argv[i][1] == 'l') {
				i++;
				if (i >= argc){ usageAndDie(); }
				llvmFile = argv[i];
				useful = true;
			} else {
				std::cerr << "Unrecognized argument: ";
				std::cerr << argv[i] << std::endl;
				usageAndDie();
			}
		} else {
			if (inFile == NULL){
				inFile = argv[i];
			} else {
				std::cerr << "Only 1 input file allowed";
				std::cerr << argv[i] << std::endl;
				usageAndDie();
			}
		}
	}
	if (inFile == NULL){
		usageAndDie();
	}
	if (!useful){
		std::cerr << "Hey, you didn't tell cshantyc to do anything!\n";
		usageAndDie();
	}

	try {
		if (tokensFile != nullptr){
			writeTokenStream(inFile, tokensFile);
		}
		if (checkParse){
			if (!parse(inFile)){
				std::cerr << "Parse failed" << std::endl;
			}
		}
		if (unparseFile != nullptr){
			doUnparsing(inFile, unparseFile);
		}
		if (namesFile){
			cshanty::NameAnalysis * na;
			na = doNameAnalysis(inFile);
			if (na == nullptr){
				std::cerr << "Name Analysis Failed\n";
				return 1;
			}
			outputAST(na->ast, namesFile);
		}
		if (checkTypes){
			cshanty::TypeAnalysis * ta;
			ta = doTypeAnalysis(inFile);
			if (ta == nullptr){
				std::cerr << "Type Analysis Failed\n";
				return 1;
			}
		}
		if (threeACFile != nullptr){
			auto prog = do3AC(inFile);
			if (prog == nullptr){ return 1; }
			write3AC(prog, threeACFile);
		}
		if (asmFile != nullptr){
			auto prog = do3AC(inFile);
			if (prog == nullptr){ return 1; }
			writeX64(prog, asmFile);
		}
		if (llvmFile != nullptr) {
			std::string cfile = doTranspiling(inFile, llvmFile);
			execl("/usr/bin/clang-9","/usr/bin/clang-9",cfile.c_str(),"-S","-emit-llvm","-o",llvmFile, (char*)NULL);
		}
	} catch (cshanty::ToDoError * e){
		std::cerr << "ToDoError: " << e->msg() << "\n";
		return 1;
	} catch (cshanty::InternalError& e){
		std::cerr << "InternalError: " << e.msg() << "\n";
		return 1;
	} catch (cshanty::InternalError* e) {
		std::cerr << "InternalError: " << e->msg() << "\n";
		return 1;
	}

	return 0;
}
