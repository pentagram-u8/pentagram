/*
 *	Folder.cpp - The core of the folding utility
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

#include "Folder.h"
#include "FuncNodes.h"

#include <deque>
using	std::deque;
#include <set>
using	std::set;

/****************************************************************************
	Unit
 ****************************************************************************/

void Unit::print_extern_unk(Console &o, const uint32 isize) const
{
	o.Print("// External Functions:\n");
	FOR_CONST_SET(DCCallNode, externFuncs, i)
	//for(std::set<DCCallNode *>::const_iterator i=externFuncs.begin(); i!=externFuncs.end(); ++i)
	{
		indent(o, isize+1);
		(*i)->print_extern_unk(o, isize+1);
		o.Putchar('\n');
	}
	o.Print("// External Intrinsics:\n");
	{
		FOR_CONST_SET(DCCallNode, externIntrinsics, i)
		//for(std::set<DCCallNode *>::const_iterator i=externIntrinsics.begin(); i!=externIntrinsics.end(); ++i)
		{
			indent(o, isize+1);
			(*i)->print_extern_unk(o, isize+1);
			o.Putchar('\n');
		}
	}
}

void Unit::print_unk(Console &o, const uint32 isize) const
{
	FOR_CONST_DEQUE(DCFuncNode, functions, i)
	{
		// function header
		(*i)->print_unk_funcheader(o, isize);

		// main function body
		indent(o, isize);
		o.Print("{\n");
		
		//indent(o, isize+1);
		(*i)->print_unk(o, isize+1);
		//o.Putchar('\n');
		indent(o, isize);
		o.Print("}\n");
	}
}

void Unit::print_asm(Console &o) const
{
	FOR_CONST_DEQUE(DCFuncNode, functions, i)
	{
		(*i)->print_asm(o);
		o.Putchar('\n');
	}
}

void Unit::print_bin(ODequeDataSource &o) const
{
	FOR_CONST_DEQUE(DCFuncNode, functions, i)
	{
		o.clear();
		(*i)->print_mac(con);
		(*i)->print_bin(o);
		// FIXME: The following is a bit of a hack, just so we get some 'real' output
		for(std::deque<char>::const_iterator i=o.buf().begin(); i!=o.buf().end(); ++i)
			con.Printf("%02X ", static_cast<uint8>(*i));
		con.Putchar('\n');
	}
}

/****************************************************************************
	DCUnit
 ****************************************************************************/
#define UNITDEBUG
/* returns true if we've finished folding the current function */
const bool DCUnit::fold(Node *n)
{
	if(n==0)
	{
		print_asm(con);
		print_unk(con, 0);
		ODequeDataSource o;
		//print_bin(o);
		print_extern_unk(con, 0);

		con.Print("assert failed: n!=0\n");
		print_assert(0, this);
		exit(-1);
	}
	// DEBUGGING. Will produce _lots_ of output.
	//print_asm(con);
	//print_assert(n, this);
	//con.Printf("currop.offset: %04X\tifstack.size: %d\telsestack.size: %d\n", n->offset(), ifstack.size(), elsestack.size());
	
	while(elsestack.size()>0 && n->offset()==elsestack.back()->TargetOffset())
	{
		IfNode *last = elsestack.back();
		elsestack.pop_back();
		#ifdef UNITDEBUG
		con.Printf("Popping elsestack offset: %04X type %d at %04X\n", last->offset(), last->itype, n->offset());
		#endif

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
			#ifdef UNITDEBUG
			print_assert(0, this);
			#endif
		}
		//if(!(ifstack.size()>0)) { last->print_asm(con); last->print_unk(con, 3); n->print_unk(con, 5); print_assert(n, this); }//debugging, DELETE:
		//assert(ifstack.size()>0 || print_assert(n, this));
		if(ifstack.size()>0 && last!=ifstack.back() && last->elsenode==0)
		{
			IfNode *newnode = new IfNode(IfNode::I_ELSE, last->TargetOffset());
			//con.Printf(">Fnord<\n");
			//print_assert(ifstack.back(), this);
			//con.Printf(">Fnord<\n");
			newnode->fold_else(this, ifstack.back()->nodes());
			assert(last->elsenode==0 || print_assert(last->elsenode, this));
			assert(ifstack.back()->opcode()==0x51);
			ifstack.back()->nodes().push_back(newnode);
		}
		else if(nodes.size()>0 &&
				last!=nodes.back() &&
				last->elsenode==0 //&&
				//nodes.back()->opcode()!=0x77 &&
				//nodes.back()->opcode()!=0x78 // 77 and 78 are special.
				)
							// Not sure why they're not being stripped before getting here though...
		{
			IfNode *newnode = new IfNode(IfNode::I_ELSE, last->TargetOffset());
			newnode->fold_else(this, nodes);
			assert(last->elsenode==0 || print_assert(last->elsenode, this));
			assert(nodes.back()->opcode()==0x51);
			nodes.push_back(newnode);
		}
	}

	while(ifstack.size()>0 && n->offset()==ifstack.back()->TargetOffset())
	{
		IfNode *last = ifstack.back();
		ifstack.pop_back();
		#ifdef UNITDEBUG
		con.Printf("Popping ifstack offset: %04X type %d at %04X\n", last->offset(), last->itype, n->offset());
		#endif
		// fold the last if node once more, with the 'parent' stack,
		// just in case it wants to do anything funky (FIXME: Be just a _tad_ more specific here)
		if(ifstack.size()==0)
			last->fold(this, nodes);
		else
			last->fold(this, ifstack.back()->nodes());
		
		// if we're an else type, append us to the stack
		if(last->itype!=IfNode::I_IF && last->itype!=IfNode::I_ELSE_IF)
		{
			#if 0
			con.Printf("Fnord: %04X\n", last->offset());
			print_assert(last, this);
			#endif
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
	}
	
	// special handling for jmp...
	if(n->opcode()==0x51)
		setJump(static_cast<IfNode *>(n));
	
	// are we at the end of a function...
	if(n->opcode()==0x50)
	{
		// doesn't handle spawn inline, will have to handle it sometime
		// should just be a case of making sure to pass the appropriate
		// 'nodes' array to fold, like:
		/*if(ifstack.size()==0)
			if(n->fold(this, nodes))
				nodes.push_back(n);
		else
			if(n->fold(this, ifstack.back()->nodes()))
				ifstack.back()->nodes().push_back(n);
		*/
		
		assert(elsestack.size()==0 || print_assert(n, this));
		DCFuncNode *func = new DCFuncNode();
		assert(ifstack.size()==0);
		
		if(func->fold(this, nodes))
			nodes.push_back(func);
	}
	
	// special handling for 'end' opcodes
	if(n->opcode()==0x7A)
	{
		//con.Printf("<< Assign func %d\n", functions.size()+1);
		assert(nodes.back()->opcode()==0xFFFF);
		functions.push_back(static_cast<DCFuncNode *>(nodes.back()));
		nodes.pop_back();
		assert(nodes.size()==0);
		return true;
	}
	return false;
}

/****************************************************************************
	Folder
 ****************************************************************************/

void Folder::fold(Node *n)
{
	if(n!=0)
		con.Printf("\t%04X:\t%02X\n", n->offset(), n->opcode());
	else
		con.Printf("WARNING: Got a null node, so something's not right, will terminate soon...\n");

	//n->print_asm(con);
	if(curr->fold(n))
	{
		//assert(curr!=0);
		//units.push_back(curr);
		//curr = 0;
		
	}
}

void Folder::print_unk(Console &o) const
{
	con.Printf("Printing... %d\n", units.size());
	FOR_CONST_DEQUE(DCUnit, units, i)
	{
		(*i)->print_unk(o, 0);
	}
}

void Folder::print_asm(Console &o) const
{
	FOR_CONST_DEQUE(DCUnit, units, i)
	{
		(*i)->print_asm(o);
	}
}

void Folder::print_bin(ODequeDataSource &o) const
{
	FOR_CONST_DEQUE(DCUnit, units, i)
	{
		(*i)->print_bin(o);
	}
}

/****************************************************************************
	Useful Funcs
 ****************************************************************************/

bool print_assert_nodes(std::deque<Node *> &nodes, uint32 index)
{
	indent(con, index);
	con.Printf("Nodes:");
	FOR_CONST_DEQUE(Node, nodes, i)
	{
		con.Putchar('\n');
		indent(con, index+1);
		con.Printf("%04X: %02X", (*i)->offset(), (*i)->opcode());
	}
	if(nodes.size()) con.Printf("  <-\n");
	con.Putchar('\n');
	return false;
}

bool print_assert(const Node *n, const DCUnit *u)
{
	if(n!=0)
	{
		con.Printf("\n========================================\n");
		con.Printf("  Error with opcode %02X at offset %04X.\n", n->opcode(), n->offset());
		con.Printf("========================================\n");
		//n->print_unk(con, 0);
	}

	if(u!=0)
	{
		con.Printf("Num functions parsed: %d\n", u->functions.size());
		//for(std::deque<DCFuncNode *>::const_iterator i=u->functions.begin(); i!=u->functions.end(); ++i)
		//{
		//	(*i)->print_unk(con, 1);
		//}
		u->print_asm(con);
		u->print_unk(con, 0);

		con.Printf("IfStack:");
		{
			FOR_CONST_DEQUE(IfNode, u->ifstack, i)
			{
				con.Printf("\n    %04X: %02X -> %04X", (*i)->offset(), (*i)->opcode(), (*i)->TargetOffset());
				FOR_CONST_DEQUE(Node, (*i)->nodes(), j)
				{
					con.Printf("\n        %04X: %02X", (*j)->offset(), (*j)->opcode());
				}
			}
		}
		if(u->ifstack.size()) con.Printf("  <-");
		con.Putchar('\n');
		
		con.Printf("ElseStack:");
		{
			FOR_CONST_DEQUE(IfNode, u->elsestack, i)
			{
				con.Printf("\n    %04X: %02X -> %04X", (*i)->offset(), (*i)->opcode(), (*i)->TargetOffset());
				FOR_CONST_DEQUE(Node, (*i)->nodes(), j)
				{
					con.Printf("\n        %04X: %02X", (*j)->offset(), (*j)->opcode());
				}
			}
		}
		if(u->elsestack.size()) con.Printf("  <-");
		con.Putchar('\n');

		con.Printf("Nodes:");
		{
			FOR_CONST_DEQUE(Node, u->nodes, i)
			{
				con.Printf("\n    %04X: %02X", (*i)->offset(), (*i)->opcode());
			}
		}
		if(u->nodes.size()) con.Printf("  <-\n");
		con.Putchar('\n');

		/*con.Printf("FuncStack:");
		for(deque<DCUnit *>::const_iterator k=units.begin(); k!=units.end(); ++k)
		{
			con.Printf("\n    %04X: %02X", (*k)->offset(), (*k)->opcode());
			if((*k)->opcode()==0x51)
			{
			}
		}*/
	}
	
	return false;
}

