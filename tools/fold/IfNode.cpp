/*
 *	IfNode.cpp -
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

#include "IfNode.h"
#include "OperatorNodes.h"
#include "Folder.h"

#include <deque>
using	std::deque;

//#define DEBUG_JUNK

/****************************************************************************
	IfNode
 ****************************************************************************/

void IfNode::print_unk(Console &o, const uint32 isize) const
{
	assert(rtype().type()==Type::T_INVALID);
	
	// in case we need to print 'else if(...' instead...
	switch(itype)
	{
		case I_IF:
		case I_IF_ELSE:
		case I_IF_ELSE_IF:
			break;
		case I_ELSE_IF:
		case I_ELSE_IF_ELSE:
		case I_ELSE:
			o.Print("else ");
			break;
		default: assert(false);
	}

	switch(itype)
	{
		case I_IF:
		case I_IF_ELSE:
		case I_IF_ELSE_IF:
		case I_ELSE_IF:
		case I_ELSE_IF_ELSE:
			assert(node!=0);
			// print the if(...) { header
			o.Print("if(");
			// add an extra 'not' since the things inverted.
			// FIXME: at some point in time we need to strip the extra not node,
			// if there is one, rather then just dropping an extra 'not ' on the front
			o.Print("not ");
			
			node->print_unk(o, isize);
			o.Print(")");
			break;
		case I_ELSE: // print nuthin'
			break;
		default: assert(false);
	}
	o.Print("\n");
	indent(o, isize);
	o.Print("{");
	Node::print_linenum_unk(o, isize);
	#ifdef DEBUG_JUNK
	if(itype!=I_ELSE)
		o.Printf(" //jne_NOPRINT(0x%04X)", targetOffset);
	#endif
	o.Print("\n");
	// print the internal operations
	for(std::deque<Node *>::const_iterator i=ifnodes.begin(); i!=ifnodes.end(); ++i)
	{
		indent(o, isize+1);
		(*i)->print_unk(o, isize+1);
		o.Putchar('\n');
	}
	// print the terminating structure '}\n' or '} else'
	indent(o, isize);
	o.Putchar('}');
	#ifdef DEBUG_JUNK
	o.Printf("/*%02X*/", itype);
	#endif
	switch(itype)
	{
		case I_IF:
		case I_ELSE_IF:
			assert(jmpnode==0);
			break;
		case I_IF_ELSE:
		case I_ELSE_IF_ELSE:
		case I_IF_ELSE_IF:
			assert(jmpnode!=0);
			#ifdef DEBUG_JUNK
			jmpnode->print_unk(o, isize);
			#endif
			//o.Print(" else ");
			#ifdef DEBUG_JUNK
			if(elsenode!=0) o.Print("/*(elsenode)*/");
			#endif
			/*assert(elsenode!=0);
			if(elsenode)
			{
				o.Putchar('\n');
				indent(o, isize);
				elsenode->print_unk(o, isize);
			}*/
			break;
		case I_ELSE: // again, nothing special
			break;
		default: assert(false); // can't happen
	}
}

void IfNode::print_asm(Console &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	switch(itype)
	{
		case I_IF:
		case I_IF_ELSE:
		case I_IF_ELSE_IF:
		case I_ELSE_IF:
		case I_ELSE_IF_ELSE:
			{
				Node::print_linenum_asm(o);
				assert(node!=0);
				node->print_asm(o);
				o.Putchar('\n');
				Node::print_asm(o);
				o.Printf("jne\t\t%04Xh\t(to %04X)", targetOffset - _offset - 3, targetOffset);
				for(std::deque<Node *>::const_iterator i=ifnodes.begin(); i!=ifnodes.end(); ++i)
				{
					o.Putchar('\n');
					(*i)->print_asm(o);
				}
				if(jmpnode!=0)
				{
					o.Putchar('\n');
					jmpnode->print_asm(o);
				}
				break;
			}
		case I_ELSE:
			{
				for(std::deque<Node *>::const_iterator i=ifnodes.begin(); i!=ifnodes.end(); ++i)
				{
					if(i!=ifnodes.begin()) o.Putchar('\n');
					(*i)->print_asm(o);
				}
				break;
			}
		default: assert(false); // can't happen
	}
}

void IfNode::print_bin(ODequeDataSource &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_bin(o);
	switch(itype)
	{
		case I_IF:
		case I_IF_ELSE:
		case I_IF_ELSE_IF:
		case I_ELSE_IF:
		case I_ELSE_IF_ELSE:
			{
				assert(node!=0);
				node->print_bin(o);
				o.write1(0x51);
				o.write2(targetOffset - _offset - 3);
				if(jmpnode!=0)
					jmpnode->print_bin(o);
				for(std::deque<Node *>::const_iterator i=ifnodes.begin(); i!=ifnodes.end(); ++i)
				{
					(*i)->print_bin(o);
				}
				break;
			}
		default: assert(false); // can't happen
	}
}

bool IfNode::fold(DCUnit *unit, std::deque<Node *> &nodes)
{
	//print_assert(this, unit);
	//con.Printf("\n>>> Nodes: %d <<<\n", nodes.size());
	//print_assert(nodes.back(), unit);

	switch(itype)
	{
		// "If I were an ifnode, lalala lalalaLAlaLAlala. I would be a very iffy ifnode..." *ahem*
		case I_IF:
		case I_ELSE_IF:
			// if this is the first time we do this, the only thing we grab
			// is the parameters of the if.
			if(node==0)
			{
				assert(nodes.size()>0);
				grab_n(nodes);
				fold_linenum(nodes);

				// test if we need to alter the state of our previous ifnode, if there's one there.
				if(/*(itype!=I_ELSE_IF) &&*/ nodes.size()>0 && nodes.back()->opcode()==0x51)
				{
					// Bad Darke! Time to get out the whips and paddles...
					IfNode *n=static_cast<IfNode *>(nodes.back());
					// a hack, let's see if it works...
					//nodes.pop_back();
					//unit->setJump(n);
					
					assert(n!=this);
					switch(n->itype)
					{
						case I_IF:           n->itype=I_IF_ELSE;      break;
						case I_IF_ELSE:      n->itype=I_IF_ELSE_IF;   break;
						case I_ELSE_IF_ELSE: /* leave as is */        break;
						//case I_IF_ELSE_IF:   n->itype=I_ELSE_IF_ELSE; break;
						default: assert(print_assert(this, unit));
					}
					switch(itype)
					{
						case I_IF: itype=I_ELSE_IF; break;
						default: assert(print_assert(this, unit));
					}
				}
			}
			// handle the pessimal if(true){code}->if(false){}else{code} case
			// we've also got to handle the... 'unique' problem that if we get a cmp
			// node after our jne node, our jmp node will be 'inside' us.
			// the simplest solution then, is to have the jmp append to our 'nodes',
			// then have us be called again through fold() by the loop in Unit::fold()
			// as a special case
			else if(jmpnode==0 && ifnodes.size()>0 && ifnodes.back()->opcode()==0x52)
			{
				jmpnode=static_cast<EndNode *>(ifnodes.back());
				ifnodes.pop_back();
				switch(itype)
				{
					case I_IF:      itype = I_IF_ELSE;      break;
					case I_ELSE_IF: itype = I_ELSE_IF_ELSE; break;
					default: assert(print_assert(this, unit));
				}
			}
			//else
			//	assert(print_assert(this, unit));
			break;
			break;
		case I_IF_ELSE:
		case I_IF_ELSE_IF:
		case I_ELSE_IF_ELSE:
			break;
		default: assert(print_assert(this, unit));
	}
	
	return true;
}

bool IfNode::fold_else(DCUnit *unit, std::deque<Node *> &nodes)
{
	switch(itype)
	{
		case I_ELSE:
			if(ifnodes.size()==0)
			{
				bool finished=false;
				while(!finished)
				{
					assert(nodes.size()>0 || print_assert(this, unit));
					if(nodes.back()->opcode()==0x51 || nodes.back()->opcode()==0x52)
						if(static_cast<IfNode *>(nodes.back())->TargetOffset()==TargetOffset())
							finished=true;

					if(!finished)
					{
						ifnodes.push_front(nodes.back());
						nodes.pop_back();
					}
					if(nodes.size()==0)
						finished = true; // test!
				}
			}
			break;
		default: assert(print_assert(this, unit));
	}
	return false; // can't happen
}

/****************************************************************************
	EndNode
 ****************************************************************************/

void EndNode::print_unk(Console &o, const uint32 isize) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_unk(o, isize);
	switch(etype)
	{
		case JMP:
			o.Printf("/*jmp_NOPRINT(0x%04X)*/", targetOffset);
			break;
		default: assert(false); // can't happen
	}                                
}

void EndNode::print_asm(Console &o) const   
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_asm(o);
	switch(etype)                                 
	{
		case JMP:
			Node::print_asm(o);
			o.Printf("jmp\t\t%04Xh\t(to %04X)", targetOffset - _offset - 3, targetOffset);
			break;
		default: assert(false); // can't happen
	}
}

void EndNode::print_bin(ODequeDataSource &o) const
{
	assert(rtype().type()==Type::T_INVALID);
	Node::print_linenum_bin(o);
	switch(etype)
	{
		case JMP:
			o.write1(0x52);
			o.write2(targetOffset - _offset - 3);
			break;
		default: assert(false); // can't happen
	}
}

bool EndNode::fold(DCUnit * /*unit*/, std::deque<Node *> &nodes)
{
	fold_linenum(nodes);

	return true;
}

