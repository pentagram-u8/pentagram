/*
 *	IfNode.h -
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

#ifndef IFNODE_H
#define IFNODE_H

#include "GenericNodes.h"
#include <deque>
#include "CallNodes.h"

class EndNode : public Node
{
	public:
		EndNode(const uint32 opcode, const uint32 offset, const uint32 newTargetOffset)
			: Node(opcode, offset, Type(Type::T_INVALID)), targetOffset(newTargetOffset)
			{
				assert(acceptOp(opcode, 0x52));
				switch(opcode)
				{
					case 0x52: etype = JMP; break;
					default: assert(false);
				}
			};
		~EndNode() {};
		
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;
		bool fold(DCUnit *unit, std::deque<Node *> &nodes);
		
		inline uint32 TargetOffset() const { return targetOffset; };
		
	protected:
		enum endtype { JMP } etype;

	private:
		uint32 targetOffset; // jmp
};

class IfNode;

class IfNode : public UniNode
{
	public:
		enum iftype { I_IF=0, I_IF_ELSE, I_IF_ELSE_IF, I_ELSE_IF, I_ELSE_IF_ELSE, I_ELSE };
		
		IfNode(const iftype newItype, const uint32 newTargetOffset)
			: UniNode(0x51, 0x0000, Type(Type::T_INVALID)), itype(newItype),
			targetOffset(newTargetOffset), jmpnode(0), elsenode(0)
			{};
		IfNode(const uint32 opcode, const uint32 offset, const uint32 newTargetOffset)
			: UniNode(opcode, offset, Type(Type::T_INVALID)),
			targetOffset(newTargetOffset), jmpnode(0), elsenode(0)
			{
				assert(acceptOp(opcode, 0x51));
				switch(opcode)
				{
					case 0x51: itype = I_IF; break;
					default: assert(false);
				}
			};
		~IfNode() {};
		
		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;
		bool fold(DCUnit *unit, std::deque<Node *> &nodes);
		bool fold_else(DCUnit *unit, std::deque<Node *> &nodes);
		
		inline uint32 TargetOffset() const
		{
			if(jmpnode!=0)
				return jmpnode->TargetOffset();
			return targetOffset;
		};
		
		void AddJmp(EndNode *jmp) { assert(jmpnode==0); jmpnode=jmp; };
		
		std::deque<Node *> &nodes() { return ifnodes; };
		IfNode *elsen() { return elsenode; };

		/* IF, IF_ELSE, and IF_ELSE_IF are all technically the same type,
			what they represent is the state the function is in.

			* If we're at IF, then we've either not parsed a 'cmp' to terminate
			our basic block yet, or we're 'just' a plain if(){} statement.
			* If we're at IF_ELSE, then we've found our associated 'cmp' opcode,
			just before our targetOffset, and we're either an if(){}else{} statment
			or we've not tripped over the next 'if' statement in an if/else if/else
			series.
			* If we're at IF_ELSE_IF, and IF node has been parsed coming after us,
			and we were already an IF_ELSE flagged node.
		*/
		iftype itype;

		// 'special' functions
		void setAddSP(DCCallPostfixNode *newAddSP) { addSP = newAddSP; };

	protected:
		std::deque<Node *> ifnodes;

	private:
		uint32 targetOffset; // jne
		EndNode *jmpnode; // this is for when we're handling the pessimised if(true){code}->if(false){}else{code}
		DCCallPostfixNode *addSP;
	public:
		IfNode  *elsenode;
};

#endif
