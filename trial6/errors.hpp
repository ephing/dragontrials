#ifndef CSHANTY_ERRRORS_H
#define CSHANTY_ERRRORS_H

#define EXPAND2(x) #x
#define EXPAND1(x) EXPAND2(x)
#define CODELOC __FILE__ ":" EXPAND1(__LINE__) " - "
#define TODO(x) throw new ToDoError(CODELOC #x);

#include <iostream>
#include "position.hpp"

namespace cshanty{

class InternalError{
public:
	InternalError(const char * msgIn) : myMsg(msgIn){}
	std::string msg(){ return myMsg; }
private:
	std::string myMsg;
};

class ToDoError{
public:
	ToDoError(const char * msgIn) : myMsg(msgIn){}
	const char * msg(){ return myMsg; }
private:
	const char * myMsg;
};

class Report{
public:
	static void fatal(
		Position * pos,
		const char * msg
	){
		std::cerr << "FATAL " 
		<< pos->span()
		<< ": " 
		<< msg  << std::endl;
	}

	static void fatal(
		Position * pos,
		const std::string msg
	){
		fatal(pos,msg.c_str());
	}

	static void warn(
		Position * pos,
		const char * msg
	){
		std::cerr << "WARNING "
		<< pos->span()
		<< " " 
		<< msg  << std::endl;
	}

	static void warn(
		Position * pos,
		const std::string msg
	){
		warn(pos,msg.c_str());
	}
};

}

#endif
