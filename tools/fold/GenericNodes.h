/*
 *	GenericNodes.h - The base node types from which all other nodes are derived.
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

#ifndef GENERICNODES_H
#define GENERICNODES_H

#include "Type.h"

#include <deque>
#include <vector>
#include "Console.h"

// 'cause I'm sick of this ugly construct overrunning everything...
#define FOR_CONST_DEQUE(CLAS, DEQ, I) for(std::deque<CLAS *>::const_iterator I=(DEQ).begin(); (I)!=DEQ.end(); ++(I))
#define FOR_CONST_SET(CLAS, SET, I)   for(std::set<CLAS *>::const_iterator   I=(SET).begin(); (I)!=SET.end(); ++(I))

class DCUnit;

inline bool acceptOp(const uint32 opcode, const uint32 want1)
	{ return ((opcode==want1) ? true : false); };
inline bool acceptOp(const uint32 opcode, const uint32 want1, const uint32 want2)
	{ return (acceptOp(opcode, want1) ? true :
		(opcode==want2) ? true : false); };
inline bool acceptOp(const uint32 opcode, const uint32 want1, const uint32 want2, const uint32 want3)
	{ return (acceptOp(opcode, want1, want2) ? true :
			(opcode==want3) ? true : false); };
inline bool acceptOp(const uint32 opcode, const uint32 want1, const uint32 want2, const uint32 want3, const uint32 want4)
	{ return (acceptOp(opcode, want1, want2, want3) ? true :
				(opcode==want4) ? true : false); };

/*inline bool acceptOp(const uint32 opcode, const uint32 want1)
{ return acceptIt(opcode, want1); }
inline bool acceptOp(const uint32 opcode, const uint32 want1, const uint32 want2)
{ return acceptIt(opcode, want1, want2); }
inline bool acceptOp(const uint32 opcode, const uint32 want1, const uint32 want2, const uint32 want3)
{ return acceptIt(opcode, want1, want2, want3); }
inline bool acceptOp(const uint32 opcode, const uint32 want1, const uint32 want2, const uint32 want3, const uint32 want4)
{ return acceptIt(opcode, want1, want2, want3, want4); }*/


inline void indent(Console &o, uint32 size)
{
	switch(size) {
		case 0: break;
		case 1: o.Print("\t"); break;
		case 2: o.Print("\t\t"); break;
		case 3: o.Print("\t\t\t"); break;
		case 4: o.Print("\t\t\t\t"); break;
		case 5: o.Print("\t\t\t\t\t"); break;
		case 6: o.Print("\t\t\t\t\t\t"); break;
		case 7: o.Print("\t\t\t\t\t\t\t"); break;
		case 8: o.Print("\t\t\t\t\t\t\t\t"); break;
		case 9: o.Print("\t\t\t\t\t\t\t\t\t"); break;
		default: assert(false);
	}
}

class PrintHelperNode
{
	public:
		inline void print_asm_header(Console &o, const uint32 h_off, const uint32 h_op) const
		{
			o.Printf("    %04X: %02X\t", h_off, h_op);
		}
		inline void print_mac_header(Console &o, const uint32 h_off, const uint32 h_op) const
		{
			o.Printf("    (%04X: %02X)\t", h_off, h_op);
		}

};

class Node;

/****************************************************************************
	Node
	The basic node type.
 ****************************************************************************/
class Node : public PrintHelperNode
{
	public:
		Node(const sint32 newOpcode=-1, const uint32 newOffset=0, const Type newRType=Type())
			: _opcode(newOpcode), _offset(newOffset), _rtype(newRType), linenum(0) {};
		virtual ~Node() {};
		
		inline sint32 opcode() const { return _opcode; };
		inline uint32 offset() const { return _offset; };
		
		inline const Type &rtype() const { return _rtype; };
		inline void rtype(const Type &newRType) { _rtype=newRType; };
		
		/* 'folds' the opcodes back into their original structure. returns
			'true' if the opcode needs to be pushed on the processing stack,
			'false' if it doesn't.
			Most fold functions will return true, since most opcodes will be
			pushed, but there are exeptional opcodes that will include
			themselves in the previous opcode, these will return false.
		*/
		virtual bool fold(DCUnit *unit, std::deque<Node *> &nodes)=0;
		
		// outputs 'unk' formatted script
		virtual void print_unk(Console &o, const uint32 isize) const=0;
		// outputs psuedo assembler code
		virtual void print_asm(Console &o) const;
		// outputs nothing interesting...
		virtual void print_mac(Console &o) const;
		// outputs raw binary code
		virtual void print_bin(ODequeDataSource &o) const=0;
		
		inline void fold_linenum(std::deque<Node *> &nodes);
		
		inline void print_linenum_unk(Console &o, const uint32 isize) const;
		inline void print_linenum_asm(Console &o) const;
		inline void print_linenum_bin(ODequeDataSource &o) const;

		//enum NodeType { IS_ERROR=0, IS_CALL, IS_FUNC
		//const NodeType isA() const=0;
		
	protected:
		Node *grab(std::deque<Node *> &nodes)
		{
			assert(nodes.size()>0); // always a good thing *grin*
			Node *n = nodes.back();
			nodes.pop_back();
			return n;
		};
		
		sint32	_opcode;
		uint32	_offset;
		Type	_rtype;
		Node *linenum;
	
	private:
};

inline void Node::print_asm(Console &o) const
{
	print_asm_header(o, _offset, _opcode);
}

inline void Node::print_mac(Console &o) const
{
	print_mac_header(o, _offset, _opcode);
}

inline void Node::print_linenum_unk(Console &o, const uint32 isize) const
{
	if(linenum!=0)
	{
		linenum->print_unk(o, isize);
	}
}

inline void Node::print_linenum_asm(Console &o) const
{
	if(linenum!=0)
	{
		linenum->print_asm(o);
		o.Putchar('\n');
	}
}

inline void Node::print_linenum_bin(ODequeDataSource &o) const
{
	if(linenum!=0) linenum->print_bin(o);
}

inline void Node::fold_linenum(std::deque<Node *> &nodes)
{
	// no point if we don't have any nodes to work with
	if(nodes.size()==0)
		return;
	// if we _are_ a line number opcode, we don't have a line number
	if(opcode()==0x5B)
		return;
	
	// HACK: FIXME: remove the line number if there's one before
	// doesn't handle multiple line numbers getting in the way,
	// but works.
	if(linenum==0)
	{
		Node *tnode=0;
		// if we're the last item on the stack, we need to remove
		// ourselves first
		if(nodes.back()->offset()==offset())
		{
			tnode = nodes.back();
			nodes.pop_back();
		}
		// then we grab the line number
		// need to make sure we've got an opcode now...
		if(nodes.size()>0)
			if(acceptOp(nodes.back()->opcode(), 0x5B))
			{
				linenum = nodes.back();
				nodes.pop_back();
			}
		// then we put ourselves back onto the stack, if we've removed us
		if(tnode!=0)
			nodes.push_back(tnode);
	}
}

/****************************************************************************
	UniNode
	Just a general 'node' to handle uniary operators.
 ****************************************************************************/

class UniNode : public Node
{
	public:
		UniNode(const sint32 newOpcode=-1, const uint32 newOffset=0,
			const Type newRType=Type())
			: Node(newOpcode, newOffset, newRType), node(0) {};
		virtual ~UniNode() { /* don't delete node */ };

	protected:
		void grab_n(std::deque<Node *> &nodes) { node=grab(nodes); };
		Node *node;

	private:
};

/****************************************************************************
	BinNode
	Just a general 'node' to handle binary operators.
 ****************************************************************************/

class BinNode : public Node
{
	public:
		BinNode(const sint32 newOpcode=-1, const uint32 newOffset=0,
			const Type newRType=Type())
			: Node(newOpcode, newOffset, newRType), lnode(0), rnode(0) {};
		virtual ~BinNode() { /* don't delete lnode, rnode */ };

	protected:
		void grab_l(std::deque<Node *> &nodes) { lnode=grab(nodes); };
		void grab_r(std::deque<Node *> &nodes) { rnode=grab(nodes); };

		Node *lnode;
		Node *rnode;

	private:
};

/****************************************************************************
	ColNode
	A base class to hold the variable sized arrays of nodes (a 'collection'),
	for function parameters and the like.
 ****************************************************************************/

class ColNode : public Node
{
	public:
		ColNode(const sint32 newOpcode=-1, const uint32 newOffset=0,
			const Type newRType=Type())
			: Node(newOpcode, newOffset, newRType), pnode(0) {};
		virtual ~ColNode() {};

	protected:
		void grab_p(std::deque<Node *> &nodes, sint32 tempsize)
		{
			while(tempsize>0)
			{
				con.Printf("tempsize=%d\n", tempsize);
				pnode.push_back(grab(nodes));
				con.Printf("nodesize=%d\n", pnode.back()->rtype().size());
				pnode.back()->print_asm(con);
				tempsize-=pnode.back()->rtype().size();
			}
			assert(tempsize==0);
		};

		std::deque<Node *> pnode;

	private:
};

/****************************************************************************
	Useful Funcs
	Defnitions in Folder.cpp, for 'obvious' reasons.
 ****************************************************************************/

class DCUnit;

bool print_assert(const Node *n, const DCUnit *u=0);
bool print_assert_nodes(std::deque<Node *> &nodes, uint32 index);

#endif

