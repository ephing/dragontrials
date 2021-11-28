#include <ostream>
#include "3ac.hpp"

namespace cshanty{

static bool iOB = false;

void IRProgram::datagenMIPS(std::ostream& out){
	out << ".data\n";
	for (auto str : strings) {
		out << "\t" << str.first->valString() << ": .asciiz " << str.second << "\n";
	}
	for (auto i : this->globals) {
		if (i.first->getKind() == SymbolKind::VAR) {
			if (i.first->getDataType()->asRecord()) {
				// if its a record instance, add adjacent data for all its fields
				for (size_t j = 0; j < i.first->getDataType()->asRecord()->getSize()/8; j++) {
					out << "\tvar_" << i.second->getName() << "_f" << j << ": .word 0\n";
					i.second->setMemoryLoc("var_" + i.second->getName() + "_f" + std::to_string(j));
				}
			} 
			else {
				out << "\tvar_" << i.second->getName() << ": .word 0\n";
				i.second->setMemoryLoc("var_" + i.second->getName());
			}
		}
	}
	out << "\t.align 3\n";
}

void IRProgram::toMIPS(std::ostream& out){
	datagenMIPS(out);
	out << ".text\n";
	out << "jal main\n";
	out << "addi $v0 $0 17\n";
	out << "addi $a0 $0 0\n";
	out << "syscall\n";
	/*
	if $a0=1 la $v0 str_true
	else la $v0 str_false
	*/
	for (auto i : *this->procs) {
		i->toMIPS(out);
	}
	if (iOB) {
		out << "sys_boolstr:\n";
		out << "\tbeq $a0 $0 SYS_F\n";
		out << "\tla $v0 str_true\n";
		out << "\tj SYS_L\n";
		out << "SYS_F:\n\tla $v0 str_false\n";
		out << "SYS_L:\n\tjr $ra\n";
		out << ".data\n";
		out << "\tstr_true: .asciiz \"true\"\n";
		out << "\tstr_false: .asciiz \"false\"\n";
	}
}

void Procedure::allocLocalsMIPS(){
	int dist = 8;
	for (auto i : this->formals) {
		int loc = i->getWidth()/2;
		i->setMemoryLoc("-" + std::to_string(dist + loc) + "($fp)");
		dist += loc;
	}
	for (auto i : this->locals) {
		int loc = i.second->getWidth()/2;
		i.second->setMemoryLoc("-" + std::to_string(dist + loc) + "($fp)");
		dist += loc;
	}
	for (auto i : this->temps) {
		int loc = i->getWidth()/2;
		i->setMemoryLoc("-" + std::to_string(dist + loc) + "($fp)");
		dist += loc;
	}
	for (auto i : this->addrOpds) {
		int loc = i->getWidth()/2;
		i->setMemoryLoc("-" + std::to_string(dist + loc) + "($fp)");
		dist += loc;
	}
}

void Procedure::toMIPS(std::ostream& out){
	//Allocate all locals
	allocLocalsMIPS();

	enter->codegenLabels(out);
	enter->codegenMIPS(out);
	for (auto quad : *bodyQuads){
		quad->codegenLabels(out);
		out << "\t";
		quad->codegenMIPS(out);
	}
	leave->codegenLabels(out);
	leave->codegenMIPS(out);
}

void BinOpQuad::codegenMIPS(std::ostream& out){
	src1->genLoadValMIPS(out,T0);
	out << "\t";
	src2->genLoadValMIPS(out,T1);
	switch (opr) {
	case BinOp::ADD64:
		out << "\tadd " << src2->getReg32(T0) << " " << src1->getReg32(T1) 
			<< " " << src2->getReg32(T0) << "\n\t";
		break;
	case BinOp::SUB64:
		out << "\tsub " << src1->getReg32(T0) << " " << src1->getReg32(T0) 
			<< " " << src2->getReg32(T1) << "\n\t";
		break;
	case BinOp::DIV64:
		out << "\tdiv " << src1->getReg32(T0) << " " << src2->getReg32(T1) << "\n\t";
		out << "mflo " << src1->getReg32(T0) << "\n\t";
		break;
	case BinOp::MULT64:
		out << "\tmul " << src1->getReg32(T0) << " " << src1->getReg32(T0) 
			<< " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::EQ64:
		/*out << "\tsubu " << src1->getReg32(T0) << " " << src1->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		out << "sltu " << src1->getReg32(T0) << " $0 " << src1->getReg32(T0) << "\n\t";
		out << "xori " << src1->getReg32(T0) << " " << src1->getReg32(T0) << " 1\n\t";*/
		out << "seq " << src1->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::NEQ64:
		/*out << "\tsubu " << src1->getReg32(T0) << " " << src1->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		out << "sltu " << src1->getReg32(T0) << " $0 " << src1->getReg32(T0) << "\n\t";*/
		out << "sne " << src1->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::LT64:
		out << "\tslt " << src1->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::LTE64:
		out << "\tsle " << src1->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::GT64:
		out << "\tsgt " << src1->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::GTE64:
		out << "\tsge " << src1->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::OR64:
	 	out << "\tor " << src2->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	case BinOp::AND64:
		out << "\tand " << src2->getReg32(T0) << " " << src2->getReg32(T0) << " " << src1->getReg32(T1) << "\n\t";
		break;
	default: break;
	}
	dst->genStoreValMIPS(out,T0);
}

void UnaryOpQuad::codegenMIPS(std::ostream& out){
	src->genLoadValMIPS(out, T0);
	switch (op) {
	case UnaryOp::NEG64: 
		out << "\tneg $t0 $t0\n\t";
		break;
    case UnaryOp::NOT64:
		out << "\txori $t0 $t0 1\n\t";
		break;
	default: break;
	}
	dst->genStoreValMIPS(out,T0);
}

void AssignQuad::codegenMIPS(std::ostream& out){
	src->genLoadValMIPS(out, T0);
	out << "\t";
	dst->genStoreValMIPS(out, T0);
}

void GotoQuad::codegenMIPS(std::ostream& out){
	out << "j " << tgt->getName() << "\n";
}

void IfzQuad::codegenMIPS(std::ostream& out){
	cnd->genLoadValMIPS(out,T0);
	out << "\tbeq $0 $t0 " << tgt->getName() << "\n";
}

void NopQuad::codegenMIPS(std::ostream& out){
	out << "nop" << "\n";
}

void IntrinsicOutputQuad::codegenMIPS(std::ostream& out){
	if (myType->isInt()){
		out << "addi $v0 $0 1\n\t";
		myArg->genLoadValMIPS(out,A0);
	} else if (myType->isBool()) {
		// I wanted to make it output the strings "true" and "false" like the x64 version does
		// however I realized this was more than a 1 liner because it would require a branch with labels
		// so I hardcoded the function sys_boolstr that returns the string based on whether $a0 is 1 or 0
		// None of the labels used in this function should conflict with the 3ac labels because those always
		// have to start with str_, var_, or fun_
		myArg->genLoadValMIPS(out,A0);
		out << "\tjal sys_boolstr\n\t";
		out << "move $a0 $v0\n\t";
		out << "addi $v0 $0 4\n";
		// I also added this variable so that the sys_boolstr function and global true and false strings
		// only appear if they are actually used
		iOB = true;
	} else {
		out << "addi $v0 $0 4\n\t";
		// literal: la $a0 str_0
		// not literal: lw $a0 -n($fp)
		myArg->genLoadValMIPS(out,A0);
	}
	out << "\tsyscall\n";
}

void IntrinsicInputQuad::codegenMIPS(std::ostream& out){
	out << "addi $v0 $0 5\n\t";
	out << "syscall\n\t";
	if (myType->isBool()){
		//normalize to 1 or 0
		out << "sltu $v0 $0 $v0 \n\t";
	} else if (myType->isString()) throw InternalError("Attempt to receive string");
	out << "sw $v0 " << myArg->getMemoryLoc() << "\n";
}

void CallQuad::codegenMIPS(std::ostream& out){
	out << "jal fun_" << callee->getName() << "\n";
	auto t = callee->getDataType()->asFn()->getFormalTypes();
	if (t->size() > 4) out << "\taddi $sp $sp " << 4*(t->size() - 4) << " # pop those extra args \n";
}

void EnterQuad::codegenMIPS(std::ostream& out){
	out << "\n\taddi $sp $sp -" << this->myProc->arSizeMIPS() << "\n";
	out << "\tsw $ra " << this->myProc->arSizeMIPS()-4 << "($sp)\n";
	out << "\tsw $fp " << this->myProc->arSizeMIPS()-8 << "($sp)\n";
	out << "\taddi $fp $sp " << this->myProc->arSizeMIPS() << "\n\t";
	out << "# ^Function Header^\n";
}

void LeaveQuad::codegenMIPS(std::ostream& out){
	out << "\n\tlw $fp " << this->myProc->arSizeMIPS()-8 << "($sp)\n";
	out << "\tlw $ra " << this->myProc->arSizeMIPS()-4 << "($sp)\n";
	out << "\taddi $sp $sp " << this->myProc->arSizeMIPS() << "\n";
	out << "\tjr $ra\n";
}

void SetArgQuad::codegenMIPS(std::ostream& out){
	Register r;
	switch (index) {
	case 1: r = A0; break;
	case 2: r = A1; break;
	case 3: r = A2; break;
	case 4: r = A3; break;
	default: break;
	}
	if (index < 5) {
		if (type->isRecord()) {
			int offset = std::stoi(opd->getMemoryLoc().substr(0,opd->getMemoryLoc().find("(")));
			out << "addi " << opd->getReg32(r) << " $fp " << offset << " # get addr of record arg\n";
		}
		else getSrc()->genLoadValMIPS(out,r);
	} else {
		if (type->isRecord()) {
			int offset = std::stoi(opd->getMemoryLoc().substr(0,opd->getMemoryLoc().find("(")));
			out << "addi $t0 $fp " << offset << " # get addr of record arg\n\t";
		} else {
			getSrc()->genLoadValMIPS(out,T0);
			out << "\t";
		}
		out << "sw $t0 -4($sp)\n\t";
		out << "addi $sp $sp -4 # move arg to stack\n";
	}
}

void GetArgQuad::codegenMIPS(std::ostream& out){
	Register r;
	switch (index) {
	case 1: r = A0; break;
	case 2: r = A1; break;
	case 3: r = A2; break;
	case 4: r = A3; break;
	default: r = T0; break;
	}
	int loc = 4 * (numFormals - index);
	if (!isRecord()) {
		if (index < 5) getDst()->genStoreValMIPS(out,r);
		else {
			out << "lw $t0 " << loc << "($fp) # move arg into %rax\n\t";
			out << "sw $t0 " << getDst()->getMemoryLoc() << " # move formal into act. record\n";
		}
	} else {
		if (index > 4) out << "lw " << opd->getReg32(r) <<" "<< loc << "($fp) # get address of record\n\t";
		int offset = std::stoi(opd->getMemoryLoc().substr(0,opd->getMemoryLoc().find("(")));
		out << "addi $t1 $fp " << offset << " # get wanted loc\n";
		for (size_t i = 0; i < opd->getWidth()/2; i+=4) {
			out << "\tlw $t2 " << i << "(" << opd->getReg32(r) << ") # get value into %rax\n\t";
			out << "sw $t2 " << i << "($t1) # put value in address in %rbx\n";
		}
	}
}

void SetRetQuad::codegenMIPS(std::ostream& out){
	getSrc()->genLoadValMIPS(out,V0);
}

void GetRetQuad::codegenMIPS(std::ostream& out){
	opd->genStoreValMIPS(out,V0);
}

void IndexQuad::codegenMIPS(std::ostream& out){
	std::string loc = src->getMemoryLoc();
	if (loc.find("(") != std::string::npos) {
		int offset = std::stoi(src->getMemoryLoc().substr(0,src->getMemoryLoc().find("("))) 	
			+ std::stoi(off->valString())/2;
		out << "addi $t0 $fp " << offset << "\n\t";
	}
	else {
		out << "la $t0 " << loc << "\n\t";
		out << "addi $t0 $t0 " << off->valString() << "\n\t";
	}
	dst->genStoreAddrMIPS(out,T0);
}

void SymOpd::genLoadValMIPS(std::ostream& out, Register reg){
	out << "lw " << getReg32(reg) << ", " << getMemoryLoc() << "\n";
}

void SymOpd::genStoreValMIPS(std::ostream& out, Register reg){
	out << "sw " << getReg32(reg) << ", " << getMemoryLoc() << "\n";
}

void SymOpd::genLoadAddrMIPS(std::ostream& out, Register reg) {
	TODO(Implement me)
}

void AuxOpd::genLoadValMIPS(std::ostream& out, Register reg){
	out << "lw " << getReg32(reg) << " " << getMemoryLoc() << "\n";
}

void AuxOpd::genStoreValMIPS(std::ostream& out, Register reg){
	out << "sw " << getReg32(reg) << ", " << getMemoryLoc() << "\n";
}

void AuxOpd::genLoadAddrMIPS(std::ostream& out, Register reg){
	TODO(Implement me)
}


void AddrOpd::genStoreValMIPS(std::ostream& out, Register reg){
	out << "lw $t2 " << getMemoryLoc() << "\n\t";
	out << "sw " << getReg32(reg) << " 0($t2)\n";
}

void AddrOpd::genLoadValMIPS(std::ostream& out, Register reg){
	out << "lw " << getReg32(reg) << " " << getMemoryLoc() << "\n\t";
	out << "lw " << getReg32(reg) << " 0(" << getReg32(reg) << ")\n";
}

void AddrOpd::genStoreAddrMIPS(std::ostream& out, Register reg){
	out << "sw " << getReg32(reg) << " " << getMemoryLoc() << "\n";
}

void AddrOpd::genLoadAddrMIPS(std::ostream & out, Register reg){
	TODO(Implement me)
}

void LitOpd::genLoadValMIPS(std::ostream & out, Register reg){
	if (this->isStr) out << "la " << getReg32(reg) << " " << valString() << "\n";
	else out << "addi " << getReg32(reg) << " $0 " << val << "\n";
}

}
