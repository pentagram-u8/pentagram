%{
#include "llcTokens.h"

%}

	// just to 'simply' things
%option never-interactive
%option noyywrap
%option c++
%option 8bit
%option ansi-definitions
%option ansi-prototypes
%option warn

	// spits out a short performance warning
%option perf-report

	// to make things easier I hope. *grin*
%option yylineno

	// only for debugging
%option debug
%option nodefault

	// enable start state stacks (say *that* three times fast!)
%option stack

%option prefix="llc"

%x CCOMMENT HASH_EXPECT

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

{LineComment}		{ }

"/*"				{ BEGIN(CCOMMENT);		}

<CCOMMENT>{
[^*\n]*				{ }
[^*\n]*\n			{ }
"*"+[^*/\n]*		{ }
"*"+[^*/\n]*\n		{ }
"*"+"/"				{ BEGIN(INITIAL); }
}

 /***************************************************************************
	Constants
  ***************************************************************************/

{BinaryConst}		{ return LLC_INTEGER;	}

{DecimalConst}		{ return LLC_INTEGER;	}

{HexConst}			{ return LLC_INTEGER;	}

{StringConst}		{ return LLC_STRING;	}

{CStringConst}		{ return LLC_CSTRING;	}

 /***************************************************************************
	Whitespace
  ***************************************************************************/

{WS}				{ }

 /***************************************************************************
	Variable types
  ***************************************************************************/

"Bool"				{ return LLC_WORD;		}
"Boolean"			{ return LLC_WORD;		}

"Byte"				{ return LLC_WORD;		}

"Word"				{ return LLC_WORD;		}
"DWord"				{ return LLC_DWORD;		}

 /* Unsure if the default 'number' should be WORD or DWORD, shall find out soon enough
    we shall... */

"Int"				{ return LLC_WORD;		}
"Integer"			{ return LLC_WORD;		}
"Number"			{ return LLC_WORD;		}

"String"			{ return LLC_STRING;	}

 /* Postfix modifiers... */

"List"				{ return LLC_LIST;		}
"Pointer"			{ return LLC_POINTER;	}

 /* Prefix modifiers... */

"Const"				{ return LLC_CONST;		}
"Constant"			{ return LLC_CONST;		}
"Really"			{ return LLC_REALLY;	}

 /***************************************************************************
	Function bits
  ***************************************************************************/

\{ 				{ return LLC_OPEN_BRACE;		}
\} 				{ return LLC_CLOSE_BRACE;		}
\( 				{ return LLC_OPEN_PAREN;		}
\) 				{ return LLC_CLOSE_PAREN;		}
"." 			{ return LLC_DOT;				}
"," 			{ return LLC_COMMA;				}
";" 			{ return LLC_SEMICOLON;			}
":" 			{ return LLC_COLON;				}
"::" 			{ return LLC_DCOLON;			}
"[["			{ return LLC_OPEN_REFERENCE;	}
"]]"			{ return LLC_CLOSE_REFERENCE;	}

"return"		{ return LLC_RETURN;			}
"suspend"		{ return LLC_SUSPEND;			}

"routine"		{ return LLC_ROUTINE;			}
"process"		{ return LLC_PROCESS;			}

"exclude"		{ return LLC_EXCLUDE;			}

"overloadable"	{ return LLC_OVERLOAD;			}
"overload"		{ return LLC_OVERLOAD;			}

"result"		{ return LLC_RESULT;			}

 /***************************************************************************
	Math - Shift
  ***************************************************************************/

"<<<"			{ return LLC_LEFT_SHIFT;			}
">>>"			{ return LLC_RIGHT_SHIFT;			}
"<<<="			{ return LLC_LEFT_SHIFT_EQUALS;		}
">>>="			{ return LLC_RIGHT_SHIFT_EQUALS;	}

 /***************************************************************************
	Math - Arith
  ***************************************************************************/

"*"			{ return LLC_MUL;				}
"/"			{ return LLC_DIV;				}
"+"			{ return LLC_ADD;				}
"-"			{ return LLC_SUB;				}
"mod"		{ return LLC_MOD;				}
"modulo"	{ return LLC_MOD;				}
"^"			{ return LLC_EXCLUSIVE_ADD;		}

"*="		{ return LLC_MUL_EQ;			}
"/="		{ return LLC_DIV_EQ;			}
"+="		{ return LLC_ADD_EQ;			}
"-="		{ return LLC_SUB_EQ;			}
"^="		{ return LLC_EXCLUSIVE_ADD_EQ;	}
"++"		{ return LLC_INC;				}
"--"		{ return LLC_DEC;				}

"="			{ return LLC_EQUALS;			}

 /***************************************************************************
	Math - Compare
  ***************************************************************************/

"&&"		{ return LLC_AND;			}
"and"		{ return LLC_AND;			}
"||"		{ return LLC_OR;			}
"or"		{ return LLC_OR;			}
"!"			{ return LLC_NOT;			}
"not"		{ return LLC_NOT;			}

"<"			{ return LLC_LT;			}
"<="		{ return LLC_LE;			}
">"			{ return LLC_GT;			}
">="		{ return LLC_GE;			}
"<>"		{ return LLC_NE;			}
"!="		{ return LLC_NE;			}

"&"			{ return LLC_BIT_AND;		}
"|"			{ return LLC_BIT_OR;		}
"~"			{ return LLC_BIT_NOT;		}
"&="		{ return LLC_BIT_AND_EQ;	}
"|="		{ return LLC_BIT_OR_EQ;		}

"true"		{ return LLC_TRUE;			}
"false"		{ return LLC_FALSE;			}

 /***************************************************************************
	List
  ***************************************************************************/

"<<"		{ return LLC_LIST_OPEN;		}
">>"		{ return LLC_LIST_CLOSE;	}

"["			{ return LLC_OPEN_INDEX;	}
"]"			{ return LLC_CLOSE_INDEX;	}

"in"		{ return LLC_IN;			}

 /***************************************************************************
	Conditionals
  ***************************************************************************/

"#if"{WS}+/"("?		{ return LLC_HASHIF;	}
"if"{WS}+/"("?		{ return LLC_IF;		}
"else"				{ return LLC_ELSE;		}
"while"{WS}+/"("?	{ return LLC_WHILE;		}
"foreach"			{ return LLC_FOREACH;	}
"recursive"			{ return LLC_RECURSIVE;	}
"case"				{ return LLC_CASE;		}
"default"			{ return LLC_DEFAULT;	}
"break"				{ return LLC_BREAK;		}
"continue"			{ return LLC_CONTINUE;	}

 /***************************************************************************
	Overloadable Function
  ***************************************************************************/

 /* Only a couple defined atm, will work out the rest later once things start
	get put together... */

"look"				{ return LLC_OF_LOOK;	}
"use"				{ return LLC_OF_USE;	}

 /***************************************************************************
	Misc
  ***************************************************************************/

"class"				{ return LLC_CLASS;			}
"inherits"			{ return LLC_INHERITS;		}
"is"{WS}"a"			{ return LLC_IS_A;			}
"are"				{ return LLC_ARE;			}
"address"{WS}"of"	{ return LLC_ADDRESS_OF;	}

 /***************************************************************************
	Identifiers
  ***************************************************************************/

{FunctionIdentifier}	{ return LLC_FUNC_IDENT;	}

{VariableIdentifier}	{ return LLC_IDENT;			}

 /***************************************************************************
 	Debugging/Regression Testing
  ***************************************************************************/

"#expect"		{ BEGIN(HASH_EXPECT);		}

<HASH_EXPECT>{
"XWARN"			{ return LLC_XWARN;			}
"XFAIL"			{ return LLC_XFAIL;			}
"XPASS"			{ return LLC_XPASS;			}
{WS}			{ 							}
\n				{ BEGIN(INITIAL);			}
 /* ignore everything else, since this really isn't a 'user' function lacking
 	random warnings isn't a problem */
.				{ }
}

 /***************************************************************************
	Catch All
  ***************************************************************************/

<<EOF>>	{ return LLC_EOF;	}

.		{ return LLC_ERROR;	}

%%

