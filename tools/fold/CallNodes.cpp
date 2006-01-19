/*
 *	CallNodes.cpp -
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

#include "CallNodes.h"
#include "Folder.h"
#include "VarNodes.h"

// FIXME: Fix this
/* 'cause we don't have dynamic binding. c++ sucks. */
void AddSpToDCCallNode(DCCallPostfixNode *cpn, Node *node)
{
	DCCallNode *cn = static_cast<DCCallNode *>(node);
	cn->setAddSP(cpn);
}
void PushRetValToDCCallNode(DCCallPostfixNode *cpn, Node *node)
{
	DCCallNode *cn = static_cast<DCCallNode *>(node);
	cn->setRetVal(cpn);
}
void AddFreeToDCCallNode(DCCallPostfixNode *cpn, Node *node)
{
	DCCallNode *cn = static_cast<DCCallNode *>(node);
	cn->addFree(cpn);
}

/****************************************************************************
	DCCallPostfixNode
 ****************************************************************************/

bool DCCallPostfixNode::fold(DCUnit *unit, std::deque<Node *> &nodes)
{
	if(acceptOp(nodes.back()->opcode(), 0x0F, 0x11, 0x57))
	{
		switch(ptype)
		{
			case PUSH_RETVAL: PushRetValToDCCallNode(this, nodes.back()); break;
			case FREESTR: AddFreeToDCCallNode(this, nodes.back()); break;
			case ADDSP: AddSpToDCCallNode(this, nodes.back()); break;
			default: assert(print_assert(this));
		}
		nodes.back()->fold(unit, nodes);
	}
	return false;
}

void DCCallPostfixNode::print_unk(Console &o, const uint32 /*isize*/, const bool comment) const
{
	if(!comment) return; // handle debug output

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
		case FREESTR:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("free_str_NOPRINT(%s)", suc::print_sp(sp));
			break;
		case ADDSP:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("addsp_NOPRINT(0x%s%02X)", sp>0x7F?"-":"", sp>0x7F?0x100-sp:sp);
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

void DCCallPostfixNode::print_asm(Console &o) const
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
		case FREESTR:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("free string\t%s", suc::print_sp(sp));
			break;
		case ADDSP:
			assert(rtype().type()==Type::T_INVALID);
			o.Printf("add sp\t\t%s%02Xh", sp>0x7F?"-":"", sp>0x7F?0x100-sp:sp);
			break;
		default: assert(print_assert(this)); // can't happen
	}
}

void DCCallPostfixNode::print_bin(ODequeDataSource &o) const
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
		case FREESTR:
			assert(rtype().type()==Type::T_INVALID);
			o.write1(0x65);
			o.write1(sp);
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
	DCCallMutatorNode
 ****************************************************************************/

void DCCallMutatorNode::print_unk(Console &o, const uint32 isize, const bool comment) const
{
	if(!comment && mtype!=PROCESS_EXCLUDE) return;
	
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

void DCCallMutatorNode::print_asm(Console &o) const
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

void DCCallMutatorNode::print_bin(ODequeDataSource &o) const
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

bool DCCallMutatorNode::fold(DCUnit * /*unit*/, std::deque<Node *> &nodes)
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
	DCCallNode
	Handles 'call', 'calli' opcodes.
 ****************************************************************************/

bool DCCallNode::fold(DCUnit *unit, std::deque<Node *> &nodes)
{
	// register ourselves so we're printed as an 'extern', even if we really aren't.
	switch(ctype)
	{
		case CALL: unit->registerExternFunc(this); break;
		case CALLI: unit->registerExternIntrinsic(this); break;
		case SPAWN: unit->registerExternFunc(this); break;
		default: assert(print_assert(this, unit));
	}
	
	/* if it's the first time we're called, we don't know how
		many bytes of parameters we've got, so we're kinda stuck.
		we also don't know what our return value is either.
		we'll 'fix' this on a latter call once we've got our data. */
	if(addSP==0 && retVal==0)
		return true;
	// we've just gotten our addsp opcode
	else if((addSP!=0 && retVal==0) // normal
		|| (ctype==SPAWN && (thispsize>0 && thisP==0)/*&& addSP==0 && retVal==0 && spsize==0*/)) // 'spawn' with thispsize!=0
	{
		con.Printf("Op %02X %04X\n", nodes.back()->opcode(), nodes.back()->offset());
		// we need to remove ourselves from the stack before doing anything tricky
		assert(acceptOp(nodes.back()->opcode(), 0x0F, 0x11, 0x57));
		Node *us=nodes.back();
		nodes.pop_back();
		
		// if we're a 'spawn' we need to strip our *this off the stack, before
		// we do anything funky.
		if(thispsize>0 && ctype==SPAWN)
		{
			thisP = grab(nodes);
			//con.Printf("SPAWN THISP GRAB:\n");
			//print_assert(thisP, unit);
			//need to make sure it's a dword too.
			assert(thisP!=0 && thisP->rtype()==Type::T_DWORD);
		}
		// grab the nodes, note the '-', gotta invert the value since it's the number
		// we're _removing_ from the stack
		// if we're a SPAWN, we need to double this
		if(ctype==SPAWN) {
			if(spsize>0)
				grab_p(nodes, std::abs(static_cast<sint32>(addSP->size()))/* * 2*/);
		} else
			grab_p(nodes, std::abs(static_cast<sint32>(addSP->size())));

		// add us back to the stack
		nodes.push_back(us);
	}
	// we've just gotten our return opcode, or there is no return opcode forthcoming...
	else if(ctype==CALLI && spsize==0 && addSP==0 && retVal!=0)
		rtype(retVal->rtype());
	else if(addSP!=0 && retVal!=0)
		rtype(retVal->rtype());
	else if(nodes.size()>0 && acceptOp(nodes.back()->opcode(), 0x65))
	{ /* do nothing... */ }
	else if(retVal!=0 && nodes.size()>0 && acceptOp(nodes.back()->opcode(), 0x57))
	{ /* do nothing...
		since we're just got the retval of a 'spawn' opcode, and there shouldn't be
		anything else to grab... */
	}
	else
	{
		con.Printf("Fnord: %d\n", freenodes.size());
		assert(print_assert(this, unit)); // need to add the case where there's no return value and/or no addSP.
	}
	
	// potential line numbers come 'before' the pushed values, so we need to grab
	// them after the values
	fold_linenum(nodes);
	
	return false;
}

void DCCallNode::print_extern_unk(Console &o, const uint32 /*isize*/) const
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
		case SPAWN:
			{
				o.Print("spawn ");
				if(rtype().type()!=Type::T_INVALID)
				{
					o.Print(rtype().name()); o.Putchar('\t');
				}
				if(thisP!=0) o.Print(thisP->rtype().name());
				o.Printf("->class_%04X_function_%04X(", uclass, targetOffset);
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

void DCCallNode::print_unk(Console &o, const uint32 isize) const
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
		case SPAWN:
			{
				o.Print("spawn ");
				#if 0
				if(rtype().type()!=Type::T_VOID)
				{
					rtype().print_unk(o); o.Putchar(' ');
				}
				#endif
				assert(thispsize>0 || thisP!=0 || print_assert(this));
				if(thispsize) // only if we have a this we should worry about it.
					thisP->print_unk(o, isize);
				o.Printf("->class_%04X_function_%04X(", uclass, targetOffset);
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

void DCCallNode::print_asm(Console &o) const
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
				for(std::list<DCCallPostfixNode *>::const_reverse_iterator i=freenodes.rbegin(); i!=freenodes.rend(); ++i)
				{
					o.Putchar('\n'); (*i)->print_asm(o);
				}
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
				if(pnode.size()==0)
					o.Putchar('\n');

				Node::print_asm(o);
				o.Printf("call\t\t%04X:%04X\n", uclass, targetOffset);
				//o.Putchar('\n');
				if(addSP!=0)
					addSP->print_asm(o);
				if(rtype()!=Type::T_VOID)
				{
					assert(retVal!=0);
					o.Putchar('\n');
					retVal->print_asm(o);
				}
				//FIXME: o.Printf(" (%s)", convert->UsecodeFunctionAddressToString(uclass, targetOffset, ucfile).c_str());
				break;
			}
		case SPAWN:
			{
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					(*i)->print_asm(o); o.Putchar('\n');
				}
				assert((thispsize>0 && thisP!=0) || print_assert(this));
				if(thispsize) // only if we have a this we should worry about it.
					thisP->print_asm(o);
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

void DCCallNode::print_bin(ODequeDataSource &o) const
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
		case SPAWN:
			{
				for(std::deque<Node *>::const_reverse_iterator i=pnode.rbegin(); i!=pnode.rend(); ++i)
				{
					(*i)->print_bin(o);
				}
				assert(thispsize>0 || thisP!=0 || print_assert(this));
				if(thispsize) // only if we have a this we should worry about it.
					thisP->print_bin(o);
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

/* Takes an opcode number and returns true if it's potential DCCallNode */
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

class DCCallNode : public ColNode
{
	public:
		DCCallNode() : ColNode("Call"), sp_size(0),
			num_bytes(0), intrinsic_num(0),
			target_class(0), target_func(0),
			param_size(0), this_size(0) {};
		~DCCallNode() {};

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

void DCCallNode::fold(const uint32 end)
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

void DCCallNode::print() const
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

void DCCallNode::get_return(const uint32 end)
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

