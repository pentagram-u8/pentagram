/*
 *	VarNodes.h -
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

#ifndef VARNODES_H
#define VARNODES_H

#include "Type.h"
#include "GenericNodes.h"
#include <string>

class PopVarNode : public UniNode
{
	public:
		PopVarNode(const uint32 opcode, const uint32 offset, const uint32 newValue)
			: UniNode(opcode, offset)
		{
			assert(acceptOp(opcode, 0x01));
			switch(opcode)
			{
				case 0x01: // popping a word var (2 bytes)
					_dtype = DataType(Type::T_WORD, DataType::DT_BP, newValue);
					break;
				default: assert(false);
			}
			rtype(_dtype.type());
		};
		PopVarNode(const uint32 opcode, const uint32 offset)
			: UniNode(opcode, offset)
		{
			assert(acceptOp(opcode, 0x12));
			switch(opcode)
			{
				case 0x12: // popping a word into temp
					_dtype = DataType(Type::T_WORD, DataType::DT_TEMP);
					break;
				default: assert(false);
			}
			rtype(_dtype.type());
		};
		~PopVarNode() { /*FORGET_OBJECT(lnode);*/ /* don't delete rnode */ };

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;

		bool fold(DCUnit * /*unit*/, std::deque<Node *> &nodes);

	protected:

	private:
		DataType	_dtype;
};

class PushVarNode : public Node
{
	public:
		PushVarNode(const uint32 opcode, const uint32 offset, const uint32 newValue)
			: Node(opcode, offset)
		{
			assert(acceptOp(opcode, 0x0B, 0x3F, 0x40, 0x4B) || acceptOp(opcode, 0x0C, 0x0A));
			switch(opcode)
			{
				case 0x0A: // pushing a byte (1 byte)
					_dtype = DataType(Type::T_BYTE, DataType::DT_BYTES, newValue);
					break;
				case 0x0B: // pushing a word (2 bytes)
					_dtype = DataType(Type::T_WORD, DataType::DT_BYTES, newValue);
					break;
				case 0x0C: // pushing a dword (4 bytes)
					_dtype = DataType(Type::T_DWORD, DataType::DT_BYTES, newValue);
					break;
				case 0x3F: // pushing a word var (2 bytes)
					_dtype = DataType(Type::T_WORD, DataType::DT_BP, newValue);
					break;
				case 0x40: // pushing a dword var (4 bytes)
					_dtype = DataType(Type::T_DWORD, DataType::DT_BP, newValue);
					break;
				case 0x4B: // pushing an address (4 bytes)
					_dtype = DataType(Type::T_DWORD, DataType::DT_BPADDR, newValue);
					break;
				default: assert(false);
			}
			rtype(_dtype.type());
		};
		PushVarNode(const uint32 opcode, const uint32 offset, const uint32 strSize, const std::string &str)
			: Node(opcode, offset)
		{
			assert(acceptOp(opcode, 0x0D));
			switch(opcode)
			{
				case 0x0D: // pushing a string (2 bytes)
					assert(strSize==str.size());
					_dtype = DataType(Type::T_STRING, DataType::DT_STRING, str);
					break;
				default: assert(false);
			}
			rtype(_dtype.type());
		};
		PushVarNode(const uint32 opcode, const uint32 offset, const uint32 variable, const uint32 varindex)
			: Node(opcode, offset)
		{
			assert(acceptOp(opcode, 0x4E));
			_dtype = DataType(Type::T_WORD, DataType::DT_GLOBAL, variable, varindex);
			rtype(_dtype.type());
		};
		PushVarNode(const uint32 opcode, const uint32 offset)
			: Node(opcode, offset)
		{
			assert(acceptOp(opcode, 0x59));
			switch(opcode)
			{
				case 0x59:
					_dtype = DataType(Type::T_WORD, DataType::DT_PID);
					break;
				default: assert(false);
			}
			rtype(_dtype.type());
		};

		~PushVarNode() {};

		void print_unk(Console &o, const uint32 isize) const;
		void print_asm(Console &o) const;
		void print_bin(ODequeDataSource &o) const;

		bool fold(DCUnit * /*unit*/, std::deque<Node *> &/*nodes*/) { return true; /* to be done */ };

		/* NOTE: 'VT_VOID' is not really a vartype, just used for proper typecasting of
			calls to functions that return no value, DT_DNULL isn't a real datatype either */
		/* NOTE: 'VT_VAR' is not really a vartype either, it's just to handle a variable
			number of parameters pushed as a global, if it's not a 'nice' number */

		const DataType &dtype() const { return _dtype; };

	protected:

	private:
		DataType	_dtype;

/*		sint32		value;
		// for list use only
		uint32		value2;
		// for string use only
		std::string		strval;
		// for global use only
		uint32 global_offset;
		uint32 global_size;*/
};

#endif
