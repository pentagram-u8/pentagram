/*
 *	VarNodes.cpp -
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

#include "pent_include.h"

#include "VarNodes.h"

/****************************************************************************
	PopVarNode
 ****************************************************************************/

/*void PopVarNode::fold(const uint32 end)
{
	PTRACE(("(PopVar)\tPOS: %4d\tOP: %04X offset: %04X\n", end, foldops[end].op(),
		foldops[end].offset));
	assert(foldops[end].deleted==false);

	_opcode = foldops[end].op(); // store the op locally
	_offset = foldops[end].offset; // store the offset locally

	switch(_opcode)
	{
		case 0x00: // popping a byte (1 byte)
			lnode  = new PushVarNode(PushVarNode::VT_BYTE, PushVarNode::DT_BP,
				foldops[end].i0);
			rtype=Type::T_VOID;
			break;
		case 0x01: // popping a word (2 bytes)
			lnode  = new PushVarNode(PushVarNode::VT_WORD, PushVarNode::DT_BP,
				foldops[end].i0);
			rtype=Type::T_VOID;
			break;
		case 0x02: // popping a dword (4 bytes)
			lnode  = new PushVarNode(PushVarNode::VT_DWORD, PushVarNode::DT_BP,
				foldops[end].i0);
			rtype=Type::T_VOID;
			break;
		case 0x08: // popping a result (4 bytes)
			lnode  = new PushVarNode(PushVarNode::VT_DWORD, PushVarNode::DT_RESULT, 0);
			rtype=Type::T_VOID;
			break;
		default: // die if we don't know the op
			printf("\nopcode %02X not supported in call to PopVarNode from offset %04X\n",
				_opcode, _offset);
			//printfolding();
			assert(false);
			break;
	}

	sint32 tempsize = lnode->rtype.size();
	assert(tempsize>0);
	grab_r(tempsize, end);
	assert(tempsize==0);
}*/


void PopVarNode::print_unk(Console &o, const uint32 isize) const
{
	assert(node!=0);
	Node::print_linenum_unk(o, isize);
	#if 0
	_dtype.print_type_unk(o);
	o.Putchar(' ');
	#endif
	_dtype.print_value_unk(o);
	o.Print(" = ");
	node->print_unk(o, isize);
}

void PopVarNode::print_asm(Console &o) const
{
	assert(node!=0);
	Node::print_linenum_asm(o);
	node->print_asm(o);
	o.Putchar('\n');
	Node::print_asm(o);
	switch(_dtype.dtype())
	{
		case DataType::DT_BP:
			switch(_dtype.type().type())
			{
				case Type::T_WORD:  o.Printf("pop\t\t");     _dtype.print_value_asm(o); break;
				//case Type::T_DWORD: o.Printf("pop dword\t"); _dtype.print_value_asm(o); break;
				default: assert(false);
			}
			break;
		case DataType::DT_TEMP:
			switch(_dtype.type().type())
			{
				case Type::T_WORD:  o.Printf("pop\t\ttemp"); break;
				default: assert(false);
			}
			break;
		default: assert(false);
	}
}

void PopVarNode::print_bin(ODequeDataSource  &o) const
{
	assert(node!=0);
	Node::print_linenum_bin(o);
	node->print_bin(o);
	switch(_dtype.dtype())
	{
		case DataType::DT_BP:
			switch(_dtype.type().type())
			{
				case Type::T_WORD:  o.write1(0x01); _dtype.print_value_bin(o); break;
				default: assert(false);
			}
			break;
		case DataType::DT_TEMP:
			switch(_dtype.type().type())
			{
				case Type::T_WORD:  o.write1(0x12); break;
				default: assert(false);
			}
			break;
		default: assert(false);
	}
}

bool PopVarNode::fold(DCUnit *unit, std::deque<Node *> &nodes)
{
	//con.Printf("RTypes: %d %d", nodes.back()->rtype().type(), rtype().type());
	assert(nodes.back()->rtype()==rtype() || print_assert(this, unit));
	grab_n(nodes);
	fold_linenum(nodes);
	return true;
}


/****************************************************************************
	PushVarNode
 ****************************************************************************/

/*void PushVarNode::fold(const uint32 end)
{
	PTRACE(("(PushVar)\tPOS: %4d\tOP: %04X offset: %04X\n", end, foldops[end].op(),
		foldops[end].offset));
	assert(foldops[end].deleted==false);*/

/*	_opcode = foldops[end].op(); // store the op locally
	_offset = foldops[end].offset; // store the offset locally
*/
/*	switch(_opcode)
	{
		case 0x0D: // pushing a string (2 bytes)
			_vtype = VT_STRING;
			_dtype  = DT_CHARS;
			strval = foldops[end].str;
			rtype=Type::T_STRING;
			break;
		case 0x3E: // pushing a byte var (2 bytes)
			_vtype  = VT_BYTE;
			_dtype  = DT_BP;
			value  = foldops[end].i0;
			rtype=Type::T_BYTE;
			break;
		case 0x3F: // pushing a word var (2 bytes)
			_vtype  = VT_WORD;
			_dtype  = DT_BP;
			value  = foldops[end].i0;
			rtype=Type::T_WORD;
			break;
		case 0x41: // pushing a string var (2 bytes)
			_vtype = VT_STRING;
			_dtype  = DT_BP;
			value  = foldops[end].i0;
			rtype=Type::T_STRING;
			break;
		case 0x42: // pushing a list (2 bytes?)
			_vtype = VT_LIST;
			_dtype  = DT_BPLIST;
			value  = foldops[end].i0;
			value2 = foldops[end].i1;
			assert(value2==2); // FIXME: incorrect, but a quick hack for my purposes
			rtype=Type::T_LIST;
			break;
		case 0x43: // pushing a slist (2 bytes?)
			_vtype  = VT_SLIST;
			_dtype  = DT_BP;
			value  = foldops[end].i0;
			rtype=Type::T_SLIST;
			break;
		case 0x4E: // pushing a global (x bytes - round up to an even pair of bytes
			_dtype  = DT_GLOBAL;
			global_offset = foldops[end].i0;
			global_size   = foldops[end].i1;
			switch(global_size)
			{
				case 0x01: _vtype = VT_BYTE;  rtype=Type::T_BYTE; break;
				case 0x02: _vtype = VT_WORD;  rtype=Type::T_WORD; break;
				case 0x03: _vtype = VT_DWORD; rtype=Type::T_DWORD; break;
//				case 0x04: _vtype = DWORD; rtype=Type::T_DWORD; break;
				// debugging, remove this and replace it with a _vtype=VAR when finished
				default: assert(false);
			}
			break;
		case 0x59: // pushing a pid (2 bytes - maybe)
			_vtype  = VT_VPID;
			_dtype  = DT_DPID;
			value  = 0; // unused
			rtype=Type::T_PID;
			break;
		case 0x69: // pushing a string ptr (4 bytes)
			_vtype = VT_DWORD;
			_dtype  = DT_BPSTRPTR;
			value  = 0x100-foldops[end].i0;
			rtype=Type::T_DWORD;
			break;
		case 0x6D: // pushing an address of a SP (4 bytes)
			_vtype = VT_DWORD;
			_dtype  = DT_PRESULT;
			value  = 0; // unused
			rtype=Type::T_DWORD;
			break;
		case 0x6F: // pushing an address of a SP (4 bytes)
			_vtype = VT_DWORD;
			_dtype  = DT_SPADDR;
			value  = 0x100-foldops[end].i0;
			rtype=Type::T_DWORD;
			break;
		case 0x79: // pushing a global address (4 bytes)
			if(crusader) // we're only a global for crusader
			{
				_vtype = VT_DWORD;
				_dtype  = DT_SPADDR;
				value  = 0x100-foldops[end].i0;
				rtype=Type::T_DWORD;
			}
			else // not for u8
				assert(false);
			break;
		default: // die if we don't know the op
			printf("\nopcode %02X not supported in call to PushVarNode from offset %04X\n",
				_opcode, _offset);
			//printfolding();
			assert(false);
			break;
	}
}*/

void PushVarNode::print_unk(Console &o, const uint32 /*isize*/) const
{
	#if 0
	_dtype.print_type_unk(o);
	o.Putchar(' ');
	#endif
	_dtype.print_value_unk(o);
}

void PushVarNode::print_asm(Console &o) const
{
	Node::print_asm(o);
	switch(_dtype.dtype())
	{
		case DataType::DT_BYTES:
			switch(_dtype.type().type())
			{
				case Type::T_BYTE:  o.Printf("push byte\t");  _dtype.print_value_asm(o); break;
				case Type::T_WORD:  o.Printf("push\t\t");     _dtype.print_value_asm(o); break;
				case Type::T_DWORD: o.Printf("push dword\t"); _dtype.print_value_asm(o); break;
				default: assert(false); // can't happen
			}
			break;
		case DataType::DT_BP:
			switch(_dtype.type().type())
			{
				case Type::T_WORD:  o.Printf("push\t\t");     _dtype.print_value_asm(o); break;
				case Type::T_DWORD: o.Printf("push dword\t"); _dtype.print_value_asm(o); break;
				default: assert(false);
			}
			break;
		case DataType::DT_BPADDR:
			switch(_dtype.type().type())
			{
				case Type::T_DWORD: o.Printf("push addr\t"); _dtype.print_value_asm(o); break;
				default: assert(false); // can't happen
			}
			break;
		case DataType::DT_STRING:
			switch(_dtype.type().type())
			{
				case Type::T_STRING: o.Printf("push string\t"); _dtype.print_value_asm(o); break;
				default: assert(false); // can't happen
			}
			break;
		case DataType::DT_PID:
			//assert(_dtype.type().type()==Type::T_PID);
			o.Printf("push\t\tpid");
			break;
		case DataType::DT_GLOBAL:
			assert(_dtype.type().type()==Type::T_WORD);
			o.Printf("push\t\tglobal ");
			_dtype.print_value_asm(o);
			break;
		default: assert(false);
	}
}

void PushVarNode::print_bin(ODequeDataSource  &o) const
{
	switch(_dtype.dtype())
	{
		case DataType::DT_BYTES:
			switch(_dtype.type().type())
			{
				case Type::T_BYTE:  o.write1(0x0A); _dtype.print_value_bin(o); break;
				case Type::T_WORD:  o.write1(0x0B); _dtype.print_value_bin(o); break;
				case Type::T_DWORD: o.write1(0x0C); _dtype.print_value_bin(o); break;
				default: assert(false); // can't happen
			}
			break;
		case DataType::DT_BP:
			switch(_dtype.type().type())
			{
				case Type::T_WORD:  o.write1(0x3F); _dtype.print_value_bin(o); break;
				case Type::T_DWORD: o.write1(0x40); _dtype.print_value_bin(o); break;
				default: assert(false);
			}
			break;
		case DataType::DT_BPADDR:
			switch(_dtype.type().type())
			{
				case Type::T_DWORD: o.write1(0x4B); _dtype.print_value_bin(o); break;
				default: assert(false); // can't happen
			}
			break;
		case DataType::DT_STRING:
			switch(_dtype.type().type())
			{
				case Type::T_STRING: o.write1(0x0D); _dtype.print_value_bin(o); break;
				default: assert(false); // can't happen
			}
			break;
		case DataType::DT_PID:
			o.write1(0x59);
			break;
		case DataType::DT_GLOBAL:
			assert(_dtype.type().type()==Type::T_WORD);
			o.write1(0x4E);
			_dtype.print_value_bin(o); break;
			break;
		default: assert(false);
	}
}

