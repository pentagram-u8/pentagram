/*
 *	CompileNodes.h - The defintions of all the compile node types
 *
 *  Copyright (C) 2002-2003 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef COMPILENODES_H
#define COMPILENODES_H

#include "pent_include.h"
#include "llcTokens.h"

#include <string>

// Basic Node Class *********************************************************
class CompileNode
{
	public:
		CompileNode(const uint32 _linenum) : linenum(_linenum) {};

		virtual void print_unk(std::ostream &o)=0;
		virtual bool isA(const LLCToken &tok)=0; // check if node is of particular type
		
		const uint32 linenum;
};

// General stringy stuff *****************************************************
class CStringNode : public CompileNode
{
	public:
		CStringNode(const char * const _str, const uint32 _linenum) :
			CompileNode(_linenum), str(_str) {};
		virtual ~CStringNode() {};

		virtual void print_unk(std::ostream &o) { o << str; };
		virtual bool isA(const LLCToken &tok) { return tok==LLC_STRING; };
		const std::string str;
};

class VarIdentNode : public CStringNode
{
	public:
		VarIdentNode(const char * const _idname, const uint32 _linenum) :
			CStringNode(_idname, _linenum) {};
		virtual ~VarIdentNode() {};

		virtual bool isA(const LLCToken &tok) { return tok==LLC_IDENT; };

};

// Classes *******************************************************************
class ClassNode : public CompileNode
{
	public:
		ClassNode(const uint32 _linenum) : CompileNode(_linenum) {}
		virtual ~ClassNode() {}

		void print_unk(std::ostream &o)
		{
			o << "class ";
			if(name!="")
				o << name << ' ';
		}

		bool isA(const LLCToken &tok) { return tok==LLC_CLASS; };

		std::string name;
};

#endif

