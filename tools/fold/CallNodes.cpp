/*
 *	CallNodes.cpp -
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

#include "CallNodes.h"
#include "Folder.h"
// FIXME: Fix this
/* 'cause we don't have dynamic binding. c++ sucks. */
void AddSpToCallNode(CallPostfixNode *cpn, Node *node)
{
	CallNode *cn = dynamic_cast<CallNode *>(node);
	cn->setAddSP(cpn);
}
void PushRetValToCallNode(CallPostfixNode *cpn, Node *node)
{
	CallNode *cn = dynamic_cast<CallNode *>(node);
	cn->setRetVal(cpn);
}

/****************************************************************************
	CallPostfixNode
 ****************************************************************************/

bool CallPostfixNode::fold(Unit *unit, std::deque<Node *> &nodes)
{
	if(acceptOp(nodes.back()->opcode(), 0x0F, 0x11))
	{
		switch(ptype)
		{
			case PUSH_RETVAL: PushRetValToCallNode(this, nodes.back()); break;
			case ADDSP: AddSpToCallNode(this, nodes.back()); break;
			default: assert(print_assert(this));
		}
		nodes.back()->fold(unit, nodes);
	}
	return false;
}

void CallPostfixNode::print_unk(Console &o, const uint32 /*isize*/) const
{
	switch(ptype)
	{
		case PUSH_RETVAL:
			assert(rtype().type()!=Type::T_INVALID);
			switch(rtype().type())
			{
				case Type::T_BYTE: o.Printf("push_byte_retval_NOPRINT()"); break;
				case Type::T_WORD: o.Printf("push_retval_NOPRINT()"); break;
				default: assert(print_assert(this));
			}
			break;
		case ADDSP:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("addsp_NOPRINT(0x%s%02X)", sp>0x7F?"-":"", sp>0x7F?0x100-sp:sp);
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

void CallPostfixNode::print_asm(Console &o) const
{
	Node::print_asm(o);
	switch(ptype)
	{
		case PUSH_RETVAL:
			assert(rtype().type()!=Type::T_INVALID);
			switch(rtype().type())
			{
				case Type::T_BYTE: o.Printf("push byte\tretval"); break;
				case Type::T_WORD: o.Printf("push\t\tretval"); break;
				default: assert(false);
			}
			break;
		case ADDSP:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("add sp\t\t%s%02Xh", sp>0x7F?"-":"", sp>0x7F?0x100-sp:sp);
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

void CallPostfixNode::print_bin(OBufferDataSource &o) const
{
	switch(ptype)
	{
		case PUSH_RETVAL:
			assert(rtype().type()!=Type::T_INVALID);
			switch(rtype().type())
			{
				case Type::T_BYTE: o.write1(0x5D); break;
				case Type::T_WORD: o.write1(0x5E); break;
				default: assert(print_assert(this));
			}
			break;
		case ADDSP:
			assert(rtype().type()==Type::T_INVALID);
			o.write1(0x6E);
			o.write1(sp);
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

/****************************************************************************
	CallMutatorNode
 ****************************************************************************/

void CallMutatorNode::print_unk(Console &o, const uint32 isize) const
{
	switch(mtype)
	{
		case PUSH_INDIRECT:
			assert(rtype().type()!=Type::T_INVALID);
			o.Printf("push_indirect_NOPRINT(0x%02X, ", numBytes);
			lnode->print_unk(o, isize);
			o.Putchar(')');
			break;
		case SET_INFO:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("setinfo_NOPRINT(");
			lnode->print_unk(o, isize);
			o.Printf(", ");
			rnode->print_unk(o, isize);
			o.Putchar(')');
			break;
		case PROCESS_EXCLUDE:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("process_exclude()");
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

void CallMutatorNode::print_asm(Console &o) const
{
	switch(mtype)
	{
		case PUSH_INDIRECT:
			assert(rtype().type()!=Type::T_INVALID);
			lnode->print_asm(o);
			o.Putchar('\n');
			Node::print_asm(o);
			o.Printf("push indirect\t%02Xh bytes",  numBytes);
			break;
		case SET_INFO:
			assert(rtype().type()==Type::T_INVALID);
			lnode->print_asm(o);
			o.Putchar('\n');
			rnode->print_asm(o);
			o.Putchar('\n');
			Node::print_asm(o);
			o.Printf("set info");
			break;
		case PROCESS_EXCLUDE:
			assert(rtype().type()==Type::T_INVALID);
			Node::print_asm(o);
			o.Printf("process exclude");                 
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

void CallMutatorNode::print_bin(OBufferDataSource &o) const
{
	switch(mtype)
	{
		case PUSH_INDIRECT:
			assert(rtype().type()!=Type::T_INVALID);
			lnode->print_bin(o);
			o.write1(0x4C);
			o.write1(numBytes);
			break;
		case SET_INFO:
			assert(rtype().type()==Type::T_INVALID);
			//Node::print_mac(con); // FIXME: Debugging only!
			lnode->print_bin(o);
			rnode->print_bin(o);
			o.write1(0x77);
			break;
		case PROCESS_EXCLUDE:
			assert(rtype().type()==Type::T_INVALID);
			//Node::print_mac(con); // FIXME: Debugging only!
			o.write1(0x78);
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

bool CallMutatorNode::fold(Unit */*unit*/, std::deque<Node *> &nodes)
{
	switch(mtype)
	{
		case PUSH_INDIRECT:
			// technically this type can be anything, but we'll restrict outselves
			// to a dword for simplicity's sake atm.
			assert(nodes.back()->rtype()==Type::T_DWORD);
			grab_l(nodes);
			break;
		case SET_INFO:
			assert(nodes.back()->rtype()==Type::T_WORD);
			grab_r(nodes);
			assert(nodes.back()->rtype()==Type::T_WORD);
			grab_l(nodes);
			break;
		case PROCESS_EXCLUDE:
			break;
		default: assert(print_assert(this));
	}
	return true;
};

/****************************************************************************
	CallNode
	Handles 'call', 'calli' opcodes.
 ****************************************************************************/

bool CallNode::fold(Unit *unit, std::deque<Node *> &nodes)
{
	// register ourselves so we're printed as an 'extern', even if we really aren't.
	switch(ctype)
	{
		case CALL: unit->registerExternFunc(this); break;
		case CALLI: unit->registerExternIntrinsic(this); break;
		default: assert(print_assert(this, unit));
	}
	
	/* if it's the first time we're called, we don't know how
		many bytes of parameters we've got, so we're kinda stuck.
		we also don't know what our return value is either.
		we'll 'fix' this on a latter call once we've got our data. */
	if(addSP==0 && retVal==0)
		return true;
	// we've just gotten our addsp opcode
	else if(addSP!=0 && retVal==0)
	{
		// we need to remove ourselves from the stack before doing anything tricky
		assert(acceptOp(nodes.back()->opcode(), 0x0F, 0x11));
		Node *us=nodes.back();
		nodes.pop_back();

		// grab the nodes, note the '-', gotta invert the value since it's the number
		// we're _removing_ from the stack
		grab_p(nodes, std::abs(static_cast<sint32>(addSP->size())));
		
		// add us back to the stack
		nodes.push_back(us);
	}
	// we've just gotten our return opcode, or there is no return opcode forthcoming...
	else if(ctype==CALLI && spsize==0 && addSP==0 && retVal!=0)
		rtype(retVal->rtype());
	else if(addSP!=0 && retVal!=0)
		rtype(retVal->rtype());
	else
		assert(print_assert(this, unit)); // need to add the case where there's no return value and/or no addSP.
	
	// potential line numbers come 'before' the pushed values, so we need to grab
	// them after the values
	fold_linenum(nodes);
	
	return false;
}

void CallNode::print_extern_unk(Console &o, const uint32 /*isize*/) const
{
	switch(ctype)
	{
		case CALLI:
			{
				if(rtype().type()!=Type::T_INVALID)
				{
					o.Print(rtype().name()); o.Putchar('\t');
				}
				o.Printf("Intrinsic%04X(", intrinsic);
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					if(i!=pnode.rbegin()) o.Print(", ");
					o.Print((*i)->rtype().name());
				}
				o.Print(");");
				break;
			}
		case CALL:
			{
				if(rtype().type()!=Type::T_INVALID)
				{
					o.Print(rtype().name()); o.Putchar('\t');
				}
				o.Printf("class_%04X_function_%04X(", uclass, targetOffset);
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					if(i!=pnode.rbegin()) o.Print(", ");
					o.Print((*i)->rtype().name());
				}
				o.Print(");");
				break;
			}
		default: assert(print_assert(this)); // can't happen
	}
}

void CallNode::print_unk(Console &o, const uint32 isize) const
{
	print_linenum_unk(o, isize);
	switch(ctype)
	{
		case CALLI:
			{
				#if 0
				if(rtype().type()!=Type::T_VOID)
				{
					rtype().print_unk(o); o.Putchar(' ');
				}
				#endif
				o.Printf("Intrinsic%04X(", intrinsic);
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					if(i!=pnode.rbegin()) o.Print(", ");
					(*i)->print_unk(o, isize);
				}
				o.Putchar(')');
				break;
			}
		case CALL:
			{
				#if 0
				if(rtype().type()!=Type::T_VOID)
				{
					rtype().print_unk(o); o.Putchar(' ');
				}
				#endif
				o.Printf("class_%04X_function_%04X(", uclass, targetOffset);
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					if(i!=pnode.rbegin()) o.Print(", ");
					(*i)->print_unk(o, isize);
				}
				o.Putchar(')');
				break;
			}
		default: assert(print_assert(this)); // can't happen
	}
}

void CallNode::print_asm(Console &o) const
{
	print_linenum_asm(o);
	switch(ctype)
	{
		case CALLI:
			{
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					(*i)->print_asm(o); o.Putchar('\n');
				}
				Node::print_asm(o);
				o.Printf("calli\t\t%02Xh %04Xh", spsize, intrinsic);
				//FIXME: o.Printf(" (%s)", convert->intrinsics()[op.i1]);
				if(addSP!=0)
				{
					o.Putchar('\n');
					addSP->print_asm(o);
				}
				if(rtype()!=Type::T_VOID)
				{
					assert(retVal!=0);
					o.Putchar('\n');
					retVal->print_asm(o);
				}
				break;
			}
		case CALL:
			{
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					(*i)->print_asm(o); o.Putchar('\n');
				}
				Node::print_asm(o);
				o.Printf("call\t\t%04X:%04X", uclass, targetOffset);
				o.Putchar('\n');
				if(addSP!=0)
					addSP->print_asm(o);
				if(rtype()!=Type::T_VOID)
				{
					assert(retVal!=0);
					o.Putchar('\n');
					retVal->print_asm(o);
				}
				//FIXME: o.Printf(" (%s)", functionaddresstostring(uclass, targetOffset, ucfile).c_str());
				break;
			}
		default: assert(print_assert(this)); // can't happen
	}
}

void CallNode::print_bin(OBufferDataSource &o) const
{
	print_linenum_bin(o);
	switch(ctype)
	{
		case CALLI:
			{
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					(*i)->print_bin(o);
				}
				o.write1(0x0F);
				o.write1(spsize);
				o.write2(intrinsic);
				if(addSP!=0)
					addSP->print_bin(o);
				if(rtype()!=Type::T_VOID)
				{
					assert(retVal!=0);
					retVal->print_bin(o);
				}
				break;
			}
		case CALL:
			{
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					(*i)->print_bin(o);
				}
				o.write1(0x11);
				o.write2(uclass);
				o.write2(targetOffset);
				if(addSP!=0)
					addSP->print_bin(o);
				if(rtype()!=Type::T_VOID)
				{
					assert(retVal!=0);
					retVal->print_bin(o);
				}
				break;
			}
		default: assert(print_assert(this)); // can't happen
	}
}


///////////////////////////////////////////////////////

/* Takes an opcode number and returns true if it's potential CallNode */
/*bool is_call(uint32 op)
{
	switch(op)
	{
		case 0x0F: // calli
		case 0x57: // spawn
			return true;
		default:   // else...
			return false;
	}
	return false; // can't happen
}

class CallNode : public ColNode
{
	public:
		CallNode() : ColNode("Call"), sp_size(0),
			num_bytes(0), intrinsic_num(0),
			target_class(0), target_func(0),
			param_size(0), this_size(0) {};
		~CallNode() {};

		void print() const;
		void fold(const uint32 pos);

	protected:

	private:
		void get_return(const uint32 end);

		enum calltype { CALLI, CALL, SPAWN } ctype;

		uint32 sp_size;

		// for CALLI
		uint32 num_bytes; // technically should be the same as sp_size, but...
		uint32 intrinsic_num;

		// for CALL and SPAWN
		uint32 target_class;
		uint32 target_func;

		// for SPAWN
		uint32 param_size;
		uint32 this_size;
};

void CallNode::fold(const uint32 end)
{
	PTRACE(("(Call)\t\tPOS: %4d\tOP: %04X offset: %04X\n", end, foldops[end].op(), foldops[end].offset));
	assert(foldops[end].deleted==false);

	_opcode = foldops[end].op(); // store the op locally
	_offset = foldops[end].offset; // store the offset locally

	get_return(end); // grab us out return node

	sp_size = foldops[end].sp_size;
	sint32 tempsize;

	switch(_opcode)
	{
		case 0x0F:
			ctype = CALLI;
			num_bytes = foldops[end].i0;
			intrinsic_num = foldops[end].i1;
			tempsize=sp_size;
			break;
		case 0x57:
			ctype = SPAWN;
			param_size = foldops[end].i0;
			this_size = foldops[end].i1; // probably not. We don't know what it's for.
			target_class = foldops[end].i2;
			target_func = foldops[end].i3;
			tempsize=param_size+4; // +4== size of dword/this pointer (FIXME: #define this or something)
			break;
		default:	assert(false);	// can't happen
	}

	grab_p(tempsize, end);
	assert(tempsize==0);
}

void CallNode::print() const
{
	printf("(%s) ", rtype.name());

	switch(ctype)
	{
		case CALLI:
			if(crusader)
				printf("unknown_%04X(", intrinsic_num);
			else
			{
				string is(convert->intrinsics()[intrinsic_num]);
				printf("%s(", is.substr(0, is.find("(")).c_str());
			}
			break;
		case SPAWN:
			printf("spawn_%02X_%02X_class_%04X_function_%04X(", param_size, this_size, target_class, target_func);
			break;
		default:
			assert(false);
	}

	for(vector<Node *>::const_iterator n=pnode.begin(); n!=pnode.end(); ++n)
	{
		if(n!=pnode.begin()) printf(", ");
		(*n)->print();
	}
	printf(")");
}

void CallNode::get_return(const uint32 end)
{
	// then do the same thing for the three return types.
	for(unsigned int ret=end+1; ret<foldops.size(); ++ret)
	{
		if(foldops[ret].deleted==false)
		{
			switch(foldops[ret].op())
			{
				// first the 'proper' return values.
				// yeah, using PushVarNodes here is somewhat of a hack, but they do exactly what I need.
				case 0x5D: // push byte return (2 bytes)
					foldops[ret].node = newNode(ret); // pointless, just to keep things clean
					foldops[ret].deleted=true; // ... and delete the opcode...
					rtype=Type::T_BYTE;
					ret=foldops.size(); // finished
					break;
				case 0x5E: // push word return (2 bytes)
					foldops[ret].node = newNode(ret); // pointless, just to keep things clean
					foldops[ret].deleted=true; // ... and delete the opcode...
					rtype=Type::T_WORD;
					ret=foldops.size(); // finished
					break;
				case 0x5F: // push dword return (4 bytes)
					foldops[ret].node = newNode(ret); // pointless, just to keep things clean
					foldops[ret].deleted=true; // ... and delete the opcode...
					rtype=Type::T_DWORD;
					ret=foldops.size(); // finished
					break;
				// then the case where we find a function call,
				// but no push, so there's been no return value
				case 0x0F: // calli
				case 0x11: // call
				case 0x57: // spawn
				case 0x58: // spawn inline
					// if we didn't locate a 'push return' opcode, then it's a 'void' returning function
					rtype=Type::T_VOID;
					ret=foldops.size(); // finished
					break;
				default: // ignore all other ops.
					break;
			}
		}
	}
}*/

