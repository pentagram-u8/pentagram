/*
 *	CallNodes.h -
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

#ifndef CALLNODES_H
#define CALLNODES_H

#include "GenericNodes.h"

class CallPostfixNode : public Node
{
	public:
		CallPostfixNode() : Node() {};
		CallPostfixNode(const uint32 opcode, const uint32 offset, const uint32 newSP)
			: Node(opcode, offset, Type(Type::T_INVALID)), sp(newSP)
			{
				assert(acceptOp(opcode, 0x6E));
				switch(opcode)
				{
					case 0x6E: ptype=ADDSP; break;
					default: assert(false);
				}
			};
		CallPostfixNode(const uint32 opcode, const uint32 offset)
			: Node(opcode, offset, Type(Type::T_INVALID)), sp(0)
			{
				assert(acceptOp(opcode, 0x5D, 0x5E));
				switch(opcode)
				{
					case 0x5D: ptype=PUSH_RETVAL; rtype(Type::T_BYTE); break;
					case 0x5E: ptype=PUSH_RETVAL; rtype(Type::T_WORD); break;
					default: assert(false);
				}
			};
		~CallPostfixNode() {};

		void print() const {};
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(OBufferDataSource &o) const;

		bool fold(Unit *unit, std::deque<Node *> &nodes);

		sint32 size() const { return static_cast<sint8>(sp); };

	protected:
		enum callpostfixtype { PUSH_RETVAL, ADDSP } ptype;

	private:
		uint32 sp;
};

class CallMutatorNode : public BinNode
{
	public:
		CallMutatorNode(const uint32 opcode, const uint32 offset, const uint32 newNumBytes)
			: BinNode(opcode, offset, Type(Type::T_INVALID)), numBytes(newNumBytes)
			{
				assert(acceptOp(opcode, 0x4C));
				mtype=PUSH_INDIRECT;
				switch(numBytes)
				{
					case 2: rtype(Type::T_WORD); break;
					default: assert(false);
				}
			};
		CallMutatorNode(const uint32 opcode, const uint32 offset)
			: BinNode(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x77, 0x78));
				switch(opcode)
				{
					case 0x77: mtype=SET_INFO; break;
					case 0x78: mtype=PROCESS_EXCLUDE; break;
					default: assert(false);
				}
			};
		~CallMutatorNode() {};

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(OBufferDataSource &o) const;

		bool fold(Unit *unit, std::deque<Node *> &nodes);

	protected:
		enum mutatortype { PUSH_INDIRECT, SET_INFO, PROCESS_EXCLUDE } mtype;

	private:
		uint32 numBytes;
};

class CallNode : public ColNode
{
	public:
		CallNode() : ColNode() {};
		CallNode(const uint32 opcode, const uint32 offset, const uint32 newValue1, const uint32 newValue2)
			: ColNode(opcode, offset, Type(Type::T_VOID)), addSP(0), retVal(0)
			{
				assert(acceptOp(opcode, 0x0F, 0x11));
				switch(opcode)
				{
					case 0x0F:
						ctype = CALLI;
						spsize = newValue1;
						intrinsic = newValue2;
						break;
					case 0x11:
						ctype = CALL;
						uclass = newValue1;
						targetOffset = newValue2;
						break;
					default: assert(false);
				}
			};
		~CallNode() {};

		void print() const {};
		void print_extern_unk(Console &o, const uint32 isize) const;
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(OBufferDataSource &o) const;

		bool fold(Unit *unit, std::deque<Node *> &nodes);

		// 'special' functions
		void setAddSP(CallPostfixNode *newAddSP) { addSP = newAddSP; };
		void setRetVal(CallPostfixNode *newRetVal) { retVal = newRetVal; };

	protected:
		enum calltype { CALLI, CALL /*, SPAWN*/ } ctype;

	private:
		uint32 uclass; // call
		uint32 targetOffset; // call
		
		uint32 spsize; // calli
		uint32 intrinsic; // calli
		
		CallPostfixNode *addSP;
		CallPostfixNode *retVal;
};


#endif
