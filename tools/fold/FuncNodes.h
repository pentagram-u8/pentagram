/*
 *	FuncNodes.h -
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef FUNCNODES_H
#define FUNCNODES_H

#include "GenericNodes.h"
#include <deque>
#include <string>

class FuncMutatorNode : public Node
{
	public:
		FuncMutatorNode(const uint32 opcode, const uint32 offset, const uint32 newValue1)
			: Node(opcode, offset, Type(Type::T_INVALID)), _initsize(newValue1), _linenum(newValue1)
			{
				assert(acceptOp(opcode, 0x5A, 0x5B));
				switch(opcode)
				{
					case 0x5A: mtype = INIT; break;
					case 0x5B: mtype = LINE_NUMBER; break;
					default: assert(false);
				}
			};
		FuncMutatorNode(const uint32 opcode, const uint32 offset, const uint32 newSymbolOffset, const std::string &newClassName)
			: Node(opcode, offset, Type(Type::T_INVALID)), _symboloffset(newSymbolOffset), _classname(newClassName)
			{
				assert(acceptOp(opcode, 0x5C));
				mtype = SYMBOL_INFO;
			};
		FuncMutatorNode(const uint32 opcode, const uint32 offset)
			: Node(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x50));
				switch(opcode)
				{
					case 0x50: mtype = RET; break;
					default: assert(false);
				}
			};
		~FuncMutatorNode() {};

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(OBufferDataSource &o) const;
		bool fold(Unit *unit, std::deque<Node *> &nodes);

	protected:
		enum mutatortype { RET, INIT, LINE_NUMBER, SYMBOL_INFO } mtype;

	private:
		uint32 _initsize; // init
		uint32 _linenum; // line number
		uint32 _symboloffset; // symbol info
		std::string _classname; // symbol info
};

class FuncNode : public ColNode
{
	public:
		FuncNode(const uint32 opcode, const uint32 offset)
			: ColNode(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x7A));
			};
		~FuncNode() {};

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(OBufferDataSource &o) const;
		bool fold(Unit *unit, std::deque<Node *> &nodes);

	protected:
	
	private:
};

#endif
