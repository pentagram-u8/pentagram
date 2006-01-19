/*
 *	OperatorNodes.cpp -
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

#include "OperatorNodes.h"
#include "Folder.h"

/****************************************************************************
	NonOperatorNode
 ****************************************************************************/
/*void TempNode::fold(const uint32 end)
{
	PTRACE(("(Temp)\t\tPOS: %4d\tOP: %04X offset: %04X\n", end, foldops[end].op(), foldops[end].offset));
	assert(foldops[end].deleted==false);

	_opcode = foldops[end].op(); // store the op locally
	_offset = foldops[end].offset; // store the offset locally

	switch(_opcode)
	{
		case 0x12:
			ttype = POP_TEMP;
			break;
		case 0x4E:
			ttype = PUSH_GLOBAL;
			offset = foldops[end].i0;
			size = foldops[end].i1;
			break;
		case 0x4F:
			ttype = POP_GLOBAL;
			offset = foldops[end].i0;
			size = foldops[end].i1;
			break;
		case 0x50:
			ttype = RET;
			break;
		case 0x51:
			ttype = JNE;
			offset = foldops[end].nextoffset + static_cast<short>(foldops[end].i0);
			break;
		case 0x52:
			ttype = JMP;
			offset = foldops[end].nextoffset + static_cast<short>(foldops[end].i0);
			break;
		case 0x53:
			ttype = SUSPEND;
			break;
		// the PUSH_*_RET ops are just placeholders to temporarially stop things complaning
		// should be removed soon: FIXME:
		case 0x5D:
			ttype = PUSH_BYTE_RET;
			break;
		case 0x5E:
			ttype = PUSH_WORD_RET;
			break;
		case 0x5F:
			ttype = PUSH_DWORD_RET;
			break;
		case 0x62:
			ttype = FREE_STRING_BP;
			node  = new PushVarNode(PushVarNode::VT_STRING, PushVarNode::DT_BP,
				foldops[end].i0);
			break;
		case 0x63:
			ttype = FREE_SLIST_BP;
			node  = new PushVarNode(PushVarNode::VT_SLIST, PushVarNode::DT_BP,
				foldops[end].i0);
			break;
		case 0x64:
			ttype = FREE_LIST_BP;
			node  = new PushVarNode(PushVarNode::VT_LIST, PushVarNode::DT_BP,
				foldops[end].i0);
			break;
		case 0x65:
			ttype = FREE_STRING_SP;
			node = new PushVarNode(PushVarNode::VT_STRING, PushVarNode::DT_SP,
				foldops[end].i0);
			break;
		case 0x66:
			ttype = FREE_LIST_SP;
			node = new PushVarNode(PushVarNode::VT_LIST, PushVarNode::DT_SP,
				foldops[end].i0);
			break;
		case 0x6C:
			ttype = PARAM_PID_CHANGE;
			type = foldops[end].i1;
			switch(type)
			{
				case 0x01:
					node = new PushVarNode(PushVarNode::VT_STRING, PushVarNode::DT_BP,
						foldops[end].i0);
					break;
				case 0x02:
					node = new PushVarNode(PushVarNode::VT_SLIST, PushVarNode::DT_BP,
						foldops[end].i0);
					break;
				case 0x03:
					node = new PushVarNode(PushVarNode::VT_LIST, PushVarNode::DT_BP,
						foldops[end].i0);
					break;
			}
			break;
		case 0x70:
			ttype = LOOP;
			node  = new PushVarNode(PushVarNode::VT_WORD, PushVarNode::DT_BP,
				foldops[end].i0);
			size = foldops[end].i1;
			type = foldops[end].i2;
			break;
		case 0x73:
			ttype = LOOPNEXT;
			break;
		case 0x74:
			ttype = LOOPSCR;
			delta = static_cast<short>(foldops[end].i0);
			break;
		case 0x75:
			ttype = FOREACH_LIST;
			node  = new PushVarNode(PushVarNode::VT_LIST, PushVarNode::DT_BP,
				foldops[end].i0);
			size = foldops[end].i1;
			offset = foldops[end].nextoffset + static_cast<short>(foldops[end].i2);
			break;
		case 0x76:
			ttype = FOREACH_SLIST;
			node  = new PushVarNode(PushVarNode::VT_SLIST, PushVarNode::DT_BP,
				foldops[end].i0);
			size = foldops[end].i1;
			offset = foldops[end].nextoffset + static_cast<short>(foldops[end].i2);
			break;
		case 0x79:
			if(!crusader) // if we're u8...
				ttype = END;
			else
				assert(false); // if we're crusader, which can't happen.
		case 0x7A:
			ttype = END;
			break;
		default:
			assert(false); // can't happen
	}
}

void TempNode::print() const
{
	switch(ttype)
	{
		case POP_TEMP:
			printf("pop_temp()");
			break;
		case PUSH_GLOBAL:
			printf("push_global(0x%04X, 0x%02X)", offset, size);
			break;
		case POP_GLOBAL:
			printf("pop_global(0x%04X, 0x%02X)", offset, size);
			break;
		case RET:
			printf("ret");
			break;
		case JNE:
			printf("jne(0x%04X)", offset);
			break;
		case JMP:
			printf("jmp(0x%04X)", offset);
			break;
		case SUSPEND:
			printf("suspend");
			break;
		case PUSH_BYTE_RET:
			printf("push_byte_ret()");
			break;
		case PUSH_WORD_RET:
			printf("push_byte_ret()");
			break;
		case PUSH_DWORD_RET:
			printf("push_byte_ret()");
			break;
		case FREE_STRING_BP:
			printf("free_string_bp(");
			node->print();
			printf(")");
			break;
		case FREE_SLIST_BP:
			printf("free_slist_bp(");
			node->print();
			printf(")");
			break;
		case FREE_LIST_BP:
			printf("free_list_bp(");
			node->print();
			printf(")");
			break;
		case FREE_STRING_SP:
			printf("free_string_sp(");
			node->print();
			printf(")");
			break;
		case FREE_LIST_SP:
			printf("free_list_sp(");
			node->print();
			printf(")");
			break;
		case PARAM_PID_CHANGE:
			printf("param_pid_change(");
			node->print();
			printf(", 0x%02X)", type);
			break;
		case LOOP:
			printf("loop(");
			node->print();
			printf(", %d, %d)", size, type);
			break;
		case LOOPNEXT:
			printf("loopnext()");
			break;
		case LOOPSCR:
			printf("loopscr(%d, \"%c\")", delta, static_cast<char>(delta));
			break;
		case FOREACH_LIST:
			printf("foreach_list(");
			node->print();
			printf(", %d, 0x%04X)", size, offset);
			break;
		case FOREACH_SLIST:
			printf("foreach_slist(");
			node->print();
			printf(", %d, 0x%04X)", size, offset);
			break;
		case END:
			printf("end");
			break;
		default:
			assert(false); // can't happen
	}

}*/


/****************************************************************************
	UniOperatorNode
 ****************************************************************************/

void UniOperatorNode::print_unk(Console &o, const uint32 isize) const
{
	assert(rtype().type()!=Type::T_INVALID);
	switch(otype)
	{
		case NOT:
			assert(node!=0);
			o.Printf("not ");
			node->print_unk(o, isize);
			break;
		case STR_TO_PTR:
			assert(node!=0);
			o.Printf("str_to_ptr(");
			node->print_unk(o, isize);
			o.Putchar(')');
			break;
		default: assert(false); // can't happen
	}
}

void UniOperatorNode::print_asm(Console &o) const
{
	assert(rtype().type()!=Type::T_INVALID);
	switch(otype)
	{
		case NOT:
			assert(node!=0);
			node->print_asm(o);
			o.Putchar('\n');
			Node::print_asm(o);
			o.Printf("not");
			break;
		case STR_TO_PTR:
			assert(node!=0);
			node->print_asm(o);
			o.Putchar('\n');
			Node::print_asm(o);
			o.Printf("str to ptr");
			break;
		default: assert(false); // can't happen
	}
}

void UniOperatorNode::print_bin(ODequeDataSource &o) const
{
	assert(rtype().type()!=Type::T_INVALID);
	switch(otype)
	{
		case NOT:
			assert(node!=0);
			node->print_bin(o);
			o.write1(0x30);
			break;
		case STR_TO_PTR:
			assert(node!=0);
			node->print_bin(o);
			o.write1(0x6B);
			break;
		default: assert(false); // can't happen
	}
}

bool UniOperatorNode::fold(DCUnit * /*unit*/, std::deque<Node *> &nodes)
{
	switch(otype)
	{
		case NOT:
			assert(acceptType(nodes.back()->rtype(), Type::T_WORD, Type::T_BYTE));
			//assert(nodes.back()->rtype()==Type::T_WORD || nodes.back()->rtype()==Type::T_BYTE);
			grab_n(nodes);
			break;
		case STR_TO_PTR:
			assert(acceptType(nodes.back()->rtype(), Type::T_STRING));
			grab_n(nodes);
			break;
		default: assert(false);
	}
	return true;
};

/*void UniOperatorNode::fold(const uint32 end)
{
	assert(foldops[end].deleted==false);

	_opcode = foldops[end].op(); // store the op locally
	_offset = foldops[end].offset; // store the offset locally

	switch(_opcode)
	{
		case 0x3B: otype=BITNOT;	rtype=Type::T_WORD; break;
		default: assert(false); // can't happen
	}

	sint32 tempsize = 2;
	grab_n(tempsize, end);
	assert(tempsize==0);

	// debugging
	assert(node!=0);
}*/

/*void UniOperatorNode::print() const
{
	assert(node!=0);

	printf("(%s) (", rtype.name());

	switch(otype)
	{
		case BITNOT:	printf("~");		break;
		default:	assert(false); // can't happen
	}

	node->print();
	printf(")");
}*/

/*void ConvertNode::fold(const uint32 end)
{
	assert(foldops[end].deleted==false);

	_opcode = foldops[end].op(); // store the op locally
	_offset = foldops[end].offset; // store the offset locally

	switch(_opcode)
	{
		case 0x60: ctype=WORD_DWORD;  break;
		case 0x61: ctype=DWORD_WORD;  break;
		case 0x6B: ctype=STR_PTR; break;
		default:   assert(false); // can't happen
	}

	sint32 tempsize;

	switch(ctype)
	{
		case WORD_DWORD: tempsize=2; break;
		case DWORD_WORD: tempsize=4; break;
		case STR_PTR:    tempsize=2; break;
		default:         assert(false); // can't happen
	}

	grab_n(tempsize, end);
	assert(tempsize==0);

	// now we switch the size, since we're converting the type
	switch(ctype)
	{
		case WORD_DWORD: assert(node->rtype.size()==2); rtype=Type::T_DWORD; break;
		case DWORD_WORD: assert(node->rtype.size()==4); rtype=Type::T_WORD; break;
		case STR_PTR:    assert(node->rtype.size()==2); rtype=Type::T_STRPTR; break;
		default:         assert(false); // can't happen
	}
}

void ConvertNode::print() const
{
	assert(node!=0);

	switch(ctype)
	{
		case WORD_DWORD: printf("(dword) to_dword("); break;
		case DWORD_WORD: printf("(word) to_word(");  break;
		case STR_PTR:    printf("(dword) to_ptr(");  break;
		default:         assert(false); // can't happen
	}
	node->print();
	printf(")");
}*/

/****************************************************************************
	BinOperatorNode
 ****************************************************************************/

void BinOperatorNode::print_unk(Console &o, const uint32 isize) const
{
	assert(rtype().type()!=Type::T_INVALID);
	assert(lnode!=0);
	assert(rnode!=0);
	
	lnode->print_unk(o, isize);
	Node::print_linenum_unk(o, isize);
	switch(otype)
	{
		case M_ADD: o.Printf(" + ");  break;
		case M_SUB: o.Printf(" - ");  break;
		case M_MUL: o.Printf(" * ");  break;
		case M_CMP: o.Printf(" == "); break;
		case M_LT:  o.Printf(" < ");  break;
		case M_LE:  o.Printf(" <= "); break;
		case M_GT:  o.Printf(" > ");  break;
		case M_AND:  o.Printf(" and "); break;
		case M_OR:  o.Printf(" or "); break;
		case M_NE: o.Printf(" != "); break;
		case M_IMPLIES: o.Printf(" <=> "); break;
		default: assert(false); // can't happen
	}
	rnode->print_unk(o, isize);
}

void BinOperatorNode::print_asm(Console &o) const
{
	assert(rtype().type()!=Type::T_INVALID);
	assert(lnode!=0);
	assert(rnode!=0);
	
	Node::print_linenum_asm(o);
	lnode->print_asm(o);
	o.Putchar('\n');
	rnode->print_asm(o);
	o.Putchar('\n');
	Node::print_asm(o);
	switch(otype)
	{
		case M_ADD: o.Printf("add"); break;
		case M_SUB: o.Printf("sub"); break;
		case M_MUL: o.Printf("mul"); break;
		case M_CMP: o.Printf("cmp"); break;
		case M_LT:  o.Printf("lt");  break;
		case M_LE:  o.Printf("le");  break;
		case M_GT:  o.Printf("gt");  break;
		case M_AND:  o.Printf("and");  break;
		case M_OR:  o.Printf("or");  break;
		case M_NE:  o.Printf("ne");  break;
		case M_IMPLIES: o.Printf("implies\t\t01 01"); break;
		default: assert(false); // can't happen
	}
}

void BinOperatorNode::print_bin(ODequeDataSource &o) const
{
	assert(rtype().type()!=Type::T_INVALID);
	assert(lnode!=0);
	assert(rnode!=0);
	
	Node::print_linenum_bin(o);
	lnode->print_bin(o);
	rnode->print_bin(o);
	switch(otype)
	{
		case M_ADD: o.write1(0x14); break;
		case M_SUB: o.write1(0x1C); break;
		case M_MUL: o.write1(0x1E); break;
		case M_CMP: o.write1(0x24); break;
		case M_LT:  o.write1(0x28); break;
		case M_LE:  o.write1(0x2A); break;
		case M_GT:  o.write1(0x2C); break;
		case M_AND: o.write1(0x32); break;
		case M_OR:  o.write1(0x34); break;
		case M_NE:  o.write1(0x36); break;
		case M_IMPLIES: o.write1(0x54); o.write2(0x0101); break;
		default: assert(false); // can't happen
	}
}

bool BinOperatorNode::fold(DCUnit *unit, std::deque<Node *> &nodes)
{
	switch(otype)
	{
		case M_ADD:	case M_SUB:	case M_MUL:
		case M_CMP:	case M_LT:	case M_LE:	case M_GT:  case M_NE:
		case M_AND: case M_OR:
			assert(acceptType(nodes.back()->rtype(), Type::T_WORD, Type::T_BYTE));
			grab_r(nodes);
			//fold_linenum(nodes);
			assert(acceptType(nodes.back()->rtype(), Type::T_WORD, Type::T_BYTE));
			grab_l(nodes);
			fold_linenum(nodes);
			rtype(Type::T_WORD);
			break;
		case M_IMPLIES:
			assert(acceptType(nodes.back()->rtype(), Type::T_WORD, Type::T_BYTE, Type::T_PID) || print_assert(this, unit));
			grab_r(nodes);
			//fold_linenum(nodes);
			assert(acceptType(nodes.back()->rtype(), Type::T_WORD, Type::T_BYTE, Type::T_PID) || print_assert(nodes.back(), unit) || print_assert(this, unit));
			grab_l(nodes);
			fold_linenum(nodes);
			rtype(Type::T_WORD);
			break;
		default: assert(print_assert(this, unit));
	}
	return true;
};

