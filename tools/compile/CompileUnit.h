/*
 *	CompileUnit.h - The core of the compiler
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

#ifndef COMPILEUNIT_H
#define COMPILEUNIT_H

#include "CompileNodes.h"
#include "IDataSource.h"
#include "FileSystem.h"

#include <list>
#include <fstream>

class llcFlexLexer;

class CompileUnit
{
	public:
		enum CState { CSTATE_NEW, CSTATE_FAIL, CSTATE_WORKING, CSTATE_FINISHED };

		CompileUnit(FileSystem *filesystem) : currclass(0), ifile(0), idatasource(0),
			parser(0), filesys(filesystem), _state(CSTATE_NEW)
			#ifdef COMPILER_TEST
			,testidx(0)
			#endif
		{
			setState(CSTATE_NEW);
		}

		~CompileUnit()
		{}

		bool parse();
		bool parse_openblock();
		bool parse_closeblock();

		void debugPrint(std::ostream &o, CompileNode *n) const;
		void debugPrint(std::ostream &o) const;

		// state handling
		CState state() const { return _state; };
		bool setState(const CState cs);

	private:
		// internal print stuff
		void debugPrintHead(std::ostream &o, CompileNode *n=0) const;
		void debugPrintBody(std::ostream &o) const;

		// parser shortcut checking
		inline bool found(const LLCToken &tok) const { return nodes.size()>0 && nodes.front()->isA(tok); }
		bool consume(const LLCToken &tok);

		std::list<CompileNode *> nodes; // the temporary node list, cleared upon finding a seperator

		//std::list<ExpressionNode *> expressions; // the current expression stack
		//StatNode *currstat; // the current statement we're building
		//FuncNode *currfunc; // the current function we're operating on
		ClassNode *currclass; // the current class (and thus current file) we're operating on
		std::list<ClassNode *> tailclasses; // the classes we've previously compiled
		/* The basic logic of the above is that we parse expressions until the stack is full,
			then fold the expressions down to one expression 'tree' which then gets attached
			to the RHS of the currstat, which then gets appended to the internal statement
			list of currfunc, which when the function is finished parsing gets appended to
			the tail of the funclist inside currclass, once the class is finished it gets stored
			on the tailclasses stack.
			We can actually probably discard most of the data in the class and store it's info in
			a different data structure for outputting to some sort of index struct.

			Scarily enough, I think this is the simplest way of massaging this...
		*/
		
		std::ifstream *ifile; // real file we use
		IDataSource *idatasource; // temp data source

		llcFlexLexer *parser;

		FileSystem * const filesys; // the filesystem from Application

		CState _state; // the current state of the compile
		#ifdef COMPILER_TEST
		uint32 testidx;
		#endif
};

#endif


