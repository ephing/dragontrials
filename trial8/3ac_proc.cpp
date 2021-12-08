#include "3ac.hpp"
#include <algorithm>

namespace cshanty{

Procedure::Procedure(IRProgram * prog, std::string name)
: myProg(prog), myName(name){
	maxTmp = 0;
	enter = new EnterQuad(this);
	leave = new LeaveQuad(this);
	bodyQuads = new std::list<Quad *>();
	if (myName.compare("main") == 0){
		enter->addLabel(new Label("main"));
	} else {
		enter->addLabel(new Label("fun_" + myName));
	}
	leaveLabel = myProg->makeLabel();
	leave->addLabel(leaveLabel);
}

std::string Procedure::getName(){
	return myName;
}

Label * Procedure::getLeaveLabel(){
	return leaveLabel;
}

IRProgram * Procedure::getProg(){ return myProg; }

std::string Procedure::toString(bool verbose){
	std::string res = "";

	res += "[BEGIN " + this->getName() + " LOCALS]\n";
	for (const auto formal : this->formals){
		res += formal->getName() + " (formal arg of " 
			+ std::to_string(formal->getWidth()) + ")\n";
			+ " bytes)\n";
	}

	for (auto local : this->locals){
		res += local.second->getName() + " (local var of "
			+ std::to_string(local.second->getWidth())
			+ " bytes)\n";
	}

	for (auto tmp : temps){
		res += tmp->locString() + " (tmp var of "
			+ std::to_string(tmp->getWidth())
			+ " bytes)\n";
	}
	for (auto loc : this->addrOpds){
		res += loc->locString() + " (tmp loc of "
			+ std::to_string(loc->getWidth())
			+ " bytes)\n";
	}
	res += "[END " + this->getName() + " LOCALS]\n";

	res += enter->toString(verbose) + "\n";
	for (auto quad : *bodyQuads){
		res += quad->toString(verbose) + "\n";
	}
	res += leave->toString(verbose) + "\n";
	return res;
}

Label * Procedure::makeLabel(){
	return myProg->makeLabel();
}

void Procedure::addQuad(Quad * quad){
	bodyQuads->push_back(quad);
}

Quad * Procedure::popQuad(){
	Quad * last = bodyQuads->back();
	bodyQuads->pop_back();
	return last;
}

void Procedure::gatherLocal(SemSymbol * sym){
	size_t width = Opd::width(sym->getDataType());
	locals[sym] = new SymOpd(sym, width);
}

void Procedure::gatherFormal(SemSymbol * sym){
	size_t width = Opd::width(sym->getDataType());
	formals.push_back(new SymOpd(sym, width));
}

SymOpd * Procedure::getSymOpd(SemSymbol * sym){
	for(auto formalSeek : formals){
		if (formalSeek->getSym() == sym){
			return formalSeek;
		}
	}

	auto localFound = locals.find(sym);
	if (localFound != locals.end()){
		return localFound->second;
	}
	
	return this->getProg()->getGlobal(sym);
}

AuxOpd * Procedure::makeTmp(size_t width){
	std::string name = "tmp";
	name += std::to_string(maxTmp++);
	AuxOpd * res = new AuxOpd(name, width);
	temps.push_back(res);

	return res;
}

AddrOpd * Procedure::makeAddrOpd(size_t width){
	std::string name = "addrTmp";
	name += std::to_string(maxTmp++);
	AddrOpd * res = new AddrOpd(name, width);
	addrOpds.push_back(res);

	return res;
}

size_t Procedure::numTemps() const{
	return this->temps.size();
}

size_t Procedure::arSize() const{
	size_t size = 0;
	for (auto local : locals){
		size += local.second->getWidth();
	}
	for (auto tmp : temps){
		size += tmp->getWidth();
	}
	for (auto formal : formals){
		size += formal->getWidth();
	}
	for (auto loc : addrOpds){
		size += 8;
	}
	size_t slack = (16 - (size % 16)) % 16;
	size += slack;

	return size;
}

}
