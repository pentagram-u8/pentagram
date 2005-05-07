/*
 *	Type.h - General type object
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

#ifndef TYPE_H
#define TYPE_H

#include "Console.h"
#include "ODataSource.h"
 
class Type
{
	public:
		enum ttype { T_VOID=0, T_BYTE, T_WORD, T_DWORD, T_STRING,
			T_PID, T_VAR, T_LIST, T_SLIST, T_STRPTR, T_INVALID };

		Type(const ttype t) : _type(t) {};
		Type() : _type(T_INVALID) {};

		inline Type &operator=(const ttype t) { _type = t; return *this; };
		inline Type &operator=(const Type &t) { _type = t._type; return *this; };
		
		inline bool operator==(const ttype t) const
		{
			if( (_type==T_BYTE || _type==T_WORD) && (t==T_BYTE || t==T_WORD) )
				return true;
			else
				return _type == t;
		};
		inline bool operator!=(const ttype t) const { return !(*this == t); };
		inline bool operator==(const Type &t) const { return *this == t._type; };
		inline bool operator!=(const Type &t) const { return *this != t._type; };

		inline ttype type() const { return _type; };
		inline const char *name() const { return _namearr[_type]; };
		inline uint32 size() const
		{
			switch(_type)
			{
				case T_VOID:	return 0; break;
				case T_BYTE:	return 2; break;
				case T_WORD:	return 2; break;
				case T_DWORD:	return 4; break;
				case T_STRING:	return 2; break;
				case T_PID:		return 2; break; // maybe?
				case T_VAR:		assert(false); break; // can't handle this atm
				case T_LIST:	return 2; break; // 'almost' correct
				case T_SLIST:	return 2; break;
				case T_STRPTR:	return 4; break;
				case T_INVALID:	return 0; break; // it's not possible for this to return
				default: assert(false); // can't happen;
			}
			return 0; // can't happen
		};
		void print_unk(Console &o) const;
		
	private:
		ttype _type;
		static const char * const _namearr[];
};

class DataType
{
	public:
		enum datatype { DT_NULL, DT_BYTES, DT_BP, DT_BPLIST, DT_BPADDR, DT_BPSTRPTR,
			DT_SP, DT_SPADDR, DT_STRING, DT_PID, DT_PRESULT, DT_RESULT, DT_GLOBAL, DT_TEMP };
		
		DataType(const Type &newVType=Type::T_VOID, const datatype newDType=DT_NULL, const sint32 newValue=0)
			: _vtype(newVType), _dtype(newDType), _value(newValue) {};
		DataType(const Type::ttype &newVType, const datatype newDType, const std::string &newStrValue)
			: _vtype(newVType), _dtype(newDType), _strvalue(newStrValue) {};
		DataType(const Type::ttype &newVType, const datatype newDType, const uint32 var, const uint32 varIndex)
			: _vtype(newVType), _dtype(newDType), _value(var), _valueIndex(varIndex) {};

		const Type &type() const { return _vtype; };
		const datatype &dtype() const { return _dtype; };
		sint32 value() const { return _value; };
		
		void print_type_unk(Console &o) const { _vtype.print_unk(o); };
		void print_value_unk(Console &o) const;
		void print_value_asm(Console &o) const;
		void print_value_bin(ODequeDataSource &o) const;
		
	private:
		Type		_vtype;
		datatype	_dtype;
		sint32		_value;
		std::string _strvalue;
		uint32		_valueIndex; // for globals
};


/* FIXME: conveniently placed functions... */

namespace suc
{
	inline const char * const print_bp(const sint32 offset)
	{
		static char str[32];
		snprintf(str, 32, "[BP%c%02Xh]", offset>0x7F?'-':'+', offset>0x7F?0x100-offset:offset);
		return str;
	}
	
	inline const char * const print_sp(const sint32 offset)
	{
		static char str[32];
		snprintf(str, 32, "[SP%c%02Xh]", offset>0x7F?'-':'+', offset>0x7F?0x100-offset:offset);
		return str;
	}
}

inline bool acceptType(const Type &opcode, const Type::ttype want1)
	{ return ((opcode==want1) ? true : false); };
inline bool acceptType(const Type &opcode, const Type::ttype want1, const Type::ttype want2)
	{ return (acceptType(opcode, want1) ? true :
			(opcode==want2) ? true : false); };
inline bool acceptType(const Type &opcode, const Type::ttype want1, const Type::ttype want2, const Type::ttype want3)
	{ return (acceptType(opcode, want1, want2) ? true :
			(opcode==want3) ? true : false); };

#endif

