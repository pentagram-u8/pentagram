/*
 *	Folder.cpp - The core of the folding utility
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

#include "Folder.h"

#include <deque>
using	std::deque;
#include <set>
using	std::set;

/****************************************************************************
	Unit
 ****************************************************************************/
 
void Unit::fold(Node *n)
{
	if(n==0)
	{
		print_asm(con);
		print_unk(con, 0);
		OBufferDataSource o;
		//print_bin(o);
		print_extern_unk(con, 0);

		con.Print("assert failed: n!=0\n");
		print_assert(0, this);
		exit(-1);
	}
	// DEBUGGING. Will produce _lots_ of output.
	//print_asm(con);
	//print_assert(n, this);
	//con.Printf("ifstack.size: %d\n", ifstack.size());
	
	while(elsestack.size()>0 && n->offset()==elsestack.back()->TargetOffset())
	{
		//print_assert(0, this);

		IfNode *last = elsestack.back();
		elsestack.pop_back();

		// bunch of random asserts
		assert(last->itype!=IfNode::I_IF && last->itype!=IfNode::I_ELSE_IF);
		if(elsestack.size()==0)
			assert(last->itype==IfNode::I_IF_ELSE || last->itype==IfNode::I_IF_ELSE_IF);
		if(elsestack.size()>0 && (last->itype==IfNode::I_ELSE_IF || last->itype==IfNode::I_ELSE_IF_ELSE))
			assert(last->TargetOffset()==elsestack.back()->TargetOffset());
		if(elsestack.size()>0)
		{
			assert(elsestack.back()->elsenode==0 || print_assert(elsestack.back(), this));
			elsestack.back()->elsenode=last;
			print_assert(0, this);
		}

		assert(ifstack.size()>0 || print_assert(0, this));
		if(last!=ifstack.back() && last->elsenode==0)
		{
			IfNode *newnode = new IfNode(IfNode::I_ELSE, last->TargetOffset());
			newnode->fold(this, ifstack.back()->nodes());
			con.Print("\n>>>Mu:\n");
			print_assert(last, this);
			assert(last->elsenode==0 || print_assert(last->elsenode, this));
			//last->elsenode = newnode;
			assert(ifstack.back()->opcode()==0x51);
			ifstack.back()->nodes().push_back(newnode);
			//ifstack.push_back(newnode);
		}
	}

	while(ifstack.size()>0 && n->offset()==ifstack.back()->TargetOffset())
	{
		IfNode *last = ifstack.back();
		ifstack.pop_back();
		con.Printf("Popping offset: %04X at %04X\n", last->offset(), n->offset());
		// fold the last if node once more, with the 'parent' stack,
		// just in case it wants to do anything funky (FIXME: Be just a _tad_ more specific here)
		if(ifstack.size()==0)
			last->fold(this, nodes);
		else
			last->fold(this, ifstack.back()->nodes());
		
		// if we're an else type, append us to the stack
		if(last->itype!=IfNode::I_IF && last->itype!=IfNode::I_ELSE_IF)
		{
			if(elsestack.size()>0 && (elsestack.back()->itype==IfNode::I_IF_ELSE
				|| elsestack.back()->itype==IfNode::I_ELSE_IF_ELSE))
			{
				elsestack.back()->elsenode=last;
				elsestack.pop_back();
			}
			elsestack.push_back(last);
		}
	}
	
	// if we're not nested inside an if, things are relatively simple
	if(ifstack.size()==0)
	{
		if(n->fold(this, nodes)) // we want to fold it
			nodes.push_back(n); // and _then_ append it to the end of the 'stack' if we're supposed to.
	}
	else
	{
		if(n->fold(this, ifstack.back()->nodes()))
			ifstack.back()->nodes().push_back(n);
		// if we're a jump opcode we have to do Special Stuff(tm). (see comment in IfNode::fold()
		/*END if(n->opcode()==0x52)
		{
			con.Printf("\n>>> FOO! <<<\n");
			assert(ifstack.back()->nodes.size()>0);
			ifstack.back()->fold(this, ifstack.back()->nodes);
		}*/
	}
	if(n->opcode()==0x51)
		setJump(dynamic_cast<IfNode *>(n));
}

void Unit::print_extern_unk(Console &o, const uint32 isize) const
{
	o.Print("// External Functions:\n");
	for(set<CallNode *>::const_iterator i=externFuncs.begin(); i!=externFuncs.end(); ++i)
	{
		indent(o, isize+1);
		(*i)->print_extern_unk(o, isize+1);
		o.Putchar('\n');
	}
	o.Print("// External Intrinsics:\n");
	for(set<CallNode *>::const_iterator i=externIntrinsics.begin(); i!=externIntrinsics.end(); ++i)
	{
		indent(o, isize+1);
		(*i)->print_extern_unk(o, isize+1);
		o.Putchar('\n');
	}
}

void Unit::print_unk(Console &o, const uint32 isize) const
{
	for(deque<Node *>::const_iterator i=nodes.begin(); i!=nodes.end(); ++i)
	{
		indent(o, isize+1);
		(*i)->print_unk(o, isize+1);
		o.Putchar('\n');
	}
}

void Unit::print_asm(Console &o) const
{
	for(deque<Node *>::const_iterator i=nodes.begin(); i!=nodes.end(); ++i)
	{
		(*i)->print_asm(o);
		o.Putchar('\n');
	}
}

void Unit::print_bin(OBufferDataSource &o) const
{
	for(deque<Node *>::const_iterator i=nodes.begin(); i!=nodes.end(); ++i)
	{
		o.clear();
		(*i)->print_mac(con);
		(*i)->print_bin(o);
		// FIXME: The following is a bit of a hack, just so we get some 'real' output
		for(deque<char>::const_iterator i=o.buf().begin(); i!=o.buf().end(); ++i)
			con.Printf("%02X ", static_cast<uint8>(*i));
		con.Putchar('\n');
	}
}

/****************************************************************************
	Folder
 ****************************************************************************/

void Folder::fold(Node *n)
{
	curr->fold(n);
}

void Folder::print_unk(Console &o) const
{
	for(deque<Unit *>::const_iterator i=units.begin(); i!=units.end(); ++i)
	{
		(*i)->print_unk(o, 0);
	}
}

void Folder::print_asm(Console &o) const
{
	for(deque<Unit *>::const_iterator i=units.begin(); i!=units.end(); ++i)
	{
		(*i)->print_asm(o);
	}
}

void Folder::print_bin(OBufferDataSource &o) const
{
	for(deque<Unit *>::const_iterator i=units.begin(); i!=units.end(); ++i)
	{
		(*i)->print_bin(o);
	}
}

/****************************************************************************
	Useful Funcs
 ****************************************************************************/

bool print_assert(const Node *n, const Unit *u)
{
	if(n!=0)
	{
		con.Printf("\n========================================\n");
		con.Printf("  Error with opcode %02X at offset %04X.\n", n->opcode(), n->offset());
		con.Printf("========================================\n");
	}
	
	if(u!=0)
	{
		con.Printf("IfStack:");
		for(deque<IfNode *>::const_iterator i=u->ifstack.begin(); i!=u->ifstack.end(); ++i)
		{
			con.Printf("\n    %04X: %02X -> %04X", (*i)->offset(), (*i)->opcode(), (*i)->TargetOffset());
		}
		if(u->ifstack.size()) con.Printf("  <- head");
		con.Putchar('\n');
		
		con.Printf("ElseStack:");
		for(deque<IfNode *>::const_iterator i=u->elsestack.begin(); i!=u->elsestack.end(); ++i)
		{
			con.Printf("\n    %04X: %02X -> %04X", (*i)->offset(), (*i)->opcode(), (*i)->TargetOffset());
		}
		if(u->elsestack.size()) con.Printf("  <- head\n");
		con.Putchar('\n');

		con.Printf("Nodes:");
		for(deque<Node *>::const_iterator i=u->nodes.begin(); i!=u->nodes.end(); ++i)
		{
			con.Printf("\n    %04X: %02X", (*i)->offset(), (*i)->opcode());
		}
		if(u->nodes.size()) con.Printf("  <- head\n");
		con.Putchar('\n');

		/*con.Printf("FuncStack:");
		for(deque<Unit *>::const_iterator k=units.begin(); k!=units.end(); ++k)
		{
			con.Printf("\n    %04X: %02X", (*k)->offset(), (*k)->opcode());
			if((*k)->opcode()==0x51)
			{
			}
		}*/
	}
	
	return false;
}

