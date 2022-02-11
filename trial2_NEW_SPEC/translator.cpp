#include <map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <stack>
const char* look = "";
//this grammar does basic list concatenation

#include <iostream>
std::stack<std::vector<int>> semStack;
std::vector<int> pop(){
	std::vector<int> res = semStack.top();
	semStack.pop();
	return res;
}

void runActions(int act) {
	switch (act) {
	case 1: {
		std::vector<int> t = pop();
		std::cout << "[";
		for (int i : t) std::cout << i << ",";
		std::cout << "]\n";
		return;
	}
	case 2: {
		std::vector<int> t2 = pop();
		std::vector<int> t1 = pop();
		for (int i : t2) t1.push_back(i);
		semStack.push(t1);
		return;
	}
	case 3: {
		semStack.push(std::vector<int>());
		return;
	}
	case 4: {
		std::vector<int> t = pop();
		t.push_back(std::atoi(look));
		semStack.push(t);
		return;
	}

	}
}

std::vector<std::string> terms, nonterms;
int lookahead = 0;
//Converting my pretty Python maps to C++ is Distgusting but must be done bc my old trial 2 was all in python
using STable = std::map<std::string,std::map<std::string,std::vector<std::string>>>;
STable* s = nullptr;
static STable* getTable() {
	if (s != nullptr) return s;
	s = new STable();
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("Z",std::map<std::string,std::vector<std::string>>()));
	(*s)["Z"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["Z"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["Z"]["["].push_back("S");
	(*s)["Z"]["["].push_back("#1");
	(*s)["Z"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["Z"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["Z"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["Z"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("S",std::map<std::string,std::vector<std::string>>()));
	(*s)["S"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["S"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["S"]["["].push_back("N");
	(*s)["S"]["["].push_back("T");
	(*s)["S"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["S"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["S"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["S"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("T",std::map<std::string,std::vector<std::string>>()));
	(*s)["T"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["T"]["+"].push_back("+");
	(*s)["T"]["+"].push_back("S");
	(*s)["T"]["+"].push_back("#2");
	(*s)["T"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["T"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["T"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["T"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["T"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	(*s)["T"]["\30"].push_back("");
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("N",std::map<std::string,std::vector<std::string>>()));
	(*s)["N"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["N"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["N"]["["].push_back("#3");
	(*s)["N"]["["].push_back("[");
	(*s)["N"]["["].push_back("E");
	(*s)["N"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["N"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["N"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["N"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("E",std::map<std::string,std::vector<std::string>>()));
	(*s)["E"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["E"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["E"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["E"]["]"].push_back("]");
	(*s)["E"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["E"]["INTLIT"].push_back("X");
	(*s)["E"]["INTLIT"].push_back("]");
	(*s)["E"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["E"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("X",std::map<std::string,std::vector<std::string>>()));
	(*s)["X"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["X"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["X"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["X"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["X"]["INTLIT"].push_back("#4");
	(*s)["X"]["INTLIT"].push_back("INTLIT");
	(*s)["X"]["INTLIT"].push_back("B");
	(*s)["X"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["X"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("B",std::map<std::string,std::vector<std::string>>()));
	(*s)["B"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["B"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["B"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["B"]["]"].push_back("");
	(*s)["B"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["B"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["B"][","].push_back(",");
	(*s)["B"][","].push_back("X");
	(*s)["B"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	s->insert(std::pair<std::string,std::map<std::string,std::vector<std::string>>>("\31",std::map<std::string,std::vector<std::string>>()));
	(*s)["\31"].insert(std::pair<std::string,std::vector<std::string>>("+",std::vector<std::string>()));
	(*s)["\31"].insert(std::pair<std::string,std::vector<std::string>>("[",std::vector<std::string>()));
	(*s)["\31"]["["].push_back("Z");
	(*s)["\31"]["["].push_back("\30");
	(*s)["\31"].insert(std::pair<std::string,std::vector<std::string>>("]",std::vector<std::string>()));
	(*s)["\31"].insert(std::pair<std::string,std::vector<std::string>>("INTLIT",std::vector<std::string>()));
	(*s)["\31"].insert(std::pair<std::string,std::vector<std::string>>(",",std::vector<std::string>()));
	(*s)["\31"].insert(std::pair<std::string,std::vector<std::string>>("\30",std::vector<std::string>()));
	nonterms.push_back("Z");
	terms.push_back("+");
	terms.push_back("[");
	terms.push_back("]");
	terms.push_back("INTLIT");
	terms.push_back(",");
	terms.push_back("\30");
	nonterms.push_back("S");
	nonterms.push_back("T");
	nonterms.push_back("N");
	nonterms.push_back("E");
	nonterms.push_back("X");
	nonterms.push_back("B");
	nonterms.push_back("\31");
	return s;
}

struct Token {
	enum class Type {
		TERMINAL,
		NONTERMINAL,
		ACTION
	};
	std::string symbol;
	std::string lexeme;
	Type type;
	int l, c;
	//Token from input token stream
	Token(std::string n1, std::string n2) : symbol(), lexeme(), type(Type::TERMINAL),l(-1),c(-1) {
		int t = 0;
		while (n1.size()) {
			if (!t) {
				if (n1 != "" && n1[0] != ':') {
					symbol += n1[0];
					n1 = n1.substr(1);
				} else if (n1[0] == ':') {
					t = 1;
					n1 = n1.substr(1);
				} else break;
			}
			else if (t == 1) {
				if (n1 != "") {
					lexeme += n1[0];
					n1 = n1.substr(1);
				} else break;
			}
		}
		std::string temp;
		n2 = n2.substr(1);
		while (n2[0] != ',') {
			temp += n2[0];
			n2 = n2.substr(1);
		}
		n2 = n2.substr(1);
		l = std::atoi(temp.c_str());
		temp = "";
		while (n2[0] != ']') {
			temp += n2[0];
			n2 = n2.substr(1);
		}
		c = std::atoi(temp.c_str());
	}
	//Token generated by parser to put in stack
	Token(std::string line) : symbol(), lexeme(), type(Type::TERMINAL), l(-1), c(-1) {
		if (line[0] == '#') {
			type = Type::ACTION;
			symbol = line.substr(1);
		} else if (std::count(nonterms.begin(),nonterms.end(),line)) {
			type = Type::NONTERMINAL;
			symbol = line;
		} else {
			symbol = line;
		}
	}
};

std::vector<Token> tks;
class LL1Parser {
private:
	std::stack<Token> stack;
public:
	LL1Parser(){ stack.push(Token("\31")); }
	~LL1Parser() { }
	bool parse() {
		while (stack.size()) {
			if (stack.top().type == Token::Type::TERMINAL) {
				if (stack.top().symbol == tks.at(lookahead).symbol) {
					lookahead++;
					if ((long unsigned int)lookahead < tks.size())
						look = tks.at(lookahead).lexeme.c_str();
					stack.pop();
				} else return false;
			} else if (stack.top().type == Token::Type::NONTERMINAL) {
				Token val = stack.top();
				stack.pop();
				std::vector<std::string> prod = (*getTable())[val.symbol][tks[lookahead].symbol];
				if (prod.size()) {
					for (int i = prod.size() - 1; i > -1 && prod.at(0) != ""; i--) {
						stack.push(Token(prod[i]));
					}
				} else return false;
			} else {
				runActions(std::atoi(stack.top().symbol.c_str()));
				stack.pop();
			}
		}
		return true;
	}
};

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Format: ./translator <.tokens>\n";
		return 1;
	}
	STable* lltable = getTable();
	LL1Parser* parser = new LL1Parser();
	std::ifstream tkstream(argv[1]);
	std::string in1,in2;
	while (tkstream >> in1 >> in2) tks.push_back(Token(in1,in2));
	tkstream.close();
	if (tks.at(tks.size() - 1).symbol != "\30") {
		int l = tks.at(tks.size() - 1).l + 1;
		tks.push_back(Token("\30","["+std::to_string(l)+",1]"));
	}
	look = tks.at(0).lexeme.c_str();
	if (parser->parse()) std::cout << "accepted\n";
	else {
		std::cout << "rejected\n";
		return 1;
	}
	delete lltable;
	delete parser;
}