/*
 *	CallNodes.h -
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

#ifndef CALLNODES_H
#define CALLNODES_H

#include "GenericNodes.h"

#include <list>

class DCCallPostfixNode : public Node
{
	public:
		DCCallPostfixNode() : Node() {};
		DCCallPostfixNode(const uint32 opcode, const uint32 offset, const uint32 newSP)
			: Node(opcode, offset, Type(Type::T_INVALID)), sp(newSP)
			{
				assert(acceptOp(opcode, 0x65, 0x6E));
				switch(opcode)
				{
					case 0x65: ptype=FREESTR; break;
					case 0x6E: ptype=ADDSP; break;
					default: assert(false);
				}
			};
		DCCallPostfixNode(const uint32 opcode, const uint32 offset)
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
		~DCCallPostfixNode() {};

		void print() const {};
		void print_unk(Console &o, const uint32 isize, const bool comment) const;
		void print_unk(Console &o, const uint32 isize) const { print_unk(o, isize, true); };
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;

		bool fold(DCUnit *unit, std::deque<Node *> &nodes);

		sint32 size() const { return static_cast<sint8>(sp); };

	protected:
		enum callpostfixtype { PUSH_RETVAL, FREESTR, ADDSP } ptype;

	private:
		uint32 sp;
};

class DCCallMutatorNode : public BinNode
{
	public:
		DCCallMutatorNode(const uint32 opcode, const uint32 offset, const uint32 newNumBytes)
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
		DCCallMutatorNode(const uint32 opcode, const uint32 offset)
			: BinNode(opcode, offset, Type(Type::T_INVALID)), numBytes(0)
			{
				assert(acceptOp(opcode, 0x77, 0x78));
				switch(opcode)
				{
					case 0x77: mtype=SET_INFO; break;
					case 0x78: mtype=PROCESS_EXCLUDE; break;
					default: assert(false);
				}
			};
		~DCCallMutatorNode() {};

		void print_unk(Console &o, const uint32 isize, const bool comment) const;
		void print_unk(Console &o, const uint32 isize) const { print_unk(o, isize, true); };
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;

		bool fold(DCUnit *unit, std::deque<Node *> &nodes);

	protected:
		enum mutatortype { PUSH_INDIRECT, SET_INFO, PROCESS_EXCLUDE } mtype;

	private:
		uint32 numBytes;
	
	public: // accessors
		const Node *a_lnode() const { return lnode; };
		const Node *a_rnode() const { return rnode; };
};

class DCCallNode : public ColNode
{
	public:
		DCCallNode() : ColNode() {};
		DCCallNode(const uint32 opcode, const uint32 offset, const uint32 newValue1, const uint32 newValue2)
			: ColNode(opcode, offset, Type(Type::T_VOID)), addSP(0), retVal(0), thisP(0)
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
		DCCallNode(const uint32 opcode, const uint32 offset, const uint32 newValue1, const uint32 newValue2, const uint32 newValue3, const uint32 newValue4)
			: ColNode(opcode, offset, Type(Type::T_VOID)), addSP(0), retVal(0), thisP(0)
			{
				assert(acceptOp(opcode, 0x57));
				con.Printf("SPAWN: %04X: %02X %02X %02X\n", opcode, offset, newValue1, newValue2);
				switch(opcode)
				{
					case 0x57:
						ctype = SPAWN;
						spsize = newValue1;
						thispsize = newValue2;
						uclass = newValue3;
						targetOffset = newValue4;
						break;
					default: assert(false);
				}
			};
		~DCCallNode() {};

		void print() const {};
		void print_extern_unk(Console &o, const uint32 isize) const;
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;

		bool fold(DCUnit *unit, std::deque<Node *> &nodes);

		// 'special' functions
		void setAddSP(DCCallPostfixNode *newAddSP) { addSP = newAddSP; };
		void setRetVal(DCCallPostfixNode *newRetVal) { retVal = newRetVal; rtype(retVal->rtype()); };
		void addFree(DCCallPostfixNode *newFree) { freenodes.push_back(newFree); };

	protected:
		enum calltype { CALLI, CALL, SPAWN } ctype;

	private:
		uint32 uclass; // call & spawn
		uint32 targetOffset; // call & spawn
		
		uint32 spsize; // calli & spawn
		uint32 intrinsic; // calli
		
		uint32 thispsize; // spawn

		DCCallPostfixNode *addSP;
		DCCallPostfixNode *retVal;
		Node *thisP;
		std::list<DCCallPostfixNode *> freenodes;
};


#endif
