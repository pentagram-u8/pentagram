%{
#include <cctype>
//#include "Trace.h"
#include "llcTokens.h"

using std::istream;

%}

 // just to 'simply' things
%option never-interactive
%option noyywrap
%option c++

 // to make things easier I hope. *grin*
%option yylineno

 // only for debugging
%option debug

%option prefix="llc"

%x CCOMMENT

WS [[:space:]]+

LineComment ("//")[^\n]*"\n"
Letter [[:alpha:]_]
VariableIdentifier {Letter}({Letter}|[[:digit:]])*
FunctionIdentifier  ({Letter}({Letter}|[[:digit:]])*)[[:space:]]*\(

BinaryConst ("0b"|"0B")[01]+
DecimalConst "-"?[[:digit:]]+
HexConst ("0x"|"0X")[[:xdigit:]]+
	 // needs to handle 'escape quoting' of '\\\"' in the string
StringConst "\""[^\"]*"\""
CStringConst "\'"[^\']*"\'"

%%

 /***************************************************************************
	Comment handling
  ***************************************************************************/

{LineComment} { }

"/*"                    { BEGIN(CCOMMENT); }
<CCOMMENT>[^*\n]+	{ }
<CCOMMENT>\n		{ }
<CCOMMENT>"*"		{ }
<CCOMMENT>"*/"		{ BEGIN(INITIAL); }
<CCOMMENT>"*/".*\n	{ BEGIN(INITIAL); }

 /***************************************************************************
	Constants
  ***************************************************************************/

{BinaryConst} {
	//FIXME: yylval = new IntegerNode(yytext, 2);
	//trace(Trace::TT_VPARSE, "<- bin_const");
	return LLC_INTEGER;
}

{DecimalConst} {
	//FIXME: yylval = new IntegerNode(yytext, 10);
	//trace(Trace::TT_VPARSE, "<- dec_const");
	return LLC_INTEGER;
}

{HexConst} {
	//FIXME: yylval = new IntegerNode(yytext, 16);
	//trace(Trace::TT_VPARSE, "<- hex_const");
	return LLC_INTEGER;
}

{StringConst} {
	//FIXME: yylval = new StringNode(yytext);
	//trace(Trace::TT_VPARSE, "<- string_const");
	return LLC_STRING;
}

{CStringConst} {
	//FIXME: yylval = new StringNode(yytext);
	//trace(Trace::TT_VPARSE, "<- string_const");
	return LLC_CSTRING;
}

 /***************************************************************************
	Whitespace
  ***************************************************************************/

{WS} { }

 /***************************************************************************
	Variable types
  ***************************************************************************/

"Bool"			{ /*trace(Trace::TT_VPARSE, "<- Bool");*/	  return LLC_WORD;    }
"Boolean"		{ /*trace(Trace::TT_VPARSE, "<- Boolean");*/  return LLC_WORD;    }

"Byte"			{ /*trace(Trace::TT_VPARSE, "<- Byte");*/     return LLC_WORD;    }

"Word"			{ /*trace(Trace::TT_VPARSE, "<- Word");*/     return LLC_WORD;    }
"DWord"			{ /*trace(Trace::TT_VPARSE, "<- DWord");*/    return LLC_DWORD;   }

 /* Unsure if the default 'number' should be WORD or DWORD, shall find out soon enough
    we shall... */

"Int"			{ /*trace(Trace::TT_VPARSE, "<- Int");*/      return LLC_WORD;    }
"Integer"		{ /*trace(Trace::TT_VPARSE, "<- Integer");*/  return LLC_WORD;    }
"Number"		{ /*trace(Trace::TT_VPARSE, "<- Number");*/   return LLC_WORD;    }

"String"		{ /*trace(Trace::TT_VPARSE, "<- String");*/   return LLC_STRING;  }

 /* Postfix modifiers... */

"List"			{ /*trace(Trace::TT_VPARSE, "<- List");*/     return LLC_LIST;    }
"Pointer"		{ /*trace(Trace::TT_VPARSE, "<- Pointer");*/  return LLC_POINTER; }

 /* Prefix modifiers... */

"Const"			{ /*trace(Trace::TT_VPARSE, "<- Const");*/    return LLC_CONST;   }
"Constant"		{ /*trace(Trace::TT_VPARSE, "<- Constant");*/ return LLC_CONST;   }
"Really"		{ /*trace(Trace::TT_VPARSE, "<- Really");*/   return LLC_REALLY;  }

 /***************************************************************************
	Function bits
  ***************************************************************************/

\{ 			{ return LLC_OPEN_BRACE; }
\} 			{ return LLC_CLOSE_BRACE; }
\( 			{ return LLC_OPEN_PAREN; }
\) 			{ return LLC_CLOSE_PAREN; }
"." 			{ return LLC_DOT; }
"," 			{ return LLC_COMMA; }
";" 			{ /*trace(Trace::TT_VPARSE, "<- ';'");*/  return LLC_SEMICOLON; }
":" 			{ /*trace(Trace::TT_VPARSE, "<- ':'");*/  return LLC_COLON; }
"::" 			{ return LLC_DCOLON; }
"[["			{ return LLC_OPEN_REFERENCE; }
"]]"			{ return LLC_CLOSE_REFERENCE; }

"return"		{ return LLC_RETURN; }
"suspend"		{ return LLC_SUSPEND; }

"routine"		{ return LLC_ROUTINE; }
"process"		{ return LLC_PROCESS; }

"exclude"		{ return LLC_EXCLUDE; }

"overloadable"		{ return LLC_OVERLOAD; }
"overload"		{ return LLC_OVERLOAD; }

"result"		{ return LLC_RESULT; }

 /***************************************************************************
	Math - Shift
  ***************************************************************************/

"<<<"			{ return LLC_LEFT_SHIFT; }
">>>"			{ return LLC_RIGHT_SHIFT; }
"<<<="			{ return LLC_LEFT_SHIFT_EQUALS; }
">>>="			{ return LLC_RIGHT_SHIFT_EQUALS; }

 /***************************************************************************
	Math - Arith
  ***************************************************************************/

"*"			{ return LLC_MUL; }
"/"			{ return LLC_DIV; }
"+"			{ return LLC_ADD; }
"-"			{ return LLC_SUB; }
"mod"			{ return LLC_MOD; }
"modulo"		{ return LLC_MOD; }
"^"			{ return LLC_EXCLUSIVE_ADD; }

"*="			{ return LLC_MUL_EQ; }
"/="			{ return LLC_DIV_EQ; }
"+="			{ return LLC_ADD_EQ; }
"-="			{ return LLC_SUB_EQ; }
"^="			{ return LLC_EXCLUSIVE_ADD_EQ; }
"++"			{ return LLC_INC; }
"--"			{ return LLC_DEC; }

"="			{ /*trace(Trace::TT_VPARSE, "<- '='");*/  return LLC_EQUALS; }

 /***************************************************************************
	Math - Compare
  ***************************************************************************/

"&&"			{ return LLC_AND; }
"and"			{ return LLC_AND; }
"||"			{ return LLC_OR; }
"or"			{ return LLC_OR; }
"!"			{ return LLC_NOT; }
"not"			{ return LLC_NOT; }

"<"			{ return LLC_LT; }
"<="			{ return LLC_LE; }
">"			{ return LLC_GT; }
">="			{ return LLC_GE; }
"<>"			{ return LLC_NE; }
"!="			{ return LLC_NE; }

"&"			{ return LLC_BIT_AND; }
"|"			{ return LLC_BIT_OR; }
"~"			{ return LLC_BIT_NOT; }
"&="			{ return LLC_BIT_AND_EQ; }
"|="			{ return LLC_BIT_OR_EQ; }

"true"			{ return LLC_TRUE; }
"false"			{ return LLC_FALSE; }

 /***************************************************************************
	List
  ***************************************************************************/

"<<"			{ return LLC_LIST_OPEN; }
">>"			{ return LLC_LIST_CLOSE; }

"["			{ return LLC_OPEN_INDEX; }
"]"			{ return LLC_CLOSE_INDEX; }

"in"			{ return LLC_IN; }

 /***************************************************************************
	Conditionals
  ***************************************************************************/

"#if"{WS}+/"("?		{ return LLC_HASHIF; }
"if"{WS}+/"("?		{ return LLC_IF; }
"else"			{ return LLC_ELSE; }
"while"{WS}+/"("?	{ return LLC_WHILE; }
"foreach"		{ return LLC_FOREACH; }
"recursive"		{ return LLC_RECURSIVE; }
"case"			{ return LLC_CASE; }
"default"		{ return LLC_DEFAULT; }
"break"			{ return LLC_BREAK; }
"continue"		{ return LLC_CONTINUE; }

 /***************************************************************************
	Overloadable Function
  ***************************************************************************/

 /* Only a couple defined atm, will work out the rest later once things start
	get put together... */

"look"			{ return LLC_OF_LOOK; }
"use"			{ return LLC_OF_USE;  }

 /***************************************************************************
	Misc
  ***************************************************************************/

"class"			{ return LLC_CLASS; }
"is"{WS}"a"		{ return LLC_IS_A;  }
"are"			{ return LLC_ARE;   }
"address"{WS}"of"	{ return LLC_ADDRESS_OF; }

 /***************************************************************************
	Identifiers
  ***************************************************************************/

{FunctionIdentifier} {
	//FIXME: yylval = new StringNode (yytext);
	/*trace(Trace::TT_VPARSE, "<- func_ident");*/
	return LLC_FUNC_IDENT;
}

{VariableIdentifier} {
	//FIXME: yylval = new StringNode (yytext);
	/*trace(Trace::TT_VPARSE, "<- var_ident");*/
	return LLC_IDENT;
}

 /***************************************************************************
	Catch All
  ***************************************************************************/

<<EOF>>				{ return LLC_EOF; };

. {
	//FIXME: yylval = new ErrorNode("Unknown character in usecode file.");
	return LLC_ERROR;
}

%%

const char *testfiles[] =
{
	"current",
	"test/comments.test",
	"test/const.test",
	"test/global.test",
	"test/fail/invalid-comments.test",
	"test/fail/invalid-func-term.test",
	0
};

/*int main(int argc, char *argv[])
{
	if(argc==2)
		if(std::strcmp(argv[1], "list")==0)
		{
			int filenum=-1;
			while(testfiles[++filenum]!=0)
				printf("test %2d: %s\n", filenum, testfiles[filenum]);
			return 0;
		}
		else if(std::strcmp(argv[1], "all")==0)
		{
			yyin=0;
			unsigned int filenum=0;
			
			while(testfiles[filenum]!=0)
			{
				if(yyin!=0) fclose(yyin);
				yyin = fopen(testfiles[filenum], "r");
				
				try {
					yyrestart(yyin); yylineno=0;
					yyparse();
				} catch(ErrorNode *n)
				{
					n->print(stdout);
				}
				filenum++;
			}
			return 0;
		}
		else if(std::isdigit(argv[1][0]))
		{
			unsigned int filenum(strtol(argv[1], 0, 0));
			yyin = fopen(testfiles[filenum], "r");
			try {
				yyrestart(yyin); yylineno=0;
				yyparse();
			} catch(ErrorNode *n)
			{
				n->print(stdout);
			}
			return 0;
		}
		
	// the default. just test current
	yyin = fopen(testfiles[0], "r");
	try {
		yyrestart(yyin); yylineno=0;
		yyparse();
	} catch(ErrorNode *n)
	{
		n->print(stdout);
	}
	return 0;
}*/
