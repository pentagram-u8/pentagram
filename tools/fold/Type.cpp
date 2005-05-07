/*
 *	Type.cpp - General type object
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

#include "Type.h"

/****************************************************************************
	Type
 ****************************************************************************/

const char * const Type::_namearr[] =
	{ "void", "byte", "word", "dword",
	  "string", "pid", "var", "list",
	  "slist", "strptr", "invalid"
	};

void Type::print_unk(Console &o) const
{
	o.Printf("(%s)", name());
}

/****************************************************************************
	DataType
 ****************************************************************************/
//class GlobalName;

//extern std::map<uint32, GlobalName> GlobalNames;

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
		case DT_BP:        o.Printf("%s", suc::print_bp(_value));          break;
		//case DT_BPLIST:    o.Printf("list(%s, %d)", print_bp(value), value); break;
		case DT_BPADDR:    o.Printf("addressof(%s)", suc::print_bp(_value)); break;
		/*case DT_BPSTRPTR:  o.Printf("strptr(%s)", print_bp(value));    break;
		case DT_SP:        o.Printf("%s", print_sp(value));            break;
		case DT_SPADDR:    o.Printf("addressof(%s)", print_sp(value)); break;*/
		case DT_STRING:    o.Printf("\"%s\"", _strvalue.c_str());         break;
		case DT_PID:       o.Printf("pid");                            break;
		/*case DT_PRESULT:   o.Printf("presult");                        break;
		case DT_RESULT:    o.Printf("result");                         break;*/
		case DT_GLOBAL:    o.Printf("global %s(%04X, %02X)",
			"name", /*FIXME: Insert: GlobalNames[_value].name.c_str() <- here*/ _value, _valueIndex); break;
		case DT_TEMP:      o.Printf("temp");                      break;
		default:           assert(false); /* can't happen */      break;
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
				case Type::T_DWORD: o.Printf("%s", suc::print_bp(_value)); break;
				default: assert(false);
			}
			break;
		case DT_BPADDR:
			switch(_vtype.type())
			{
				case Type::T_DWORD: o.Printf("%s", suc::print_bp(_value)); break;
				default: assert(false);
			}
			break;
		case DT_STRING:
			switch(_vtype.type())
			{
				case Type::T_STRING: o.Printf("\"%s\"", _strvalue.c_str()); break;
				default: assert(false);
			}
			break;
		case DT_PID:
		case DT_TEMP:
			//do nothing, I think...
			break;
		case DT_GLOBAL:
			switch(_vtype.type())
			{
				case Type::T_WORD: o.Printf("[%04X %02X]", _value, _valueIndex); break;
				default: assert(false);
			}
			break;
			
		default: assert(false);
	}
}

void DataType::print_value_bin(ODequeDataSource &o) const
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
		case DT_STRING:
			switch(_vtype.type())
			{
				case Type::T_STRING:
					o.write2(_strvalue.size());
					for(uint16 i = 0; i<_strvalue.size(); i++)
						o.write1(_strvalue[i]);
					break;
				default: assert(false);
			}
			break;
		case DT_PID:
		case DT_TEMP:
			// do nothing, I think...
			break;
		case DT_GLOBAL:
			assert(_vtype.type()==Type::T_WORD);
			o.write2(_value);
			o.write1(_valueIndex);
			break;
		default: assert(false);
	}
}
