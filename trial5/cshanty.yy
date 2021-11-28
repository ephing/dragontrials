%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace{cshanty}
%define api.parser.class {Parser}
%define parse.error verbose
%output "parser.cc"
%token-table

%code requires{
	#include <vector>
	#include "tokens.hpp"
   	#include "ast.hpp"
	namespace cshanty {
		class Scanner;
	}

//The following definition is required when 
// we don't use the %locations directive (which we won't)
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

//End "requires" code
}

%parse-param { cshanty::Scanner &scanner }
%parse-param { std::vector<DeclNode*>** root }
%code{
   // C std code for utility functions
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   // Our code for interoperation between scanner/parser
   #include "scanner.hpp"
   #include "tokens.hpp"

  //Request tokens from our scanner member, not 
  // from a global function
  #undef yylex
  #define yylex scanner.yylex
}

/*
The %union directive is a way to specify the 
set of possible types that might be used as
translation attributes on a symbol.
For this project, only terminals have types (we'll
have translation attributes for non-terminals in the next
project)
*/
%union {
   cshanty::Token* lexeme;
   cshanty::Token* transToken;
   cshanty::IDToken*                       transIDToken;
   cshanty::IntLitToken*				   transIntToken;
   cshanty::StrToken*					   transStrToken;
   std::vector<cshanty::DeclNode *> *      transDeclList;
   cshanty::DeclNode *                     transDecl;
   cshanty::VarDeclNode *                  transVarDecl;
   std::vector<cshanty::VarDeclNode *> *   transVDeclList;
   cshanty::TypeNode *                     transType;
   cshanty::IDNode *                       transID;
   cshanty::LValNode *                     transLVal;
   cshanty::ExpNode *					   transExp;
   std::vector<cshanty::ExpNode *> *	   transExpList;
   cshanty::StmtNode *					   transStmt;
   std::vector<cshanty::StmtNode *> *	   transStmtList;
   cshanty::FormalDeclNode *			   transFormal;
   std::vector<FormalDeclNode *> *		   transFormalList;
   cshanty::AssignExpNode *				   transAssignExp;
   cshanty::CallExpNode *				   transCallExp;
}

%define parse.assert

/* Terminals 
 *  No need to touch these, but do note the translation type
 *  of each node. Most are just "transToken", which is defined in
 *  the %union above to mean that the token translation is an instance
 *  of cshanty::Token *, and thus has no fields (other than line and column).
 *  Some terminals, like ID, are "transIDToken", meaning the translation
 *  also has a name field. 
*/
%token                   END	   0 "end file"
%token	<transToken>     AND
%token	<transToken>     ASSIGN
%token	<transToken>     BOOL
%token	<transToken>     CLOSE
%token	<transToken>     COMMA
%token	<transToken>     DEC
%token	<transToken>     DIVIDE
%token	<transToken>     ELSE
%token	<transToken>     EQUALS
%token	<transToken>     FALSE
%token	<transToken>     GREATER
%token	<transToken>     GREATEREQ
%token	<transIDToken>   ID
%token	<transToken>     IF
%token	<transToken>     INC
%token	<transToken>     INT
%token	<transIntToken>  INTLITERAL
%token	<transToken>     LBRACE
%token	<transToken>     LESS
%token	<transToken>     LESSEQ
%token	<transToken>     LPAREN
%token	<transToken>     MINUS
%token	<transToken>     NOT
%token	<transToken>     NOTEQUALS
%token	<transToken>     OPEN
%token	<transToken>     OR
%token	<transToken>     PLUS
%token	<transToken>     RBRACE
%token	<transToken>     RECEIVE
%token	<transToken>     RECORD
%token	<transToken>     REPORT
%token	<transToken>     RETURN
%token	<transToken>     RPAREN
%token	<transToken>     SEMICOL
%token	<transToken>     STRING
%token	<transStrToken>  STRLITERAL
%token	<transToken>     TIMES
%token	<transToken>     TRUE
%token	<transToken>     VOID
%token	<transToken>     WHILE

/* Nonterminals
*  The specifier in angle brackets
*  indicates the type of the translation attribute using
*  the names defined in the %union directive above
*  TODO: You will need to add more attributes for other nonterminals
*  to this list as you add productions to the grammar
*  below (along with indicating the appropriate translation
*  attribute type).
*/
/*    (attribute type)    (nonterminal)    */
%type <transDeclList>   globals
%type <transDecl>       decl
%type <transVarDecl>    varDecl
%type <transType>       type
%type <transLVal>       lval
%type <transID>         id
%type <transVDeclList>  varDeclList
%type <transExp>		exp
%type <transExp>		term
%type <transAssignExp>	assignExp
%type <transCallExp>	callExp
%type <transExpList>	actualsList
%type <transStmt>		stmt
%type <transStmtList>	stmtList
%type <transFormal>		formalDecl
%type <transFormalList>	formals
%type <transDecl>		fnDecl
%type <transDecl>		recordDecl


%right ASSIGN
%left OR
%left AND
%nonassoc LESS GREATER LESSEQ GREATEREQ EQUALS NOTEQUALS
%left MINUS PLUS
%left TIMES DIVIDE
%left NOT 

%%

globals 	: globals decl END
	  	  { 
	  	  $$ = $1; 
	  	  DeclNode * declNode = $2;
		  $$->push_back(declNode);
	  	  }
		| /* epsilon */
		  {
		  $$ = *root;
		  }

decl 		: fnDecl 
		  { 
			$$ = $1;
		  }
		| recordDecl { $$ = $1; }
		| stmt { $$ = new GlobalStmtNode($1->pos(),$1);	}
		| exp { $$ = new GlobalExpNode($1->pos(), $1); }


recordDecl	: RECORD id OPEN varDeclList CLOSE { 
			$$ = new RecordTypeDeclNode(new Position($1->pos(),$5->pos()),$2,$4); 
			delete $1;
			delete $3;
			delete $5;
		}

varDecl 	: type id SEMICOL 
		  { 
		    Position * p = new Position($1->pos(), $2->pos());
		    $$ = new VarDeclNode(p, $1, $2);
			delete $3;
		  }

varDeclList     : varDecl {
					$$ = new std::vector<VarDeclNode*>();
					$$->push_back($1);
 				}
		| varDeclList varDecl { 
					$$ = $1;
					VarDeclNode* decl = $2;
					$$->push_back(decl);
				}

type 		: INT { $$ = new IntTypeNode(new Position($1->pos(),$1->pos())); delete $1; }
		| BOOL { $$ = new BoolTypeNode(new Position($1->pos(),$1->pos())); delete $1; }
		| id { $$ = new RecordTypeNode($1->pos(),$1); }
		| STRING { $$ = new StringTypeNode(new Position($1->pos(),$1->pos())); delete $1; }
		| VOID { $$ = new VoidTypeNode(new Position($1->pos(),$1->pos())); delete $1; }

fnDecl 		: type id LPAREN RPAREN OPEN stmtList CLOSE { 
			std::vector<FormalDeclNode *> * f = new std::vector<FormalDeclNode *>();
			$$ = new FnDeclNode(new Position($1->pos(),$7->pos()),$1,$2,f,$6);
			delete $3;
			delete $4;
			delete $5;
			delete $7;
		}
		| type id LPAREN formals RPAREN OPEN stmtList CLOSE { 
			$$ = new FnDeclNode(new Position($1->pos(),$8->pos()),$1,$2,$4,$7);
			delete $3;
			delete $5;
			delete $6;
			delete $8;
		}

formals 	: formalDecl { 
			$$ = new std::vector<FormalDeclNode*>(); 
			FormalDeclNode* f = $1;
			$$->push_back(f);
		}
		| formals COMMA formalDecl { 
			$$ = $1;
			FormalDeclNode* f = $3;
			$$->push_back(f);
			delete $2;
		}

formalDecl 	: type id { $$ = new FormalDeclNode(new Position($1->pos(), $2->pos()), $1, $2); }

stmtList 	: /* epsilon */ {$$ = new std::vector<StmtNode*>(); }
		| stmtList stmt { 
			$$ = $1;
			StmtNode* s = $2;
			$$->push_back(s);
		}

stmt		: varDecl { $$ = $1; }
		| assignExp SEMICOL { 
			$$ = new AssignStmtNode(new Position($1->pos(),$2->pos()),$1); delete $2; }
		| lval DEC SEMICOL { 
			$$ = new PostDecStmtNode(new Position($1->pos(),$3->pos()),$1); 
			delete $2;
			delete $3;
		}
		| lval INC SEMICOL { 
			$$ = new PostIncStmtNode(new Position($1->pos(),$3->pos()),$1); 
			delete $2;
			delete $3;
		}
		| RECEIVE lval SEMICOL { 
			$$ = new ReceiveStmtNode(new Position($1->pos(),$3->pos()), $2); 
			delete $1;
			delete $3;
		}
		| REPORT exp SEMICOL { 
			$$ = new ReportStmtNode(new Position($1->pos(),$3->pos()), $2); 
			delete $1;
			delete $3;
		}
		| IF LPAREN exp RPAREN OPEN stmtList CLOSE { 
			$$ = new IfStmtNode(new Position($1->pos(),$7->pos()),$3, $6); 
			delete $1; delete $2; delete $4; delete $5; delete $7;
		}
		| IF LPAREN exp RPAREN OPEN stmtList CLOSE ELSE OPEN stmtList CLOSE { 
			$$ = new IfElseStmtNode(new Position($1->pos(),$11->pos()),$3,$6,$10); 
			delete $1; delete $2; delete $4; delete $5; delete $7; delete $8; delete $9; delete $11;
		}
		| WHILE LPAREN exp RPAREN OPEN stmtList CLOSE { 
			$$ = new WhileStmtNode(new Position($1->pos(),$7->pos()),$3, $6); 
			delete $1; delete $2; delete $4; delete $5; delete $7;
		}
		| RETURN exp SEMICOL { 
			$$ = new ReturnStmtNode(new Position($1->pos(),$3->pos()),$2); 
			delete $1;
			delete $3;
		}
		| RETURN SEMICOL { 
			$$ = new ReturnStmtNode(new Position($1->pos(),$2->pos()),nullptr);
			delete $1; delete $2;
		}
		| callExp SEMICOL { $$ = new CallStmtNode(new Position($1->pos(),$2->pos()),$1); delete $2; }

exp		: assignExp { $$ = $1; } 
		| exp MINUS exp { $$ = new MinusNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp PLUS exp { $$ = new PlusNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp TIMES exp { $$ = new TimesNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp DIVIDE exp { $$ = new DivideNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp AND exp { $$ = new AndNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp OR exp { $$ = new OrNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp EQUALS exp { $$ = new EqualsNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp NOTEQUALS exp { $$ = new NotEqualsNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp GREATER exp { $$ = new GreaterNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp GREATEREQ exp { $$ = new GreaterEqNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp LESS exp { $$ = new LessNode(new Position($1->pos(), $3->pos()),$1,$3); delete $2; }
		| exp LESSEQ exp { $$ = new LessEqNode(new Position($1->pos(), $3->pos()),$1,$3); }
		| NOT exp { $$ = new NotNode(new Position($1->pos(), $2->pos()),$2); delete $1; }
		| MINUS term { $$ = new NegNode(new Position($1->pos(), $2->pos()),$2); delete $1; }
		| term { $$ = $1; }

assignExp	: lval ASSIGN exp { $$ = new AssignExpNode(new Position($1->pos(), $3->pos()), $1, $3); delete $2; }

callExp		: id LPAREN RPAREN { 
				std::vector<ExpNode*>* a = new std::vector<ExpNode*>();
				$$ = new CallExpNode(new Position($1->pos(),$3->pos()),$1,a);
				delete $2;
				delete $3; 
			}
		| id LPAREN actualsList RPAREN { 
			$$ = new CallExpNode(new Position($1->pos(),$4->pos()),$1,$3); 
			delete $2;
			delete $4;
		}

actualsList	: exp { 
			$$ = new std::vector<ExpNode*>();
			ExpNode* e = $1;
			$$->push_back(e); 
		}
		| actualsList COMMA exp { 
			$$ = $1;
			ExpNode* e = $3;
			$$->push_back(e);
			delete $2;
		}

term 		: lval { $$ = $1; }
		| INTLITERAL { 
			$$ = new IntLitNode(new Position($1->pos(),$1->pos()), $1->num()); 
			delete $1;
		}
		| STRLITERAL { $$ = new StrLitNode(new Position($1->pos(),$1->pos()), $1->str()); delete $1; }
		| TRUE { $$ = new TrueNode(new Position($1->pos(),$1->pos())); delete $1; }
		| FALSE { $$ = new FalseNode(new Position($1->pos(),$1->pos())); delete $1; }
		| LPAREN exp RPAREN { $$ = $2; delete $1; delete $3; }
		| callExp { $$ = $1; }

lval		: id { $$ = $1; }
		| id LBRACE id RBRACE { 
				Position* p = new Position($1->pos(), $4->pos());
				$$ = new IndexNode(p, $1, $3); 
				delete $2; delete $4;
			}

id		: ID
		  {
		  Position * pos = new Position($1->pos(),$1->pos());
		  $$ = new IDNode(pos, $1->value()); 
		  delete $1;
		  }

	
%%

void cshanty::Parser::error(const std::string& msg){
	std::cout << msg << std::endl;
	std::cerr << "syntax error" << std::endl;
}
