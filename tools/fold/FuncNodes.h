/*
 *	FuncNodes.h -
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef FUNCNODES_H
#define FUNCNODES_H

#include "GenericNodes.h"
#include "CallNodes.h"
#include <deque>
#include <string>

class FuncMutatorNode : public Node
{
	public:
		FuncMutatorNode(const uint32 opcode, const uint32 offset, const uint32 newValue1)
			: Node(opcode, offset, Type(Type::T_INVALID)),
			_initsize(newValue1), _linenum(newValue1)
			{
				assert(acceptOp(opcode, 0x5A, 0x5B));
				switch(opcode)
				{
					case 0x5A: mtype = INIT; break;
					case 0x5B: mtype = LINE_NUMBER; break;
					default: assert(false);
				}
			};
		FuncMutatorNode(const uint32 opcode, const uint32 offset, const uint32 newSymbolOffset,
			const std::string &newClassName)
			: Node(opcode, offset, Type(Type::T_INVALID)),
			_symboloffset(newSymbolOffset), _classname(newClassName)
			{
				assert(acceptOp(opcode, 0x5C));
				mtype = SYMBOL_INFO;
			};
		FuncMutatorNode(const uint32 opcode, const uint32 offset)
			: Node(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x50, 0x53, 0x7A));
				switch(opcode)
				{
					case 0x50: mtype = RET; break;
					case 0x53: mtype = SUSPEND; break;
					case 0x7A: mtype = END; break;
					default: assert(false);
				}
			};
		~FuncMutatorNode() {};

		void print_unk(Console &o, const uint32 isize, const bool comment) const;
		void print_unk(Console &o, const uint32 isize) const { print_unk(o, isize, true); };
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;
		bool fold(DCUnit *unit, std::deque<Node *> &nodes);

	protected:
		enum mutatortype { RET, INIT, LINE_NUMBER, SYMBOL_INFO, SUSPEND, END } mtype;

	private:
		uint32 _initsize; // init
		uint32 _linenum; // line number
		uint32 _symboloffset; // symbol info
		std::string _classname; // symbol info
	
	public: // wheeefun accessors.
		uint32 a_initsize() const { assert(opcode()==0x5A && mtype==INIT); return _initsize; };
};

class DCFuncNode : public ColNode
{
	public:
		DCFuncNode()
			: ColNode(0xFFFF, 0x0000, Type(Type::T_INVALID)),
			setinfonode(0),
			locals_datasize(0), func_start_offset(0),
			debug_ret_offset(0), debug_end_offset(0),
			process_type(0), debug_processtype_offset(0), debug_thisp(false),
			debug_procexclude_offset(0), has_procexclude(false)
			{
			};
		~DCFuncNode() {};

		void print_unk_funcheader(Console &o, const uint32 isize) const;
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;
		bool fold(DCUnit *unit, std::deque<Node *> &nodes);
		void addEnd(FuncMutatorNode *n)
		{
			assert(n!=0);
			assert(n->opcode()==0x7A);
			debug_end_offset = n->offset();
			FORGET_OBJECT(n);
		};
		
	protected:
		std::deque<Node *> funcnodes;
		DCCallMutatorNode *setinfonode;

		uint32	locals_datasize; // from 'init' opcode
		uint32	func_start_offset; // from 'init' opcode
		uint32	debug_ret_offset; // from 'ret' opcode, FIXME: debugging only
		uint32	debug_end_offset; // from 'end' opcode, FIXME: debugging only
		
		uint32	process_type; // from 'set info' opcode
		uint32	debug_processtype_offset; // from 'set info' opcode, FIXME: debugging only
		bool	debug_thisp; // from 'set info' opcode.

		uint32	debug_procexclude_offset; // from 'process exclude' opcode, FIXME: debugging only
		bool	has_procexclude; // from 'process exclude' opcode

	private:
		void fold_init(DCUnit *unit, std::deque<Node *> &nodes);
		void fold_ret(DCUnit *unit, std::deque<Node *> &nodes);
		void fold_setinfo(DCUnit *unit, std::deque<Node *> &nodes);
		void fold_procexclude(DCUnit *unit, std::deque<Node *> &nodes);
};

#endif

