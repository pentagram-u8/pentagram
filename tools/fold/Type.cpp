/*
 *	Type.cpp - General type object
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

#include "Type.h"

/****************************************************************************
	Type
 ****************************************************************************/

const char * const Type::_namearr[]={ "void", "byte", "word", "dword", "string", "pid", "var", "list", "slist", "strptr", "invalid" };

void Type::print_unk(Console &o) const
{
	o.Printf("(%s)", name());
	/*switch(_vtype)
	{
		case VT_VOID:   o.Printf("(void)");   return 0; break;
		case VT_BYTE:   o.Printf("(byte)");   return 2; break;
		case VT_WORD:   o.Printf("(word)");   return 4; break;
		case VT_DWORD:  o.Printf("(dword)");  return 8; break;
		case VT_STRING: o.Printf("(string)"); return 0; break;
		case VT_VPID:   o.Printf("(pid)");    return 0; break;
//		case VT_VAR:    o.Printf("(var)");    return 0; break;
		case VT_LIST:   o.Printf("(list)");   return 0; break;
		case VT_SLIST:  o.Printf("(slist)");  return 0; break;
		default:     assert(false);                break; // can't happen
	}
	assert(false); return 0; // can't happen
*/
}

/****************************************************************************
	DataType
 ****************************************************************************/

const char * const print_bp(const sint32 offset);
const char * const print_sp(const sint32 offset);

void DataType::print_value_unk(Console &o) const
{
	switch(_dtype)
	{
		case DT_NULL:     /* o.Printf(""); outputs 'nothing' */       break;
		case DT_BYTES:
			switch(_vtype.type())
			{
				case Type::T_BYTE:  o.Printf("0x%0*Xh", 2, static_cast<sint8 >(_value)); break;
				case Type::T_WORD:  o.Printf("0x%0*Xh", 4, static_cast<sint16>(_value)); break;
				case Type::T_DWORD: o.Printf("0x%0*Xh", 8, static_cast<sint32>(_value)); break;
				default: assert(false);
			}
			break;
		case DT_BP:        o.Printf("%s", print_bp(_value));          break;
		//case DT_BPLIST:    o.Printf("list(%s, %d)", print_bp(value), value); break;
		case DT_BPADDR:    o.Printf("addressof(%s)", print_bp(_value)); break;
		/*case DT_BPSTRPTR:  o.Printf("strptr(%s)", print_bp(value));    break;
		case DT_SP:        o.Printf("%s", print_sp(value));            break;
		case DT_SPADDR:    o.Printf("addressof(%s)", print_sp(value)); break;
		case DT_CHARS:     o.Printf("\"%s\"", strval.c_str());         break;
		case DT_DPID:      o.Printf("pid");                            break;
		case DT_PRESULT:   o.Printf("presult");                        break;
		case DT_RESULT:    o.Printf("result");                         break;
		case DT_GLOBAL:    o.Printf("global %s(%04X, %02X)",
			GlobalNames[global_offset].name.c_str(), global_offset, global_size); break;*/
		default:        assert(false); /* can't happen */         break;
	}
}

void DataType::print_value_asm(Console &o) const
{
	switch(_dtype)
	{
		//case DT_NULL:     /* o.Printf(""); outputs 'nothing' */       break;
		case DT_BYTES:
			switch(_vtype.type())
			{
				case Type::T_BYTE:  o.Printf("%0*Xh", 2, static_cast<sint8 >(_value)); break;
				case Type::T_WORD:  o.Printf("%0*Xh", 4, static_cast<sint16>(_value)); break;
				case Type::T_DWORD: o.Printf("%0*Xh", 8, static_cast<sint32>(_value)); break;
				default: assert(false);
			}
			break;
		case DT_BP:
			switch(_vtype.type())
			{
				case Type::T_WORD:
				case Type::T_DWORD: o.Printf("%s", print_bp(_value)); break;
				default: assert(false);
			}
			break;
		case DT_BPADDR:
			switch(_vtype.type())
			{
				case Type::T_DWORD: o.Printf("%s", print_bp(_value)); break;
				default: assert(false);
			}
			break;
		default: assert(false);
	}
}

void DataType::print_value_bin(OBufferDataSource &o) const
{
	switch(_dtype)
	{
		//case DT_NULL:     /* o.Printf(""); outputs 'nothing' */       break;
		case DT_BYTES:
			switch(_vtype.type())
			{
				case Type::T_BYTE:  o.write1(static_cast<sint8 >(_value)); break;
				case Type::T_WORD:  o.write2(static_cast<sint16>(_value)); break;
				case Type::T_DWORD: o.write4(static_cast<sint32>(_value)); break;
				default: assert(false);
			}
			break;
		case DT_BP:
			switch(_vtype.type())
			{
				case Type::T_WORD:
				case Type::T_DWORD: o.write1(_value); break;
				default: assert(false);
			}
			break;
		case DT_BPADDR:
			switch(_vtype.type())
			{
				case Type::T_DWORD: o.write1(_value); break;
				default: assert(false);
			}
			break;
		default: assert(false);
	}
}
