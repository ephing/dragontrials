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
	#include <list>
	#include "tokens.hpp"
	#include "ast.hpp"
	namespace cshanty {
		class Scanner;
	}

//The following definition is required when 
// we don't have the %locations directive
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
%parse-param { cshanty::ProgramNode** root }
%code{
   // C std code for utility functions
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   // Our code for interoperation between scanner/parser
   #include "scanner.hpp"
   #include "ast.hpp"
   #include "tokens.hpp"

  //Request tokens from our scanner member, not 
  // from a global function
  #undef yylex
  #define yylex scanner.yylex
}

%union {
   bool                                  transBool;
   cshanty::Token*                         transToken;
   cshanty::Token*                         lexeme;
   cshanty::IDToken*                       transIDToken;
   cshanty::IntLitToken*                   transIntToken;
   cshanty::StrToken*                      transStrToken;
   cshanty::ProgramNode*                   transProgram;
   cshanty::DeclNode *                     transDecl;
   std::list<cshanty::DeclNode *> *        transDeclList;
   cshanty::RecordTypeDeclNode *           transRecordDecl;
   cshanty::VarDeclNode *                  transVarDecl;
   std::list<cshanty::VarDeclNode *> *     transVarDeclList;
   cshanty::FormalDeclNode *               transFormal;
   std::list<cshanty::FormalDeclNode *> *  transFormalList;
   cshanty::TypeNode *                     transType;
   cshanty::LValNode *                     transLVal;
   cshanty::IDNode *                       transID;
   cshanty::FnDeclNode *                   transFn;
   std::list<cshanty::VarDeclNode *> *     transVarDecls;
   std::list<cshanty::StmtNode *> *        transStmts;
   cshanty::StmtNode *                     transStmt;
   cshanty::ExpNode *                      transExp;
   cshanty::AssignExpNode *                transAssignExp;
   cshanty::CallExpNode *                  transCallExp;
   std::list<cshanty::ExpNode *> *         transActuals;
}

%define parse.assert


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

%type <transProgram>     program
%type <transDeclList>    globals
%type <transDecl>        decl
%type <transRecordDecl>  recordDecl
%type <transVarDecl>     varDecl
%type <transVarDeclList> varDeclList
%type <transType>        type
%type <transLVal>        lval
%type <transID>          id
%type <transFn>          fnDecl
%type <transFormal>      formalDecl
%type <transFormalList>  formals
%type <transStmts>       stmtList
%type <transStmt>        stmt
%type <transAssignExp>   assignExp
%type <transExp>         exp
%type <transExp>         term
%type <transCallExp>     callExp
%type <transActuals>     actualsList

/* NOTE: Make sure to add precedence and associativity 
 * declarations
 */
%right ASSIGN
%left OR
%left AND
%nonassoc LESS GREATER LESSEQ GREATEREQ EQUALS NOTEQUALS
%left MINUS PLUS
%left TIMES DIVIDE
%left NOT 

%%

program 	: globals
		  {
		  $$ = new ProgramNode($1);
		  *root = $$;
		  }

globals 	: globals decl 
	  	  { 
	  	  $$ = $1; 
	  	  DeclNode * declNode = $2;
		  $$->push_back(declNode);
	  	  }
		| /* epsilon */
		  {
		  $$ = new std::list<DeclNode * >();
		  }

decl 		: varDecl
		  { $$ = $1; }
		| fnDecl 
		  { $$ = $1; }
		| recordDecl
		  { $$ = $1; }

recordDecl	: RECORD id OPEN varDeclList CLOSE
		  {
		  Position * p = new Position($1->pos(), $5->pos());
		  $$ = new RecordTypeDeclNode(p, $2, $4);
		  }

varDecl 	: type id SEMICOL
		  {
		  Position * p = new Position($1->pos(), $2->pos());
		  $$ = new VarDeclNode(p, $1, $2);
		  }

varDeclList     : varDecl
		  {
		  $$ = new std::list<VarDeclNode *>();
		  $$->push_back($1);
		  }
		| varDeclList varDecl
		  {
		  $$ = $1;
		  $$->push_back($2);
		  }

type 		: INT
	  	  { 
		  $$ = new IntTypeNode($1->pos());
		  }
		| BOOL
		  {
		  $$ = new BoolTypeNode($1->pos());
		  }
		| id
		  {
		  $$ = new RecordTypeNode($1->pos(), $1);
		  }
		| STRING
		  {
		  $$ = new StringTypeNode($1->pos());
		  }
		| VOID
		  {
		  $$ = new VoidTypeNode($1->pos());
		  }

fnDecl 		: type id LPAREN RPAREN OPEN stmtList CLOSE
		  {
		  Position * pos = new Position($1->pos(), $7->pos());
		  std::list<FormalDeclNode *> * f = new std::list<FormalDeclNode *>();
		  $$ = new FnDeclNode(pos, $1, $2, f, $6);
		  }
		| type id LPAREN formals RPAREN OPEN stmtList CLOSE
		  {
		  Position * pos = new Position($1->pos(), $8->pos());
		  $$ = new FnDeclNode(pos, $1, $2, $4, $7);
		  }

formals 	: formalDecl
		  {
		  $$ = new std::list<FormalDeclNode *>();
		  $$->push_back($1);
		  }
		| formals COMMA formalDecl
		  {
		  $$ = $1;
		  $$->push_back($3);
		  }

 /*
  declList	: varDecl
		  {
		  $$ = new std::list<DeclNode *>();
		  $$->push_back($1);
		  }
		| varDecl COMMA declList 
		  {
		  $$ = $3;
		  $$->push_front($1);
		  }
  */

formalDecl 	: type id
		  {
		  Position * pos = new Position($1->pos(), $2->pos());
		  $$ = new FormalDeclNode(pos, $1, $2);
		  }

stmtList 	: /* epsilon */
	   	  {
		  $$ = new std::list<StmtNode *>();
		  //$$->push_back($1);
	   	  }
		| stmtList stmt
	  	  {
		  $$ = $1;
		  $$->push_back($2);
	  	  }

stmt		: varDecl
		  {
		  Position * p = $1->pos();
		  $$ = new VarDeclNode(p, $1->getTypeNode(), $1->ID());
		  }
		| assignExp SEMICOL
		  {
		  Position * p = new Position($1->pos(), $2->pos());
		  $$ = new AssignStmtNode(p, $1); 
		  }
		| lval DEC SEMICOL
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new PostDecStmtNode(p, $1);
		  }
		| lval INC SEMICOL
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new PostIncStmtNode(p, $1);
		  }
		| RECEIVE lval SEMICOL
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new ReceiveStmtNode(p, $2);
		  }
		| REPORT exp SEMICOL
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new ReportStmtNode(p, $2);
		  }
		| IF LPAREN exp RPAREN OPEN stmtList CLOSE
		  {
		  Position * p = new Position($1->pos(), $7->pos());
		  $$ = new IfStmtNode(p, $3, $6);
		  }
		| IF LPAREN exp RPAREN OPEN stmtList CLOSE ELSE OPEN stmtList CLOSE
		  {
		  Position * p = new Position($1->pos(), $11->pos());
		  $$ = new IfElseStmtNode(p, $3, $6, $10);
		  }
		| WHILE LPAREN exp RPAREN OPEN stmtList CLOSE
		  {
		  Position * p = new Position($1->pos(), $7->pos());
		  $$ = new WhileStmtNode(p, $3, $6);
		  }
		| RETURN exp SEMICOL
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new ReturnStmtNode(p, $2);
		  }
		| RETURN SEMICOL
		  {
		  Position * p = new Position($1->pos(), $2->pos());
		  $$ = new ReturnStmtNode(p, nullptr);
		  }
		| callExp SEMICOL
		  { 
		  Position * p = new Position($1->pos(), $2->pos());
		  $$ = new CallStmtNode(p, $1); 
		  }

exp		: assignExp 
		  { $$ = $1; } 
		| exp MINUS exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new MinusNode(p, $1, $3);
		  }
		| exp PLUS exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new PlusNode(p, $1, $3);
		  }
		| exp TIMES exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new TimesNode(p, $1, $3);
		  }
		| exp DIVIDE exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new DivideNode(p, $1, $3);
		  }
		| exp AND exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new AndNode(p, $1, $3);
		  }
		| exp OR exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new OrNode(p, $1, $3);
		  }
		| exp EQUALS exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new EqualsNode(p, $1, $3);
		  }
		| exp NOTEQUALS exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new NotEqualsNode(p, $1, $3);
		  }
		| exp GREATER exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new GreaterNode(p, $1, $3);
		  }
		| exp GREATEREQ exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new GreaterEqNode(p, $1, $3);
		  }
		| exp LESS exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new LessNode(p, $1, $3);
		  }
		| exp LESSEQ exp
	  	  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new LessEqNode(p, $1, $3);
		  }
		| NOT exp
	  	  {
		  Position * p = new Position($1->pos(), $2->pos());
		  $$ = new NotNode(p, $2);
		  }
		| MINUS term
	  	  {
		  Position * p = new Position($1->pos(), $2->pos());
		  $$ = new NegNode(p, $2);
		  }
		| term 
	  	  { $$ = $1; }

assignExp	: lval ASSIGN exp
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  $$ = new AssignExpNode(p, $1, $3);
		  }

callExp		: id LPAREN RPAREN
		  {
		  Position * p = new Position($1->pos(), $3->pos());
		  std::list<ExpNode *> * noargs =
		    new std::list<ExpNode *>();
		  $$ = new CallExpNode(p, $1, noargs);
		  }
		| id LPAREN actualsList RPAREN
		  {
		  Position * p = new Position($1->pos(), $4->pos());
		  $$ = new CallExpNode(p, $1, $3);
		  }

actualsList	: exp
		  {
		  std::list<ExpNode *> * list =
		    new std::list<ExpNode *>();
		  list->push_back($1);
		  $$ = list;
		  }
		| actualsList COMMA exp
		  {
		  $$ = $1;
		  $$->push_back($3);
		  }

term 		: lval
		  { $$ = $1; }
		| INTLITERAL 
		  { $$ = new IntLitNode($1->pos(), $1->num()); }
		| STRLITERAL 
		  { $$ = new StrLitNode($1->pos(), $1->str()); }
		| TRUE
		  { $$ = new TrueNode($1->pos()); }
		| FALSE
		  { $$ = new FalseNode($1->pos()); }
		| LPAREN exp RPAREN
		  { $$ = $2; }
		| callExp
		  {
		  $$ = $1;
		  }

lval		: id
		  {
		  $$ = $1;
		  }
		| id LBRACE id RBRACE
		  {
		  Position * pos = new Position($1->pos(), $4->pos());
		  $$ = new IndexNode(pos, $1, $3);
		  }

id		: ID
		  {
		  Position * pos = $1->pos();
		  $$ = new IDNode(pos, $1->value()); 
		  }
	
%%

void cshanty::Parser::error(const std::string& msg){
	std::cout << msg << std::endl;
	std::cerr << "syntax error" << std::endl;
}
