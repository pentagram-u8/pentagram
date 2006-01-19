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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MATHNODES_H
#define MATHNODES_H

#include "GenericNodes.h"

/*bool is_temp(uint32 op)
{
	// in crusader 'u8's end is actually a push type opcode.
	if((!crusader) && op==0x79)
		return true;

	switch(op)
	{
		case 0x12: // pop temp
		//case 0x4E: // push global
		case 0x4F: // pop global
		case 0x50: // ret
		case 0x51: // jne
		case 0x52: // jmp
		case 0x53: // suspend
		case 0x5D: // push byte retval
		case 0x5E: // push word retval
		case 0x5F: // push dword retval
		case 0x62: // free string bp
		case 0x63: // free slist bp
		case 0x64: // free list bp
		case 0x65: // free string sp
		case 0x66: // free list sp
		//case 0x67: // free slist sp
		case 0x6C: // param pid change
		case 0x70: // loop
		case 0x73: // loopnext
		case 0x74: // loopscr
		case 0x75: // foreach list
		case 0x76: // foreach slist
		case 0x7A: // end
			return true;
		default:   // else...
			return false;
	}
	return false; // can't happen
}

class TempNode : public Node
{
	public:
		TempNode() : Node(), offset(0) {};
		~TempNode() {};

		void print() const;
		void fold(const uint32 end);

		enum temptype { POP_TEMP, PUSH_GLOBAL, POP_GLOBAL,
			RET, JNE, JMP, SUSPEND,
			PUSH_BYTE_RET, PUSH_WORD_RET, PUSH_DWORD_RET,
			FREE_STRING_BP, FREE_SLIST_BP, FREE_LIST_BP,
			FREE_STRING_SP, FREE_LIST_SP, FREE_SLIST_SP,
			PARAM_PID_CHANGE,
			LOOP, LOOPNEXT, LOOPSCR,
			FOREACH_LIST, FOREACH_SLIST,
			END
			} ttype;

	protected:

	private:
		uint32 offset; // jne/jmp/ *global/foreach*list
		sint32 delta; // loopscr
		Node *node; // loop/free* /foreach*list/param pid change
		uint32 size; // loop/ *global/foreach*list
		uint32 type; // loop/param pid change
};*/


/*bool is_convert(uint32 op)
{
	switch(op)
	{
		case 0x60: // word-to-dword
		case 0x61: // dword-to-word
		case 0x6B: // str-to-ptr
			return true;
		default:   // else...
			return false;
	}
	return false; // can't happen
}

class ConvertNode : public UniNode
{
	public:
		ConvertNode() : UniNode("Convert"), ctype(NONE) {};
		~ConvertNode() {};

		void print() const;
		void fold(const uint32 end);

	protected:
		enum convtype { NONE, WORD_DWORD, DWORD_WORD, STR_PTR } ctype;

	private:
};*/

class UniOperatorNode : public UniNode
{
	public:
		UniOperatorNode(const uint32 opcode, const uint32 offset)
			: UniNode(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x30, 0x6B));
				switch(opcode)
				{
					case 0x30: otype = NOT; rtype(Type::T_WORD); break;
					case 0x6B: otype = STR_TO_PTR; rtype(Type::T_STRPTR); break;
					default: assert(false);
				}
			};
		~UniOperatorNode() {};

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource  &o) const;
		bool fold(DCUnit *unit, std::deque<Node *> &nodes);

	protected:
		enum optype { NOT, STR_TO_PTR/*, BITNOT*/ } otype;

	private:
};

class BinOperatorNode : public BinNode
{
	public:
		BinOperatorNode(const uint32 opcode, const uint32 offset)
			: BinNode(opcode, offset, Type(Type::T_INVALID))
			{
				assert(acceptOp(opcode, 0x14, 0x1C, 0x1E) || acceptOp(opcode, 0x24, 0x28, 0x2A, 0x2C)
					|| acceptOp(opcode, 0x32, 0x34, 0x36) || acceptOp(opcode, 0x54));
				switch(opcode)
				{
					case 0x14: otype = M_ADD;     rtype(Type::T_WORD); break;
					case 0x1C: otype = M_SUB;     rtype(Type::T_WORD); break;
					case 0x1E: otype = M_MUL;     rtype(Type::T_WORD); break;

					case 0x24: otype = M_CMP;     rtype(Type::T_WORD); break;
					case 0x28: otype = M_LT;      rtype(Type::T_WORD); break;
					case 0x2A: otype = M_LE;      rtype(Type::T_WORD); break;
					case 0x2C: otype = M_GT;      rtype(Type::T_WORD); break;
					case 0x32: otype = M_AND;     rtype(Type::T_WORD); break;
					case 0x34: otype = M_OR;      rtype(Type::T_WORD); break;
					case 0x36: otype = M_NE;      rtype(Type::T_WORD); break;

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
		enum optype { M_ADD, M_SUB, M_MUL, M_CMP, M_AND, M_OR, M_NE, M_LT, M_LE, M_GT, M_IMPLIES } otype;

	private:
};

#endif
