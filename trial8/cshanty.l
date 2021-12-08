%{
#include <string>
#include <limits.h>

/* Get our custom yyFlexScanner subclass */
#include "scanner.hpp"
#undef YY_DECL
#define YY_DECL int cshanty::Scanner::yylex(cshanty::Parser::semantic_type * const lval)

using TokenKind = cshanty::Parser::token;

/* define yyterminate as returning an EOF token (instead of NULL) */
#define yyterminate() return ( TokenKind::END )

/* exclude unistd.h for Visual Studio compatibility. */
#define YY_NO_UNISTD_H

#define EXIT_ON_ERR 0


%}

%option nodefault
%option yyclass="cshanty::Scanner"
%option noyywrap
%option c++

DIGIT [0-9]
WHITESPACE [\040\t]
LETTER [a-zA-Z]
ESCAPEE [nt'"\\]

 /* */ 
BADESC [\\][^nt"\\\n]?
STRELT ([\\][nt"\\])|([^\\\n"])

%%
%{
	this->yylval = lval;
%}

int    		      { return makeBareToken(TokenKind::INT); }
bool 		      { return makeBareToken(TokenKind::BOOL); }
record	      	      { return makeBareToken(TokenKind::RECORD); }
string	      	      { return makeBareToken(TokenKind::STRING); }
void 		      { return makeBareToken(TokenKind::VOID); }
if  		      { return makeBareToken(TokenKind::IF); }
else		      { return makeBareToken(TokenKind::ELSE); }
while		      { return makeBareToken(TokenKind::WHILE); }
return		    { return makeBareToken(TokenKind::RETURN); }
"we'll take our leave and go" { return makeBareToken(TokenKind::RETURN); }
false  		    { return makeBareToken(TokenKind::FALSE); }
nay  		    { return makeBareToken(TokenKind::FALSE); }
true 		    { return makeBareToken(TokenKind::TRUE); }
aye 		    { return makeBareToken(TokenKind::TRUE); }
report  	    { return makeBareToken(TokenKind::REPORT);}
receive	  	    { return makeBareToken(TokenKind::RECEIVE); }
"["		        { return makeBareToken(TokenKind::LBRACE); }
"]"		        { return makeBareToken(TokenKind::RBRACE); }
"{"		        { return makeBareToken(TokenKind::OPEN); }
"ahoy"		        { return makeBareToken(TokenKind::OPEN); }
"}"		        { return makeBareToken(TokenKind::CLOSE); }
"shove off"	        { return makeBareToken(TokenKind::CLOSE); }
"("		        { return makeBareToken(TokenKind::LPAREN); }
")"		        { return makeBareToken(TokenKind::RPAREN); }
";"		        { return makeBareToken(TokenKind::SEMICOL); }
"heave and go"	        { return makeBareToken(TokenKind::SEMICOL); }
"roll and go"	        { return makeBareToken(TokenKind::SEMICOL); }
","		        { return makeBareToken(TokenKind::COMMA); }
"++"          { return makeBareToken(TokenKind::INC); }
"+"           { return makeBareToken(TokenKind::PLUS); }
"plus"           { return makeBareToken(TokenKind::PLUS); }
"--"          { return makeBareToken(TokenKind::DEC); }
"-"		        { return makeBareToken(TokenKind::MINUS); }
"minus"		        { return makeBareToken(TokenKind::MINUS); }
"*"		        { return makeBareToken(TokenKind::TIMES); }
"times"		        { return makeBareToken(TokenKind::TIMES); }
"/"		        { return makeBareToken(TokenKind::DIVIDE); }
"divide"	        { return makeBareToken(TokenKind::DIVIDE); }
"!"	 	        { return makeBareToken(TokenKind::NOT); }
"&&"          { return makeBareToken(TokenKind::AND); }
"and"          { return makeBareToken(TokenKind::AND); }
"||"          { return makeBareToken(TokenKind::OR); }
"or"          { return makeBareToken(TokenKind::OR); }
"=="          { return makeBareToken(TokenKind::EQUALS); }
"equals"      { return makeBareToken(TokenKind::EQUALS); }
"!="          { return makeBareToken(TokenKind::NOTEQUALS); }
"<"	      { return makeBareToken(TokenKind::LESS); }
"<="          { return makeBareToken(TokenKind::LESSEQ); }
">"		        { return makeBareToken(TokenKind::GREATER); }
">="          { return makeBareToken(TokenKind::GREATEREQ); }
"="		        { return makeBareToken(TokenKind::ASSIGN); }
"gets"		        { return makeBareToken(TokenKind::ASSIGN); }
({LETTER}|_)({LETTER}|{DIGIT}|_)* { 
			  Position * pos = new Position(lineNum, colNum,
				lineNum, colNum + yyleng);
		            yylval->transToken = 
		            new IDToken(pos, yytext);
		            colNum += yyleng;
		            return TokenKind::ID; }

{DIGIT}+	    { double asDouble = std::stod(yytext);
			          int intVal = atoi(yytext);
			          bool overflow = false;
			          if (asDouble > INT_MAX){ overflow = true; }

								std::string str = yytext;
				  			std::string suffix = "";
				  			for(int i = 0 ; i < str.length(); i++){
									if (str[i] != '0'){
										suffix = str.substr(i, std::string::npos);
										break;
									}
				  			}
			          if (suffix.length() > 10){ overflow = true; }

			          if (overflow){
										Position pos(lineNum,colNum,lineNum,colNum+yyleng);
				            errIntOverflow(&pos);
				            intVal = INT_MAX;
			          }
				  			Position * pos = new Position(lineNum, colNum,
									lineNum, colNum + yyleng);
			          yylval->transToken = 
			              new IntLitToken(pos, intVal);
			          colNum += yyleng;
			          return TokenKind::INTLITERAL; }

\"{STRELT}*\" {
			Position * pos;
			pos = new Position(lineNum, colNum, lineNum, colNum + yyleng);
   		          yylval->transToken = 
                    new StrToken(pos, yytext);
		            this->colNum += yyleng;
		            return TokenKind::STRLITERAL; }

\"{STRELT}* {
			Position pos(lineNum, colNum, lineNum, colNum + yyleng);
		            errStrUnterm(&pos);
		            colNum += yyleng; /*Upcoming \n resets lineNum */
			    #if EXIT_ON_ERR
			    exit(1);
			    #endif
		            }

["]([^"\n]*{BADESC}[^"\n]*)+(\\["])? {
                // Bad, unterm string lit
		Position pos(lineNum,colNum,lineNum,colNum+yyleng);
		errStrEscAndUnterm(&pos);
                colNum += yyleng;
        }

["]([^"\n]*{BADESC}[^"\n]*)+["] {
                // Bad string lit
		Position pos(lineNum,colNum,lineNum,colNum+yyleng);
		errStrEsc(&pos);
                colNum += yyleng;
        }

\n|(\r\n)     { lineNum++; colNum = 1; }


[ \t]+	      { colNum += yyleng; }

([/][/])[^\n]*	  { /* Comment. No token, but update the 
                   char num in the very specific case of 
                   getting the correct EOF position */ 
		   colNum += yyleng;
		  }

.		          { 
				
				Position pos(lineNum,colNum,lineNum,colNum+yyleng);
				errIllegal(&pos, yytext);
			    #if EXIT_ON_ERR
			    exit(1);
			    #endif
		            this->colNum += yyleng; }
%%
