/*
 *	FuncNodes.cpp -
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

#include "FuncNodes.h"
#include "Folder.h"

/****************************************************************************
	FuncMutatorNode
 ****************************************************************************/

void FuncMutatorNode::print_unk(Console &o, const uint32 isize) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_unk(o, isize);
	switch(mtype)
	{
		case RET:         o.Printf("ret_NOPRINT()"); break;
		case INIT:        o.Printf("init_NOPRINT(0x%02X)", _initsize); break;
		case LINE_NUMBER:
			#if 0
			o.Printf(" /* Line No: %d */ ", _linenum);
			#else
			o.Printf("/*%d*/", _linenum);
			#endif
			break;
		case SYMBOL_INFO: o.Printf("symbol_info_NOPRINT(0x%04X, \"%s\")", _symboloffset, _classname.c_str()); break;
		default: assert(print_assert(this)); // can't happen
	}
}

void FuncMutatorNode::print_asm(Console &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_asm(o);
	Node::print_asm(o);
	switch(mtype)
	{
		case RET:         o.Printf("ret"); break;
		case INIT:        o.Printf("init\t\t%02X", _initsize); break;
		case LINE_NUMBER: o.Printf("line number\t%i (%04Xh)", _linenum, _linenum); break;
		case SYMBOL_INFO: o.Printf("symbol info\toffset %04Xh = \"%s\"", _symboloffset, _classname.c_str()); break;
		default: assert(print_assert(this)); // can't happen
	}
}

void FuncMutatorNode::print_bin(OBufferDataSource &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_bin(o);
	switch(mtype)
	{
		case RET:         o.write1(0x50); break;
		case INIT:        o.write1(0x5A); o.write1(_initsize); break;
		case LINE_NUMBER: o.write1(0x5B); o.write2(_linenum); break;
		case SYMBOL_INFO: o.write1(0x5C); o.write2(_symboloffset - _offset - 3); o.write(_classname.c_str(), _classname.size(), 9); break;
		default: assert(print_assert(this)); // can't happen
	}
}

bool FuncMutatorNode::fold(Unit *unit, std::deque<Node *> &nodes)
{
	fold_linenum(nodes);
	
	if(mtype==SYMBOL_INFO)
	{
		unit->setDebugOffset(_symboloffset);
		unit->setClassName(_classname);
	}
	
	return true;
}

/****************************************************************************
	FuncNode
 ****************************************************************************/

void FuncNode::print_unk(Console &o, const uint32 /*isize*/) const
{
	assert(rtype().type()==Type::T_INVALID);
	o.Printf("end_NOPRINT()");
}

void FuncNode::print_asm(Console &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_asm(o);
	o.Printf("end");
}

void FuncNode::print_bin(OBufferDataSource &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	o.write1(0x7A);
}

bool FuncNode::fold(Unit */*unit*/, std::deque<Node *> &/*nodes*/)
{
	return true;
}

