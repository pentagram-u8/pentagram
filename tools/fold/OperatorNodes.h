/*
 *	OperatorNodes.h -
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

#ifndef MATHNODES_H
#define MATHNODES_H

#include "GenericNodes.h"

class UniOperatorNode : public UniNode
{
	public:
		UniOperatorNode(const uint32 opcode, const uint32 offset)
			: UniNode(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x30));
				switch(opcode)
				{
					case 0x30: otype = NOT; rtype(Type::T_WORD); break;
					default: assert(false);
				}
			};
		~UniOperatorNode() {};

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource  &o) const;
		bool fold(DCUnit *unit, std::deque<Node *> &nodes);

	protected:
		enum optype { NOT/*, BITNOT*/ } otype;

	private:
};

class BinOperatorNode : public BinNode
{
	public:
		BinOperatorNode(const uint32 opcode, const uint32 offset)
			: BinNode(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x14) || acceptOp(opcode, 0x24, 0x28, 0x2A, 0x2C)
					|| acceptOp(opcode, 0x34) || acceptOp(opcode, 0x54));
				switch(opcode)
				{
					case 0x14: otype = M_ADD;     rtype(Type::T_WORD); break;
					
					case 0x24: otype = M_CMP;     rtype(Type::T_WORD); break;
					case 0x28: otype = M_LT;      rtype(Type::T_WORD); break;
					case 0x2A: otype = M_LE;      rtype(Type::T_WORD); break;
					case 0x2C: otype = M_GT;      rtype(Type::T_WORD); break;
					case 0x34: otype = M_OR;      rtype(Type::T_WORD); break;
					
					case 0x54: otype = M_IMPLIES; rtype(Type::T_WORD); break;

					default: assert(false);
				}
			};
		~BinOperatorNode() {};

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource  &o) const;
		bool fold(DCUnit *unit, std::deque<Node *> &nodes);

	protected:
		enum optype { M_ADD, M_CMP, M_OR, M_LT, M_LE, M_GT, M_IMPLIES } otype;

	private:
};

#endif
