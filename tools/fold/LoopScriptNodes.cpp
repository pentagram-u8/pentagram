/*
 *	LoopScriptNodes.cpp -
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

#include "LoopScriptNodes.h"

#include <map>
#include <string>
extern std::map<sint32, std::string> ScriptExpressions;

/****************************************************************************
	LoopScriptNode
 ****************************************************************************/
 
void LoopScriptNode::print_unk(Console &o, const uint32 isize) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_unk(o, isize);
	o.Printf("loopscr(%c)", static_cast<char>(scriptTok));
}

void LoopScriptNode::print_asm(Console &o) const   
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_asm(o);
	o.Printf("loopscr\t\t%02X \"%c\" - %s", scriptTok, static_cast<char>(scriptTok), ScriptExpressions[scriptTok].c_str());
}

void LoopScriptNode::print_bin(ODequeDataSource &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_bin(o);
	o.write1(0x74);
	o.write1(scriptTok);
}

bool LoopScriptNode::fold(DCUnit * /*unit*/, std::deque<Node *> &nodes)
{
	fold_linenum(nodes); // TODO: maybe? Might not get line numbers attached since it's rather special...

	return true;
}

/****************************************************************************
	LoopNode
 ****************************************************************************/
 
bool LoopNode::fold(DCUnit * /*unit*/, std::deque<Node *> &nodes)
{
		// we need to remove ourselves from the stack before doing anything tricky
		//assert(acceptOp(nodes.back()->opcode(), 0x70) || print_assert(this, unit));
		//Node *us=nodes.back();
		//nodes.pop_back();
		
		// grab the nodes, removing them all until we get to a $ 'end' node
		assert(nodes.size()>0);
		while(nodes.size()>0 && (acceptOp(nodes.back()->opcode(), 0x74) && static_cast<LoopScriptNode*>(nodes.back())->lsTok()=='$'))
		{
			pnode.push_back(grab(nodes));
		}
		//grab_p(nodes, std::abs(static_cast<sint32>(addSP->size())));
		
		// add us back to the stack
		//nodes.push_back(us);
	
	// potential line numbers come 'before' the pushed values, so we need to grab
	// them after the values
	fold_linenum(nodes);
	
	return false;
}

void LoopNode::print_unk(Console &o, const uint32 isize) const
{
	print_linenum_unk(o, isize);
	o.Printf("search_%s_type_%02X(", suc::print_bp(currObj), searchType);
	for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
	{
		if(i!=pnode.rbegin()) o.Print(", ");
		(*i)->print_unk(o, isize);
	}
	o.Putchar(')');
}

void LoopNode::print_asm(Console &o) const
{
	print_linenum_asm(o);
	for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
	{
		(*i)->print_asm(o); o.Putchar('\n');
	}
	Node::print_asm(o);
	o.Printf("loop\t\t%s %02X %02X", suc::print_bp(currObj), strSize, searchType);
}

void LoopNode::print_bin(ODequeDataSource &o) const
{
	print_linenum_bin(o);
	for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
	{
		(*i)->print_bin(o);
	}
	o.write1(0x70);
	o.write1(currObj);
	o.write1(strSize);
	o.write1(searchType);
}

/****************************************************************************
	LoopNextNode
 ****************************************************************************/

void LoopNextNode::print_unk(Console &o, const uint32 isize) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_unk(o, isize);
	o.Printf("/*loopnext_NOPRINT()*/");
}

void LoopNextNode::print_asm(Console &o) const   
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_asm(o);
	o.Printf("loopnext");
}

void LoopNextNode::print_bin(ODequeDataSource &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_bin(o);
	o.write1(0x73);
}

bool LoopNextNode::fold(DCUnit * /*unit*/, std::deque<Node *> & /*nodes*/)
{
	//fold_linenum(nodes); // should never be a linenum before this

	return true;
}

