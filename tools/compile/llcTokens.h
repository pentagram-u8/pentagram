/*
 *	llcTokens.h - Tokens for llc language
 *
 *  Copyright (C) 2002 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef LLCTOKENS_H
#define LLCTOKENS_H

enum LLCToken
{
	LLC_NONE=0x100, 
	// constants
	LLC_INTEGER,
	// vartypes
	LLC_WORD, LLC_DWORD, LLC_STRING, LLC_CSTRING, LLC_LIST, LLC_POINTER, LLC_CONST, LLC_REALLY,
	// function bits
	LLC_OPEN_BRACE, LLC_CLOSE_BRACE, LLC_OPEN_PAREN, LLC_CLOSE_PAREN, LLC_DOT,
	LLC_COMMA, LLC_SEMICOLON, LLC_COLON, LLC_DCOLON,
	LLC_OPEN_REFERENCE, LLC_CLOSE_REFERENCE,
	LLC_RETURN, LLC_SUSPEND,
	LLC_ROUTINE, LLC_PROCESS,
	LLC_EXCLUDE,
	LLC_OVERLOAD,
	LLC_RESULT,
	// math shift
	LLC_LEFT_SHIFT, LLC_RIGHT_SHIFT, LLC_LEFT_SHIFT_EQUALS, LLC_RIGHT_SHIFT_EQUALS,
	// math arith
	LLC_MUL, LLC_DIV, LLC_ADD, LLC_SUB, LLC_MOD, LLC_EXCLUSIVE_ADD, LLC_MUL_EQ, 
	LLC_DIV_EQ, LLC_ADD_EQ,	LLC_SUB_EQ, LLC_EXCLUSIVE_ADD_EQ, LLC_INC, LLC_DEC, 
	LLC_EQUALS,
	// math compare
	LLC_AND, LLC_OR, LLC_LT, LLC_LE, LLC_GT, LLC_GE, LLC_NE, LLC_NOT, LLC_BIT_AND, 
	LLC_BIT_OR, LLC_BIT_NOT, LLC_BIT_AND_EQ, LLC_BIT_OR_EQ,
	LLC_TRUE, LLC_FALSE,
	// list
	LLC_LIST_OPEN, LLC_LIST_CLOSE, LLC_OPEN_INDEX, LLC_CLOSE_INDEX, LLC_IN,
	// conditionals
	LLC_HASHIF, LLC_IF, LLC_ELSE, LLC_WHILE, LLC_FOREACH, LLC_RECURSIVE,
	LLC_CASE, LLC_DEFAULT, LLC_BREAK, 
	LLC_CONTINUE,
	// overloadable functions
	LLC_OF_LOOK, LLC_OF_USE,
	// misc
	LLC_CLASS, LLC_INHERITS, LLC_IS_A, LLC_ARE,
	LLC_ADDRESS_OF,
	// idenfitiers
	LLC_FUNC_IDENT, LLC_IDENT,
	// debugging
	LLC_XWARN, LLC_XFAIL, LLC_XPASS,
	// eof
	LLC_EOF,
	// error
	LLC_ERROR
};


#endif
