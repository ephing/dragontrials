#include <ostream>
#include "3ac.hpp"

namespace cshanty{

void IRProgram::allocGlobals(std::ostream& out){
	if (!procs->size()) return;
	std::string globl = "_start";
	for (auto i : *procs) {
		if (i->getName() == "main") globl += ", main";
		else globl += ", fun_" + i->getName();
	}
	out << ".globl " << globl << "\n";
}

void IRProgram::datagenX64(std::ostream& out){
	out << ".data\n";
	for (auto str : strings) {
		out << "\t" << str.first->valString() << ": .asciz " << str.second << "\n";
	}
	for (auto i : this->globals) {
		if (i.first->getKind() == SymbolKind::VAR) {
			if (i.first->getDataType()->asRecord()) {
				// if its a record instance, add adjacent data for all its fields
				for (size_t j = 0; j < i.first->getDataType()->asRecord()->getSize()/8; j++) {
					out << "\tvar_" << i.second->getName() << "_f" << j << ": .quad 0\n";
					i.second->setMemoryLoc("var_" + i.second->getName() + "_f" + std::to_string(j));
				}
			} 
			else {
				out << "\tvar_" << i.second->getName() << ": .quad 0\n";
				i.second->setMemoryLoc("var_" + i.second->getName());
			}
		}
	}
	out << "\t.align 8\n";
}

void IRProgram::toX64(std::ostream& out){
	datagenX64(out);
	allocGlobals(out);
	out << ".text\n";
	for (auto i : *this->procs) {
		i->toX64(out);
	}
}

void Procedure::allocLocals(){
	int dist = 16;
	for (auto i : this->formals) {
		int loc = i->getWidth();
		i->setMemoryLoc("-" + std::to_string(dist + loc) + "(%rbp)");
		dist += loc;
	}
	for (auto i : this->locals) {
		int loc = i.second->getWidth();
		i.second->setMemoryLoc("-" + std::to_string(dist + loc) + "(%rbp)");
		dist += loc;
	}
	for (auto i : this->temps) {
		int loc = i->getWidth();
		i->setMemoryLoc("-" + std::to_string(dist + loc) + "(%rbp)");
		dist += loc;
	}
	for (auto i : this->addrOpds) {
		int loc = i->getWidth();
		i->setMemoryLoc("-" + std::to_string(dist + loc) + "(%rbp)");
		dist += loc;
	}
}

void Procedure::toX64(std::ostream& out){
	//Allocate all locals
	allocLocals();

	enter->codegenLabels(out);
	enter->codegenX64(out);
	for (auto quad : *bodyQuads){
		quad->codegenLabels(out);
		out << "\t";
		quad->codegenX64(out);
	}
	leave->codegenLabels(out);
	leave->codegenX64(out);
}

void Quad::codegenLabels(std::ostream& out){
	if (labels.empty()){ return; }

	size_t numLabels = labels.size();
	size_t labelIdx = 0;
	for ( Label * label : labels){
		out << label->getName() << ": ";
		if (labelIdx != numLabels - 1){ out << "\n"; }
		labelIdx++;
	}
}

void BinOpQuad::codegenX64(std::ostream& out){
	if ((opr == BinOp::SUB64 && src2->valString() == "0") ||
		(opr == BinOp::MULT64 && src2->valString() == "1")) {
		src1->genLoadVal(out,A);
		out << "\t";
		dst->genStoreVal(out,A);
		return;
	}

	src1->genLoadVal(out,A);
	out << "\t";
	if (opr != BinOp::SUB64 || src2->valString() != "1") src2->genLoadVal(out,B);
	switch (opr) {
	case BinOp::ADD64:
		out << "\taddq " << src1->getReg(B) << ", " << src2->getReg(A) << "\n\t";
		break;
	case BinOp::SUB64:
		if (src2->valString() == "1") out << "decq " << src2->getReg(A) << "\n\t";
		else out << "\tsubq " << src1->getReg(B) << ", " << src2->getReg(A) << "\n\t";
		break;
	case BinOp::DIV64:
		out << "\tdivq " << src1->getReg(B) << "\n\t";
		break;
	case BinOp::MULT64:
		out << "\timulq " << src1->getReg(B) << "\n\t";
		break;
	case BinOp::EQ64:
		out << "\tmovq $0, %rcx\n\t";
		out << "cmpq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n\t";
		out << "sete %cl\n\t";
		out << "movq %rcx, %rax\n\t";
		break;
	case BinOp::NEQ64:
		out << "\tmovq $0, %rcx\n\t";
		out << "cmpq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n\t";
		out << "setne %cl\n\t";
		out << "movq %rcx, %rax\n\t";
		break;
	case BinOp::LT64:
		out << "\tmovq $0, %rcx\n\t";
		out << "cmpq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n\t";
		out << "setl %cl\n\t";
		out << "movq %rcx, %rax\n\t";
		break;
	case BinOp::LTE64:
		out << "\tmovq $0, %rcx\n\t";
		out << "cmpq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n\t";
		out << "setle %cl\n\t";
		out << "movq %rcx, %rax\n\t";
		break;
	case BinOp::GT64:
		out << "\tmovq $0, %rcx\n\t";
		out << "cmpq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n\t";
		out << "setg %cl\n\t";
		out << "movq %rcx, %rax\n\t";
		break;
	case BinOp::GTE64:
		out << "\tmovq $0, %rcx\n\t";
		out << "cmpq " << src2->getReg(B) << ", " << src1->getReg(A) << "\n\t";
		out << "setgt %cl\n\t";
		out << "movq %rcx, %rax\n\t";
		break;
	case BinOp::OR64:
	 	out << "\torq " << src1->getReg(B) << ", " << src2->getReg(A) << "\n\t";
		break;
	case BinOp::AND64:
		out << "\tandq " << src1->getReg(B) << ", " << src2->getReg(A) << "\n\t";
		break;
	default: break;
	}
	dst->genStoreVal(out,A);
}

void UnaryOpQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);
	switch (op) {
	case UnaryOp::NEG64: 
		out << "\tnegq " << src->getReg(A) << "\n\t";
		break;
    case UnaryOp::NOT64:
		out << "\txorq $1, " << src->getReg(A) << "\n\t";
		break;
	default: break;
	}
	dst->genStoreVal(out,A);
}

void AssignQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);
	out << "\t";
	dst->genStoreVal(out, A);
}

void GotoQuad::codegenX64(std::ostream& out){
	out << "jmp " << tgt->getName() << "\n";
}

void IfzQuad::codegenX64(std::ostream& out){
	if (cnd->valString() == "0") {
		out << "jmp " << tgt->getName() << "\n";
		return;
	} else if (cnd->valString() == "1") return;
	out << "cmpq $0," << cnd->getMemoryLoc() << "\n\t";
	out << "je " << tgt->getName() << "\n";
}

void NopQuad::codegenX64(std::ostream& out){
	out << "nop" << "\n";
}

void IntrinsicOutputQuad::codegenX64(std::ostream& out){
	if (myType->isBool()){
		myArg->genLoadVal(out, DI);
		out << "\tcallq printBool\n";
	} else if (myType->isInt()) {
		myArg->genLoadVal(out,DI);
		out << "\tcallq printInt\n";
	} else {
		myArg->genLoadVal(out,DI);
		out << "\tcallq printString\n";
	}
}

void IntrinsicInputQuad::codegenX64(std::ostream& out){
	if (myType->isBool()){
		myArg->genLoadVal(out, DI);
		out << "\tcallq getBool\n\t";
		myArg->genStoreVal(out,A);
	} else if (myType->isInt()) {
		myArg->genLoadVal(out,DI);
		out << "\tcallq getInt\n\t";
		myArg->genStoreVal(out,A);
	} else throw InternalError("Attempt to receive string");
}

void CallQuad::codegenX64(std::ostream& out){
	out << "callq fun_" << callee->getName() << "\n";
	auto t = callee->getDataType()->asFn()->getFormalTypes();
	if (t->size() > 6) out << "\taddq $" << 8*(t->size() - 6) << ", %rsp # pop those extra args \n";
}

void EnterQuad::codegenX64(std::ostream& out){
	out << "\n\tpushq %rbp\n";
	out << "\tmovq %rsp, %rbp\n";
	out << "\taddq $16, %rbp\n";
	out << "\tsubq $" << this->myProc->arSize() << ", %rsp\n\t # ^Function Header^\n";
}

void LeaveQuad::codegenX64(std::ostream& out){
	out << "\n\taddq $" << this->myProc->arSize() << ", %rsp\n";
	out << "\tpopq %rbp\n";
	out << "\tretq\n";
}

void SetArgQuad::codegenX64(std::ostream& out){
	Register r;
	switch (index) {
	case 1: r = DI; break;
	case 2: r = SI; break;
	case 3: r = D; break;
	case 4: r = C; break;
	case 5: r = T8; break;
	case 6: r = T9; break;
	default: break;
	}
	if (index < 7) {
		if (type->isRecord()) {
			out << "leaq " << getSrc()->getMemoryLoc() << ", "<<RegUtils::reg64(r)<<" # get addr of record arg\n";
		}
		else getSrc()->genLoadVal(out,r);
	} else {
		if (type->isRecord()) {
			out << "leaq " << getSrc()->getMemoryLoc() << ", %rax # get addr of record arg\n\t";
		} else {
			getSrc()->genLoadVal(out,A);
			out << "\t";
		}
		out << "pushq %rax # move arg to stack\n";
	}
}

void GetArgQuad::codegenX64(std::ostream& out){
	Register r;
	switch (index) {
	case 1: r = DI; break;
	case 2: r = SI; break;
	case 3: r = D; break;
	case 4: r = C; break;
	case 5: r = T8; break;
	case 6: r = T9; break;
	default: r = C; break;
	}
	int loc = 8 * (numFormals - index);
	if (!isRecord()) {
		if (index < 7) getDst()->genStoreVal(out,r);
		else {
			out << getDst()->getMovOp() << loc << "(%rbp), %rax # move arg into %rax\n\t";
			out << getDst()->getMovOp() << "%rax, " << getDst()->getMemoryLoc() << " # move formal into act. record\n";
		}
	} else {
		if (index > 6) out << "movq " << loc << "(%rbp), "<<RegUtils::reg64(r)<<" # get address of record\n\t";
		out << "leaq " << opd->getMemoryLoc() << ", %rbx # get wanted loc\n";
		for (size_t i = 0; i < opd->getWidth(); i+=8) {
			out << "\tmovq " << i << "(" << RegUtils::reg64(r) << "), %rax # get value into %rax\n\t";
			out << "movq %rax, " << i << "(%rbx) # put value in address in %rbx\n";
		}
	}
}

void SetRetQuad::codegenX64(std::ostream& out){
	getSrc()->genLoadVal(out,A);
}

void GetRetQuad::codegenX64(std::ostream& out){
	opd->genStoreVal(out,A);
}

void IndexQuad::codegenX64(std::ostream& out){
	src->genLoadAddr(out,A);
	out << "\taddq $" << off->valString() << ", %rax # Index, add addr offset\n\t";
	dst->genStoreAddr(out,A);
}

void SymOpd::genLoadVal(std::ostream& out, Register reg){
	out << getMovOp() << getMemoryLoc() << ", " << getReg(reg) << "\n";
}

void SymOpd::genStoreVal(std::ostream& out, Register reg){
	out << getMovOp() << getReg(reg) << ", " << this->getMemoryLoc() << "\n";
}

void SymOpd::genLoadAddr(std::ostream& out, Register reg) {
	out << "leaq " << getMemoryLoc() << ", " << RegUtils::reg64(reg) << "\n";
}

void AuxOpd::genLoadVal(std::ostream& out, Register reg){
	out << getMovOp() << this->getMemoryLoc() << ", " << getReg(reg) << "\n";
}

void AuxOpd::genStoreVal(std::ostream& out, Register reg){
	out << getMovOp() << getReg(reg) << ", " << getMemoryLoc() << "\n";
}

void AuxOpd::genLoadAddr(std::ostream& out, Register reg){
	TODO(Implement me)
}


void AddrOpd::genStoreVal(std::ostream& out, Register reg){
	//store address in rbx
	out << getMovOp() << getMemoryLoc() << ", " << getReg(B) << "\n\t";
	//store reg value in value at the address in rbx
	out << getMovOp() << getReg(reg) << ", (" << getReg(B) << ")\n";
}

void AddrOpd::genLoadVal(std::ostream& out, Register reg){
	//store address in rbx
	out << getMovOp() << getMemoryLoc() << ", " << getReg(B) << "\n\t";
	//store value at the address in rbx in reg
	out << getMovOp() << "(" << getReg(B) << "), " << getReg(reg) << "\n";
}

void AddrOpd::genStoreAddr(std::ostream& out, Register reg){
	out << getMovOp() << getReg(reg) << ", " << getMemoryLoc() << "\n";
}

void AddrOpd::genLoadAddr(std::ostream & out, Register reg){
	out << getMovOp() << getMemoryLoc() << ", " << getReg(reg) << "\n";
}

void LitOpd::genLoadVal(std::ostream & out, Register reg){
	out << getMovOp() << "$" << val << ", " << getReg(reg) << "\n";
}

}
