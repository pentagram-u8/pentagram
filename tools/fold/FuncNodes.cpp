/*
 *	FuncNodes.cpp -
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

#include "FuncNodes.h"
#include "Folder.h"

#include <deque>
using	std::deque;

/****************************************************************************
	FuncMutatorNode
 ****************************************************************************/

void FuncMutatorNode::print_unk(Console &o, const uint32 isize, const bool comment) const
{
	/* if we're in a comment, then we're actually going to output, else we silently
		do nothing. This is to handle the cases where we don't want this data
		such as in non-debugging situations */
	if(!comment && mtype!=SUSPEND) return;
	
	assert(rtype().type()==Type::T_INVALID);
	switch(mtype)
	{
		case RET:
			Node::print_linenum_unk(o, isize);
			o.Printf("ret_NOPRINT()");
			break;
		case INIT:
			Node::print_linenum_unk(o, isize);
			o.Printf("init_NOPRINT(0x%02X)", _initsize);
			break;
		case LINE_NUMBER:
			Node::print_linenum_unk(o, isize);
			#if 0
			o.Printf(" /* Line No: %d */ ", _linenum);
			#else
			o.Printf("/*%d*/", _linenum);
			#endif
			break;
		case SYMBOL_INFO:
			Node::print_linenum_unk(o, isize);
			o.Printf("symbol_info_NOPRINT(0x%04X, \"%s\")", _symboloffset, _classname.c_str());
			break;
		case SUSPEND:
			o.Printf("suspend");
			break;
		case END:
			Node::print_linenum_unk(o, isize);
			o.Printf("end_NOPRINT()");
			break;
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
		case SUSPEND:     o.Printf("suspend"); break;
		case END:         o.Printf("end"); break;
		default: assert(print_assert(this)); // can't happen
	}
}

void FuncMutatorNode::print_bin(ODequeDataSource &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_bin(o);
	switch(mtype)
	{
		case RET:         o.write1(0x50); break;
		case INIT:        o.write1(0x5A); o.write1(_initsize); break;
		case LINE_NUMBER: o.write1(0x5B); o.write2(_linenum); break;
		case SYMBOL_INFO: o.write1(0x5C); o.write2(_symboloffset - _offset - 3); o.write(_classname.c_str(), _classname.size(), 9); break;
		case SUSPEND:     o.write1(0x53); break;
		case END:         o.write1(0x7A); break;
		default: assert(print_assert(this)); // can't happen
	}
}

bool FuncMutatorNode::fold(DCUnit *unit, std::deque<Node *> &nodes)
{
	fold_linenum(nodes);
	
	if(mtype==SYMBOL_INFO)
	{
		unit->setDebugOffset(_symboloffset);
		unit->setClassName(_classname);
	}
	else if(mtype==END)
	{
		// got to do a bit of juggling...
		// ... remove ourselves from the back...
		//assert((nodes.size()>0 && nodes.back()->opcode()==0x7A) || print_assert(this, unit));
		//Node *n = nodes.back();
		//nodes.pop_back();
		//assert(n==this);
		// ... grab the function node...
		assert((nodes.size()>0 && nodes.back()->opcode()==0xFFFF) || print_assert(this, unit));
		DCFuncNode *funcnode=static_cast<DCFuncNode *>(nodes.back());
		funcnode->addEnd(this);
		// ... add ourselves...
		//nodes.push_back(this);
		// ... call the func node's fold with the appriprate parameters...
		//funcnode->fold(unit, nodes);
		return false;
	}
	
	return true;
}

/****************************************************************************
	DCFuncNode
 ****************************************************************************/
//#define DEBUG_COMMENTS

void DCFuncNode::print_unk(Console &o, const uint32 isize) const
{
	#ifdef DEBUG_COMMENTS
	indent(o, isize); o.Print("/*"); // prelude

	indent(o, 1);
	initnode->print_unk(o, isize+1, true);
	o.Putchar('\n');
	
	indent(o, isize+1);
	setinfonode->print_unk(o, isize+1, true);
	
	indent(o, 1); o.Print("*/\n"); // postfix
	#endif
	
	indent(o, isize);
	procexcludenode->print_unk(o, isize);
	o.Putchar('\n');

	for(std::deque<Node *>::const_iterator i=funcnodes.begin(); i!=funcnodes.end(); ++i)
	{
		indent(o, isize);
		(*i)->print_unk(o, isize);
		o.Putchar('\n');
	}

	#ifdef DEBUG_COMMENTS
	indent(o, isize); o.Print("/*"); // prelude
	
	indent(o, 1);
	retnode->print_unk(o, isize+1, true);
	o.Putchar('\n');
	
	indent(o, isize+1);
	endnode->print_unk(o, isize+1, true);
	
	indent(o, 1); o.Print("*/\n"); // postfix
	#endif
}

void DCFuncNode::print_asm(Console &o) const
{
	initnode->print_asm(o);
	o.Putchar('\n');
	setinfonode->print_asm(o);
	o.Putchar('\n');
	procexcludenode->print_asm(o);
	o.Putchar('\n');
	
	for(std::deque<Node *>::const_iterator i=funcnodes.begin(); i!=funcnodes.end(); ++i)
	{
		(*i)->print_asm(o);
		o.Putchar('\n');
	}
	retnode->print_asm(o);
	o.Putchar('\n');
	endnode->print_asm(o);
	o.Putchar('\n');
}

void DCFuncNode::print_bin(ODequeDataSource &o) const
{
	assert(initnode!=0);
	initnode->print_mac(con);
	initnode->print_bin(o);
	
	assert(setinfonode!=0);
	setinfonode->print_mac(con);
	setinfonode->print_bin(o);
	
	assert(procexcludenode!=0);
	procexcludenode->print_mac(con);
	procexcludenode->print_bin(o);
	
	for(std::deque<Node *>::const_iterator i=funcnodes.begin(); i!=funcnodes.end(); ++i)
	{
		o.clear();
		(*i)->print_mac(con);
		(*i)->print_bin(o);
		// FIXME: The following is a bit of a hack, just so we get some 'real' output
		for(std::deque<char>::const_iterator i=o.buf().begin(); i!=o.buf().end(); ++i)
			con.Printf("%02X ", static_cast<uint8>(*i));
		con.Putchar('\n');
	}
	
	assert(retnode!=0);
	retnode->print_mac(con);
	retnode->print_bin(o);

	assert(endnode!=0);
	endnode->print_mac(con);
	endnode->print_bin(o);
}

bool DCFuncNode::fold(DCUnit *unit, std::deque<Node *> &nodes)
{
	assert(nodes.size()>0);
	
	// we get our 'end' later...
	
	// ... get our 'ret'
	assert(nodes.size() && nodes.back()->opcode()==0x50);
	retnode = static_cast<FuncMutatorNode *>(nodes.back());
	nodes.pop_back();
	
	// while we haven't gotten init
	bool read_last=false;
	while(nodes.size() && !acceptOp(nodes.back()->opcode(), 0x5A, 0x78))
	{
		/*switch(nodes.back()->opcode())
		{
			case 0x5A:
				assert(initnode==0);
				initnode = nodes.back();
				read_last=true;
				break;
			default:*/
				funcnodes.push_front(nodes.back());
		//}
		nodes.pop_back();
	}

	// ... and our 'process exclude'
	assert(nodes.size() && nodes.back()->opcode()==0x78);
	procexcludenode = static_cast<DCCallMutatorNode *>(nodes.back());
	nodes.pop_back();

	// ... and our 'set info'
	assert(nodes.size() && nodes.back()->opcode()==0x77);
	setinfonode = static_cast<DCCallMutatorNode *>(nodes.back());
	nodes.pop_back();

	// ... and our 'init'
	assert(nodes.size() && nodes.back()->opcode()==0x5A);
	initnode = static_cast<FuncMutatorNode *>(nodes.back());
	nodes.pop_back();
	parameters_datasize = initnode->a_initsize();
	func_start_offset = initnode->offset();

	// FIXME: This will obviously be false when we're finally implementing
	// inline functions.
	assert((nodes.size()==0) || print_assert(0, unit));
	
	return true;
}

