/*
 *	disasm.cc - Disassembler for U8's usecode
 *
 *  Copyright (C) 2001-2002 Willem Jan Palenstijn and The Pentagram Team
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

#include "pent_include.h"

#include <iostream>
#include <fstream>
#include <cstdio>

#include <map>
using	std::map;

#include <string>
using	std::string;

#include <algorithm>
#include <cctype>

#include "Args.h"
#if 0 // FIXME: Add conf/
#include "Configuration.h"
#endif
#include "common_types.h"

#include "IDataSource.h"
#include "FileSystem.h"
#include "Q_strcasecmp.h"

// define to drop debugging output everywhere
//#define DISASM_DEBUG

#ifdef DISASM_DEBUG
#include <iomanip>
#endif

class UsecodeHeader
{
	public:
		UsecodeHeader()
		: routines(0), maxOffset(0), offset(0), externTable(0), fixupTable(0) {};

		uint32 routines;
		uint32 maxOffset;
		uint32 offset;
		uint32 externTable;
		uint32 fixupTable;
};

/* curOffset is the current offset from the start of the current data section
   thus the specialised read functions */

uint32 curOffset;

inline uint32 read1(IFileDataSource *ucfile) { curOffset+=1; return ucfile->read1(); };
inline uint32 read2(IFileDataSource *ucfile) { curOffset+=2; return ucfile->read2(); };
inline uint32 read4(IFileDataSource *ucfile) { curOffset+=4; return ucfile->read4(); };

std::map<int, std::string> ScriptExpressions;

map<uint32, uint32> EventMap;

#include "u8/ConvertUsecodeU8.h"
#include "crusader/ConvertUsecodeCrusader.h"

using std::cout;
using std::cerr;
using std::endl;
using std::pair;

#ifndef HAVE_SNPRINTF
extern sint32 snprintf(char *, size_t, const char *, /*args*/ ...);
namespace std {
using ::snprintf;
}
#endif

/* this should be set depending upon what we're converting, but currently
	since we really only support u8 properly here, we'll just set it as the
	default and worry about it later */
ConvertUsecode *convert = new ConvertUsecodeU8();


// Overload Table
void printoverloads(IFileDataSource *ucfile, int endpos);


class GlobalName
{
	public:
		GlobalName(const uint32 _offset=0, const uint32 _size=0,
			const string _name=string())
		: offset(_offset), size(_size), name(_name) {};

		uint32	offset; //the offset into the char[]
		uint32	size; //the number of bytes stored in the global
		string	name; //the name of the global
};

/* Additional note: The maximum size of the globals array would be 64k. */
map<uint32, GlobalName> GlobalNames;

map<string, string> FuncNames;

string	gamelanguage;
string	gametype;
bool	print_globals=false;

bool crusader=false;

void readglobals(IFileDataSource *ucfile)
{
	char buf[60];
	char c;
	ucfile->seek(0x80);
	sint32 offset = read4(ucfile);
	sint32 length = read4(ucfile);

	sint32 curoff = 0;

	ucfile->seek(offset);
	while (curoff < length-1) {
		sint32 i = 0;
		do {
			c = read1(ucfile);
			++curoff;
			buf[i++] = c;
		} while (c != 0);
		uint32 i0 = read1(ucfile);
		uint32 i1 = read1(ucfile);
		uint32 i2 = read1(ucfile);
		read1(ucfile);
		curoff+=4;

		uint32 flag = i0 + (i1<<8);
		GlobalNames[flag] = GlobalName(flag, i2, buf);
	}
}

void printglobals()
{
	for(std::map<uint32, GlobalName>::iterator i=GlobalNames.begin(); i!=GlobalNames.end(); ++i)
		printf("[%04X %02X] (%s)\n", i->first, i->second.size, i->second.name.c_str());
}

/* This looks _real_ dubious. Instead of loading all the offsets from the files and converting
   them to pair<uint32, uint32>, we're storing them in memory as strings, then having to
   convert the class:offset pair into a string, and strcmping against them.
   So instead of having a 2*O(N) operation at read, and a 2*O(1)*O(logN) at search. We've got
   a O(N) operation at read, and a O(N)*O(logN) for _each_ search. */
string functionaddresstostring(const sint32 i0, const sint32 i1, IFileDataSource *ucfile)
{
	char buf[10];
	std::map<string, string>::iterator funcoffset = FuncNames.find(buf);

	snprintf(buf, 10, "%04X:%04X", i0, i1);
	funcoffset = FuncNames.find(buf);
	if (funcoffset != FuncNames.end())
		return funcoffset->second;

	// Attempt to grab function name of the requested func

	// Save the original pos
	uint32 origpos = ucfile->getPos();

	// Seek to index table entry 1
	ucfile->seek(0x80 + 8);

	// Get details
	sint32 offset = ucfile->read4();
	sint32 length = ucfile->read4();

	// Seek to name entry
	ucfile->seek(offset + i0*13 + 4);

	// Read name
	ucfile->read(buf, 9);

	// Return to the pos
	ucfile->seek(origpos);

	// Couldn't get name, just use number
	if (!buf[0]) snprintf(buf, 10, "%04X", i0);

	// String to return
	std::string str = buf;

	// This will only work in crusader
	if (crusader)
	{
		if (i1 < 0x20)
		{
			return str + "::" + convert->event_names()[i1];
		}
		else
		{
			snprintf(buf, 10, "%02X", i1);
			return str + "::ordinal" + buf;
		}
	}
	// For Ultima 8
	else
	{
		snprintf(buf, 10, "%04X", i1);
		return str + "::" + buf;
	}

	// Shouldn't ever get here
	return "unknown";
}

const char * const print_bp(const sint32 offset)
{
	static char str[32];
	snprintf(str, 32, "[BP%c%02Xh]", offset>0x7F?'-':'+', offset>0x7F?0x100-offset:offset);
	return str;
}

const char * const print_sp(const sint32 offset)
{
	static char str[32];
	snprintf(str, 32, "[SP%c%02Xh]", offset>0x7F?'-':'+', offset>0x7F?0x100-offset:offset);
	return str;
}

/* Yes, this is icky and evil. *grin* But it works without modifying anything. */
#ifdef FOLD
	#define printf if(print_disasm) printf
#endif

uint32 read_dbg_symbols(IFileDataSource *ucfile)
{
	uint32 count=read1(ucfile);

	for(uint32 i=0; i<count; ++i)
	{
		uint32 unknown1 = read1(ucfile);
		uint32 type     = read1(ucfile);
		uint32 unknown2 = read1(ucfile);
		uint32 unknown3 = read1(ucfile);
		std::string s;
		uint32 tchar;
		while ((tchar = read1(ucfile)))
			s += static_cast<char>(tchar);
		printf("%02X: %02X type=%02X (%c) %s (%02X) %02X %s\n",
			i, unknown1, type, type, print_bp(unknown2), unknown2, unknown3, s.c_str());
	}
	count = read1(ucfile);
	assert(count==0x7a); //end
	return count;
}

bool readfunction(IFileDataSource *ucfile, const char *name, const UsecodeHeader &uch)
{
	std::string str;

	if (curOffset >= uch.maxOffset) return false;

	printf("Func_%X", curOffset);

	std::map<uint32, uint32>::iterator it = EventMap.find(curOffset);
	if (it != EventMap.end())
	{
		if (it->second < 0x20)
			printf(" (Event %X) %s::%s", it->second, name, convert->event_names()[it->second]);
		else 
			printf(" (Event %X) %s::ordinal%02X", it->second, name, it->second);
	}
	else 
		printf("  %s::%04X", name, curOffset);

	printf(":\n");

	uint32 dbg_symbol_offset=0;
	bool done = false;
	sint32 i;

	while (!done) {
		short s0;
		sint32 i0=0, i1=0, i2=0, i3=0, i4=0;
		std::string str0;

		#ifdef FOLD
		uint32 startOffset = curOffset;
		#endif

		uint32 opcode;

		if(dbg_symbol_offset==curOffset)
			opcode = read_dbg_symbols(ucfile);
		else
			opcode = read1(ucfile);

		// point to the location of the opcode we just grabbed
		printf("    %04X:", curOffset-1);

		if (!ucfile->good())
			return false;

		/*
		  Guesses of opcodes. I'm reasonably sure about most of them,
		  but they're still guesses...
		  (Questionmarks generally indicate uncertainty)
		*/

		printf(" %02X\t", opcode);

		switch(opcode) {

		// Poping to variables
		case 0x00:
			// 00 xx
			// pop 8 bit int into bp+xx
			i0 = read1(ucfile);
			printf("pop byte\t%s", print_bp(i0));
			break;
		case 0x01:
			// 01 xx
			// pop 16 bit int into bp+xx
			i0 = read1(ucfile);
			printf("pop\t\t%s", print_bp(i0));
			break;
		case 0x02:
			// 02 xx
			// pop 32 bit int into bp+xx
			i0 = read1(ucfile);
			printf("pop dword\t%s", print_bp(i0));
			break;
		case 0x03:
			// 03 xx yy
			// pop yy bytes into bp+xx
			i0 = read1(ucfile); i1 = read1(ucfile);
			printf("pop huge\t%s %i", print_bp(i0), i1);
			break;

		case 0x08:
			// 08
			// pop 32bits into result register
			printf("pop res");
			break;
		case 0x09:
			// 09 xx yy zz
    			// pop yy bytes into an element of list bp+xx (or slist if zz is set).
			i0 = read1(ucfile); i1 = read1(ucfile); i2 = read1(ucfile);
			printf("pop element\t%s (%02X) slist==%02X", print_bp(i0), i1, i2);
			break;

		// Constant pushing
		case 0x0A:
			// 0A xx
			// push signed extended 8 bit xx onto the stack
			i0 = read1(ucfile);
			printf("push byte\t%02Xh", i0);
			break;
		case 0x0B:
			// 0B xx xx
			// push 16 bit xxxx onto the stack
			i0 = read2(ucfile);
			printf("push\t\t%04Xh", i0);
			break;
		case 0x0C:
			// 0C xx xx xx xx
			// push 32 bit xxxxxxxx onto the stack
			i0 = read4(ucfile);
			printf("push dword\t%08Xh", i0);
			break;
		case 0x0D:
			// 0D xx xx yy ... yy 00
			// push string (yy ... yy) of length xx xx onto the stack
			i0 = read2(ucfile);
			str0 = "";
			while ((i1 = read1(ucfile))) str0 += static_cast<char>(i1);
			printf("push string\t\"%s\"", str0.c_str());
			break;
		case 0x0E:
			// 0E xx yy
			// pop yy values of size xx from the stack and push the resulting list
			i0 = read1(ucfile);
			i1 = read1(ucfile);
			printf("create list\t%02X (%02X)", i1, i0);
			break;

		// Usecode function and intrinsic calls
		case 0x0F:
			// 0F xx yyyy
			// intrinsic call. xx is number of arguement bytes (includes this pointer)
			i0 = read1(ucfile);
			i1 = read2(ucfile);
			printf("calli\t\t%02Xh %04X (%s)", i0, i1, convert->intrinsics()[i1]);
			break;
		case 0x11:
			// 11 xx xx yy yy
			// call the function at offset yy yy of class xx xx
			i0 = read2(ucfile);
			i1 = read2(ucfile);
			printf("call\t\t%04X:%04X (%s)", i0, i1,
				functionaddresstostring(i0, i1, ucfile).c_str());
			break;
		case 0x12:
			// 12
			// pop 16bits into temp register
			printf("pop\t\ttemp");
			break;

		case 0x14:
			// 14
			// pop two values from the stack and push the sum
			printf("add");
			break;
		case 0x15:
			// 15
			// add two longs
			printf("add dword");
			break;
		case 0x16:
			// 16
			// pop two strings from the stack and push the concatenation
			printf("concat");
			break;
		case 0x17:
			// 17
			// pop two lists from the stack and push the 'sum' of the lists
			printf("append");
			break;
		case 0x19:
			// 19 02
			// add two stringlists
			i0 = read1(ucfile);
			printf("append slist\t(%02X)", i0);
			break;
		case 0x1A:
			// 1A
			// pop two string lists from the stack and remove the 2nd from the 1st
			i0 = read1(ucfile);
			printf("remove slist\t(%02X)", i0);
			break;
		case 0x1B:
			// 1B
			// pop two lists from the stack and remove the 2nd from the 1st
			i0 = read1(ucfile);
			printf("remove list\t(%02X)", i0);
			break;
		case 0x1C:
			// 1C
			// subtract two integers
			printf("sub");
			break;
		case 0x1D:
			// 1D
			// subtract two dwords
			printf("sub dword");
			break;
		case 0x1E:
			// 1E
			// multiply two integers
			printf("mul");
			break;
		case 0x1F:
			// 1F
			// multiply two dwords
			printf("mul dword");
			break;
		case 0x20:
			// 20
			// divide two integers
			printf("div");
			break;
		case 0x21:
			// 21
			// divide two dwords
			printf("div dword");
			break;
		case 0x22:
			// 22
			// mod
			printf("mod");
			break;
		case 0x23:
			// 23
			// mod long
			printf("mod dword");
			assert(false); // Guessed opcode
			break;
		case 0x24:
			// 24
			// compare two integers
			printf("cmp");
			break;
		case 0x25:
			// 24
			// compare two dwords
			printf("cmp dword");
			assert(false); // Guessed opcode
			break;
		case 0x26:
			// 26
			// compare two strings
			printf("strcmp");
			break;
		case 0x28:
			// 28
			// less than
			printf("lt");
			break;
		case 0x29:
			// 29
			// less than 32 bit
			printf("lt dword");
			break;
		case 0x2A:
			// 2A
			// less than or equal to
			printf("le");
			break;
		case 0x2B:
			// 2B
			// less than or equal to
			printf("le dword");
			break;
		case 0x2C:
			// 2C
			// greater than
			printf("gt");
			break;
		case 0x2D:
			// 2D
			// greater than 32 bit
			printf("gt dword");
			break;
		case 0x2E:
			// 2E
			// 'greater than or equal to'
			printf("ge");
			break;
		case 0x2F:
			// 2F
			// 'greater than or equal to' (longs)
			printf("ge dword");
			break;
		case 0x30:
			// 30
			// pops a boolean from the stack and pushes the boolean not
			printf("not");
			break;
		case 0x31:
			// 31
			// pops a boolean from the stack and pushes the boolean not
			printf("not dword");
			break;
		case 0x32:
			// 32
			// pops two booleans from the stack and pushes the boolean and
			printf("and");
			break;
		case 0x33:
			// 33
			// pops two booleans from the stack and pushes the boolean and
			printf("and dword");
			break;
		case 0x34:
			// 34
			// boolean or
			printf("or");
			break;
		case 0x35:
			// 35
			// boolean or
			printf("or dword");
			break;
		case 0x36:
			// 36
			// are two integers not equal?
			printf("ne");
			break;
		case 0x37:
			// 37
			// are two dwords not equal?
			printf("ne dword");
			break;

		case 0x38:
			// 38 xx yy
			// pops a list (or slist if yy==true) from the stack, then pops
			// a value from the stack that it needs to test if it's in the
			// list, pushing 'true' if it is, 'false' if it isn't. 'xx' is
			// the 'size' of each list element, as is true for most list
			// opcodes.
			i0 = read1(ucfile); i1 = read1(ucfile);
			printf("in list\t\t%02X slist==%02X", i0, i1);
			break;

		case 0x39:
			// 39
			// bitwise and
			printf("bit_and");
			break;
		case 0x3A:
			// 3A
			// bitwise or
			printf("bit_or");
			break;
		case 0x3B:
			// 3B
			// bitwise not
			printf("bit_not");
			break;
		case 0x3C:
			// 3C
    		// left shift
			printf("lsh");
			break;
		case 0x3D:
			// 3D
			// right shift
			printf("rsh");
			break;

		case 0x3E:
			// 3E xx
			// push the value of the 8 bit local var xx ??
			i0 = read1(ucfile);
			printf("push byte\t%s", print_bp(i0));
			break;
		case 0x3F:
			// 3F xx
			// push the value of the 16 bit local var xx
			i0 = read1(ucfile);
			printf("push\t\t%s", print_bp(i0));
			break;
		case 0x40:
			// 40 xx
			// push the value of the 32 bit local var xx ??
			i0 = read1(ucfile);
			printf("push dword\t%s", print_bp(i0));
			break;
		case 0x41:
			// 41 xx
			// push the string local var at BP+xx
			i0 = read1(ucfile);
			printf("push string\t%s", print_bp(i0));
			break;
		case 0x42:
			// 42 xx yy
			// push the list (with yy size elements) at BP+xx
			i0 = read1(ucfile); i1 = read1(ucfile);
			printf("push list\t%s (%02X)", print_bp(i0), i1);
			break;
		case 0x43:
			// 43 xx
			// push the stringlist local var at BP+xx
			i0 = read1(ucfile);
			printf("push slist\t%s", print_bp(i0));
			break;
		case 0x44:
			// 44 xx yy
			// push element from the second last var pushed onto the stack
			// (a list/slist), indexed by the last element pushed onto the list
			// (a byte/word). XX is the size of the types contained in the list
			// YY is true if it's a slist (for garbage collection)
			i0 = read1(ucfile); i1 = read1(ucfile);
			printf("push element\t(%02X) slist==%02X", i0, i1);
			break;
		case 0x45:
			// 45
			// push huge
			i0 = read1(ucfile); i1 = read1(ucfile);
			printf("push huge\t%02X %02X", i0, i1);
			break;
		case 0x4B:
			// 4B xx
			// push 32 pointer address of BP+XX
			i0 = read1(ucfile);
			printf("push addr\t%s", print_bp(i0));
			break;
		case 0x4C:
			// 4C xx
			// indirect push,
			// pops a 32 bit pointer off the stack and pushes xx bytes
			// from the location referenced by the pointer
			i0 = read1(ucfile);
			printf("push indirect\t%02Xh bytes", i0);
			break;
		case 0x4D:
			// 4D xx
			// indirect pop,
			// pops a 32 bit pointer off the stack and then pops xx bytes
			// into the location referenced by the pointer
			i0 = read1(ucfile);
			printf("pop indirect\t%02Xh bytes", i0);
			break;

		case 0x4E:
			// 4E xx xx yy
			// push global xx xx size yy
			i0 = read2(ucfile); i1 = read1(ucfile);
			printf("push\t\tglobal [%04X %02X] (%s)", i0, i1,
				GlobalNames[i0].name.c_str());
			break;
		case 0x4F:
			// 4F xx xx yy
			// pop value into global xx xx size yy
			i0 = read2(ucfile); i1 = read1(ucfile);
			printf("pop\t\tglobal [%04X %02X] (%s)", i0, i1,
				GlobalNames[i0].name.c_str());
			break;

		case 0x50:
			// 50
			// return from function
			printf("ret");
			break;
		case 0x51:
			// 51 xx xx
			// relative jump to xxxx if false
			i0 = read2(ucfile); s0 = static_cast<short>(i0);
			printf("jne\t\t%04Xh\t(to %04X)", i0, curOffset + s0);
			break;
		case 0x52:
			// 52 xx xx
			// relative jump to xxxx
			i0 = read2(ucfile); s0 = static_cast<short>(i0);
			printf("jmp\t\t%04Xh\t(to %04X)", i0, curOffset + s0);
			break;

		case 0x53:
			// 50
			// suspend function
			printf("suspend");
			break;

		case 0x54:
			// 54 xx yy
			// implies
			// this seems to link two processes, 'implying' that if one terminates,
			// the other does also, but it's mostly a guess. *grin*
			i0 = read1(ucfile); i1 = read1(ucfile);
			printf("implies\t\t%02X %02X", i0, i1);
			break;

		case 0x57:
			// 57 aa tt xx xx yy yy
			// spawn process function yyyy in class xxxx
			// aa = number of arg bytes pushed
			//      (not including this pointer which is 4 bytes)
			// tt = sizeof this pointer object
			i0 = read1(ucfile); i1 = read1(ucfile);
			i2 = read2(ucfile); i3 = read2(ucfile);
			printf("spawn\t\t%02X %02X %04X:%04X (%s)",
				   i0, i1, i2, i3, functionaddresstostring(i2,i3, ucfile).c_str());
			break;
		case 0x58:
			// 58 xx xx yy yy zz zz tt uu
			// spawn inline process function yyyy in class xxxx at offset zzzz
			// tt = size of this pointer
			// uu = unknown
			i0 = read2(ucfile); i1 = read2(ucfile);
			i2 = read2(ucfile);
			i3 = read1(ucfile); i4 = read1(ucfile);
			printf("spawn inline\t%04X:%04X+%04X=%04X %02X %02X (%s+%04X)",
				   i0, i1, i2, i1+i2, i3, i4,
				   functionaddresstostring(i0, i1, ucfile).c_str(), i2);
			break;
		case 0x59:
			// 59
			// push process id of current process
			printf("push\t\tpid");
			break;

		case 0x5A:
			// 5A xx
			// init function. xx = local var size
			// sets xx bytes on stack to 0, and moves sp by xx
			i0 = read1(ucfile);
			printf("init\t\t%02X", i0);
			break;

		case 0x5B:
			// 5B xx xx
			// the current sourcecode line number
			i0 = read2(ucfile);
			printf ("line number\t%i (%04X)", i0, i0);
			break;
		case 0x5C:
			// 5C xx xx yy yy yy yy yy yy yy yy 00
			// debugging symbol information
			// xxxx is the offset to one past the last 'ret' in the function, which
			// will be pointing to an 0x7a opcode if there is no debug info, else
			// to the first byte of the debug info.
			// yy .. yy is the class' name
			i0 = read2(ucfile);
			i0 = curOffset + (static_cast<short>(i0));
			str0 = "";
			for (i=0; i < 8; ++i)
			 str0 += static_cast<char>(read1(ucfile));
			if(read1(ucfile)!=0) assert(false); // trailing 0
			printf("symbol info\toffset %04x = \"%s\"", i0, str0.c_str());
			dbg_symbol_offset = i0; // the offset to the raw symbol data.
				// nothing between it and the 0x7a opcode is opcodes
			break;

		case 0x5D:
			// 5D
			// push 8 bit value returned from function call
			printf("push byte\tretval");
			break;
		case 0x5E:
			// 5E
			// push 16 bit value returned from function call
			printf("push\t\tretval");
			break;
		case 0x5F:
			// 5F
			// push 32 bit value returned from function call?
			printf("push dword\tretval");
			break;

		case 0x60:
			// 60
			// word to dword (sign extend)
			printf("word to dword");
			break;
		case 0x61:
			// 61
			// dword to word
			printf("dword to word");
			break;

		case 0x62:
			// 62 xx
			// free the string in var BP+xx
			i0 = read1(ucfile);
			printf("free string\t%s", print_bp(i0));
			break;
		case 0x63:
			// 63 xx
			// free the list in var BP+xx
			// (This one seems to be similar to 0x64 but only used for lists
			//  of strings?)
			i0 = read1(ucfile);
			printf("free slist\t%s", print_bp(i0));
			break;
		case 0x64:
			// 64 xx
			// free the list in var BP+xx
			i0 = read1(ucfile);
			printf("free list\t%s", print_bp(i0));
			break;
		case 0x65:
			// 65 xx
			// free string at SP+xx
			i0 = read1(ucfile);
			printf("free string\t%s", print_sp(i0));
			break;
		case 0x66:
			// 66 xx
			// free the list at SP+xx
			i0 = read1(ucfile);
			printf("free list\t%s", print_sp(i0));
			break;
		case 0x67:
			// 66 xx
			// free the string list at SP+xx
			i0 = read1(ucfile);
			printf("free slist\t%s", print_sp(i0));
			break;
		case 0x69:
			// 69 xx
			// push the string in var BP+xx as 32 bit pointer
			i0 = read1(ucfile);
			printf("push strptr\t%s", print_bp(i0));
			break;
		case 0x6B:
			// 6B
			// pop a string and push 32 bit pointer to string
			printf("str to ptr");
			break;

// Just my ramblings on 0x6C. It's a bit of a mess since it's taken from an irc log
// but it's the best documentation we have on it at the moment. *grin*

/* Anyway, the number at the end (01 in the above example) means the type of
pointer the offset points to. 01==string ptr, 02==string list ptr, 03==list
ptr. 01 and 03 are verified, 02 is an educated guess, and I'm pretty sure it's
a BP+xxh value too from looking at the usecode.
 *lightbulb* It looks like it may have been used for pointer/reference
stuff. For example:
 When a 'string' type is created, the actual string is stored in a Yamm
class instantiation local to the current thread, only the reference to it is
stored on the stack.
 When a pointer to this string is passed to another newly created thread,
the data pointed to in the original thread's Yamm needs to be copied to the
newly constructed thread, incase the original thread terminates, (or the
function just returns destroying the data), before the spawned thread uses it.
 This is where Mr 0x6C comes in. He takes a pointer to the
oritinal variable, copies the data from the Parent Yamm to it's (the Child's)
Yamm.
 Eg: The call in Class 007C:
     371F: 59    push            pid
     3720: 42    push list       [BP-0Ah] (02)
     3723: 42    push list       [BP-0Ch] (02)
     3726: 42    push list       [BP-0Eh] (02)
     3729: 41    push string     [BP-02h]
     372B: 40    push dword      [BP+06h]
     372D: 57    spawn           08 02 007C:3757 (unknown)
     3734: 66    free list       [SP+06h]
     3736: 66    free list       [SP+04h]
     3738: 66    free list       [SP+02h]
     373A: 65    free string     [SP+00h]
     373C: 6E    add sp          -08h
     373E: 5E    push            retval
 Then the start of the newly spawned function:
 Func_3757:
     3757: 5A    init            0D
     3759: 6C    6C              [BP+0Ah] 01
     375C: 6C    6C              [BP+0Ch] 03
     375F: 6C    6C              [BP+0Eh] 03
     3762: 6C    6C              [BP+10h] 03
     3765: 0A    push byte       01h
 The type value (01/02/03) at the end tells the opcode what type of data
it's copying or reconstructing. And if you look closely, you'll note there's a
'tiny' flaw in this opcode. In all the other list (not string list) related
opcodes another value is passed, the size of the datatype stored in the array.
This opcode lacks that value, and thus assumes that all datatypes for 03
(list) are 2 bytes wide.
 Of course, this also tells us something about how the processes were
executed and/or queued. Either:
 1) When a new thread is created by a spawn, it is immediately executed
until it hits the first opcode that 'sleeps' it until the next cycle, and that
the 'init' and 0x6C opcodes are not 'sleep' inducing opcodes.
 Or 2) The 'spawn' is a 'sleep' inducing opcode, and the new process is
scheduled to be executed before the 'current' thread in the queue (whether it
just be inserted before, or first, who knows *grin*). That way upon the next
awakening of all the threads, the spawned thread gets the chance to copy all
the relevant data from it's parent thread, before it's parent has a chance to
terminate.
 Also in the second case, as in the first case, the 'init' and
'0x6C' opcodes aren't 'sleep' inducing.

The general consensus is that the new threads are executed instantly upon creation,
and that the child threads are indeed placed infront of the parent thread.
*/
		case 0x6C:
			// 6C xx yy
			// looks to be a BP+XX function... maybe not
			i0 = read1(ucfile); i1 = read1(ucfile);
			printf("param pid change\t\t%s %02X", print_bp(i0), i1);
			break;

		case 0x6D:
			// 6D
			// push result of process
			printf("push dword\tprocess result");
			break;
		case 0x6E:
			// 6E xx
			// add xx to stack pointer
			i0 = read1(ucfile);
			printf("add sp\t\t%s%02Xh", i0>0x7F?"-":"", i0>0x7F?0x100-i0:i0);
			break;
		case 0x6F:
			// 6F xx
			// push 32 pointer address of SP-xx
			i0 = read1(ucfile);
			printf("push addr\t%s", print_sp(0x100 - i0));
			break;

		// loop-related opcodes
		// Theory: put a 'container object' on the stack, and this will
		// loop over the objects in there. The 'loopscript' determines
		// which objects are selected. (By a simple 'shape == x, frame == y'
		// thing, it seems)
		// See the abacus code (function 375) for a simple example

		case 0x70:
			// 70 xx yy zz
			// loop something. Stores 'current object' in var xx
			// yy == num bytes in string
			// zz == type
			i0 = read1(ucfile); i1 = read1(ucfile); i2 = read1(ucfile);
			printf("loop\t\t%s %02X %02X", print_bp(i0), i1, i2);
			break;
		case 0x73:
			// 73
			// next loop object? pushes false if end reached
			printf("loopnext");
			break;
		case 0x74:
			// 74 xx
			// add xx to the current 'loopscript'
			i0 = read1(ucfile);
			printf("loopscr\t\t%02X \"%c\" - %s", i0, static_cast<char>(i0),ScriptExpressions[i0].c_str());
			break;

		// 75 appears to have something to do with lists, looks like an enum/next from u7...
		case 0x75:
			// 75 xx yy zz zz
			// xx appears to be the location to store 'current' value from the
			//   list (BP+xx)
			// yy is the 'datasize' of the list, identical to the second parameter
			//   of the create list/slist opcodes
			// zzzz appears to be the offset to jump to after it's finished the
			//   iteration, the opcode before is a 'jmp' to the original position
			//   of the opcode.
			// (all guesses from Remorse1.21 usecode, _may_ be different in u8,
			//   unlikely though)
			// the way it appears to operate is it pops a 'word' off the stack
			//   (maximum number of items to iterate through? No idea, I've only
			//   seen 0xFFFF pushed before it (in Remorse1.21)), then pops
			//   the 'list' off to iterate through
			i0 = read1(ucfile); i1 = read1(ucfile); i2 = read2(ucfile);
			printf("foreach list\t%s (%02X) %04X", print_bp(i0), i1, i2);
			break;

		/* 76 appears to be identical to 0x75, except it operates on slists */
		case 0x76:
			// 75 xx yy zz zz
			i0 = read1(ucfile); i1 = read1(ucfile); i2 = read2(ucfile);
			printf("foreach slist\t%s (%02X) %04X", print_bp(i0), i1, i2);
			break;

		case 0x77:
			// 77
			// set info
			// assigns item number and ProcessType, both values popped from the stack
			printf("set info");
			break;

		case 0x78:
			// 78
			// process exclude
			// sets a flag on the object target of the 'current' function call that
			// said function call has exclusive use of the object until the call
			// returns.
			printf("process exclude");
			break;

		case 0x79:
			// 79
			// end of function
			if (crusader) // it's not eof for crusader...
			{
				i0 = read2(ucfile);
				printf("global_address\t%04X", i0);
			}
			else // but it is for U8
			{
				printf("end");
				done = true;
			}
			break;

			case 0x7A:
			// end of function (79 = u8, 7a = crusader)
			printf("end");

			if(str0.size())
			{
				printf("\tsize of dbg symbols %04X", str0.size());
			}

			done = true;
			break;

		// can't happen.
		default:
			printf("db\t\t%02x", opcode);
			assert(false);
		}
		printf("\n");

		#ifdef FOLD
		foldops.push_back(FoldOp(startOffset, opcode, curOffset, i0, i1, i2, i3, i4, str0));
		#endif
	}
	return true;
}

void printfunc(const uint32 func, const uint32 nameoffset, IFileDataSource *ucfile)
{
	ucfile->seek(0x80 + 8*(func+2));
	uint32 offset = read4(ucfile);
	uint32 length = read4(ucfile);

	if (length == 0) return;

	ucfile->seek(nameoffset + 4 + (13 * func));

	char namebuf[9];
	ucfile->read(namebuf, 9);

	ucfile->seek(offset);
	
	UsecodeHeader uch;
	
	convert->readheader(ucfile, uch, curOffset);

	printf("Usecode function %d (%04X %s)\n", func, func, namebuf);

	convert->readevents(ucfile, uch);

	while (readfunction(ucfile, namebuf, uch));

	#ifdef FOLD
	fold(func);
	printfolding();
	clearfolding(); // clear in case we need to do another function
	#endif
}
#undef printf // undef the evil define

void readfunctionnames(void)
{
#if 0 // FIXME: conf/
	Configuration config;
	std::vector<std::string> functionnames;
	std::string functionaddress;

	config.read_config_file("usecodemap.cfg");
	functionnames = config.listkeys("usecodemap",false);
	std::vector<std::string>::iterator it = functionnames.begin();
	while (it != functionnames.end())
	{
		config.value("usecodemap/" + *it + "/" + gamelanguage, functionaddress, "N/A");
		if (functionaddress == "N/A")
			std::cerr << gamelanguage << " entry point for " << *it << " not found" << std::endl;
		else
		{
			for(std::string::iterator i=functionaddress.begin(); i!=functionaddress.end(); ++i)
				std::toupper(*i);
//			std::transform(functionaddress.begin(), functionaddress.end(), functionaddress.begin(), std::toupper);
			FuncNames[functionaddress] = *it;
		}
		it++;
	}
#endif
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		cerr << "Usage: disasm <file> [<function number>|-a] {--game [u8|crusader]} {--lang [english|german|french]}" << endl;
		cerr << "or" << endl;
		cerr << "Usage: disasm <file> -l" << endl;
		cerr << "or" << endl;
		cerr << "Usage: disasm <file> --globals {--game [u8|crusader]}" << endl;
		// Overload Table
		cerr << "or" << endl;
		cerr << "Usage: disasm <file> -overload" << endl;
		return 1;
	}

	Args parameters;
	
	parameters.declare("--lang",    &gamelanguage,  "unknown");
	parameters.declare("--game",    &gametype,      "none");
	parameters.declare("--globals", &print_globals, true);
	#ifdef FOLD
	parameters.declare("--disasm",  &print_disasm,  true);
	#endif

	parameters.process(argc, argv);

	// Filename with stripped path
	char *stripped_filename = argv[1];

	// Search for last ':', '/' or '\\'
	for (int i = 0; argv[1][i]; i++)
		if (argv[1][i] == '\\' || argv[1][i] == '/' || argv[1][i] == ':') stripped_filename  = argv[1]+i+1;

	if (gamelanguage=="unknown")  // try to determine game language from file name
	{
		switch (stripped_filename[0])
		{
			case 'g': case 'G':
				gamelanguage = "german";
				break;
			case 'e': case 'E':
				gamelanguage = "english";
				break;
			case 'f': case 'F':
				gamelanguage = "french";
				break;
			default:
				std::cerr << "Could not determine game language. Defaulting to english." << std::endl;
				gamelanguage = "english";
		}
	}
	if ( (gamelanguage != "german") && (gamelanguage != "english") && (gamelanguage != "french"))
	{
		std::cerr << "Warning: unknown language specified (" << gamelanguage << ")." << std::endl;
	}

	// Create filesystem object
	FileSystem filesys;

	// Load usecode file
	IFileDataSource *ucfile = filesys.ReadFile(argv[1]);

	// Uh oh, couldn't load it
	if(ucfile==0)
	{
		cerr << "Error reading file \"" << argv[1] << "\"" << endl;
		return 1;
	}

	// Force crusader mode if we are reading overload.dat
	if (!Q_strcasecmp(stripped_filename, "overload.dat"))
	{
		std::cout << "Using \"overload.dat\". Forcing crusader gametype." << std::endl;
		gametype="crusader";
	}
	// Or if gametype is none, attempt to autodetect game type
	else if (gametype=="none")
	{
		ucfile->seek(0x80);
		sint32 nameoffset = read4(ucfile);
		sint32 namelength = read4(ucfile);

		// Looks like Ultima8 (has a global section)
		if (nameoffset && namelength) gametype="u8";
		// Must be crusader (no globals section)
		else gametype="crusader";
	}

	// Output the game type and langauge
	std::cout << "Game type: " << gametype << std::endl <<
		"Language: " << gamelanguage << std::endl;

	// setup crusader
	if (gametype=="crusader")
	{
		crusader=true;
		FORGET_OBJECT(convert);
		convert = new ConvertUsecodeCrusader();
	}

	// Read function names from cfg file
	readfunctionnames();

	// List functions
	if (!Q_strcasecmp(argv[2], "-l")) {

		cout << "Listing functions..." << endl << endl;

		// Read num entries
		ucfile->seek( 0x54);
		uint32 entries = read4(ucfile)-2;

		ucfile->seek(0x80 + 8);
		sint32 nameoffset = read4(ucfile);
		sint32 namelength = read4(ucfile);

		cout.setf(std::ios::uppercase);

		sint32 actual_num = 0;
		for(uint32 func = 0; func < entries; ++func)
		{
			ucfile->seek(0x80 + 8*(func+2));
			sint32 offset = read4(ucfile);
			sint32 length = read4(ucfile);

			ucfile->seek(nameoffset + 4 + (13 * func));
			char namebuf[9];
			ucfile->read(namebuf, 9);

			if (length == 0) continue;

			cout << "Usecode function " << func << " (0x" << std::hex << func
				<< std::dec << ") (" << namebuf << ")" << endl;
			++actual_num;
		}
		cout << endl << actual_num << " Usecode functions" << endl;
		return 0;
	}
	// Overload Table
	else if (!Q_strcasecmp(argv[2], "-overload")) {
		int end;

		// it's overload.dat
		if (!Q_strcasecmp(stripped_filename, "overload.dat"))
		{
			end = ucfile->getSize();
			ucfile->seek(0);
		}
		// We want usecode entry 1
		else 
		{
			ucfile->seek(0x88);
			int offset = ucfile->read4();
			end = ucfile->read4();
			ucfile->seek(offset);
			end += offset;
		}

		printoverloads(ucfile, end);
		return 0;
	}

	cout.setf(std::ios::uppercase);
	cout << std::hex;

	readglobals(ucfile);

	if(print_globals)
		printglobals();
	
	// Setup script expressions
	ScriptExpressions[0] = "false";
	ScriptExpressions[1] = "true";
	ScriptExpressions['$'] = "end";
	ScriptExpressions['%'] = "int";
	ScriptExpressions['&'] = "&&";
	ScriptExpressions['+'] = "||";
	ScriptExpressions['!'] = "!";
	ScriptExpressions['?'] = "item->status";
	ScriptExpressions['*'] = "item->q";
	ScriptExpressions['#'] = "item->npc_num";
	ScriptExpressions['='] = "==";
	ScriptExpressions['>'] = ">";
	ScriptExpressions['<'] = "<";
	ScriptExpressions[']'] = ">=";
	ScriptExpressions['['] = "<=";
	ScriptExpressions[':'] = "item->family";
	ScriptExpressions['@'] = "item->shape";
	ScriptExpressions['`'] = "item->frame";

	#ifdef FOLD
	initfolding();
	#endif

	ucfile->seek(0x80 + 8);
	uint32 nameoffset = read4(ucfile);
	uint32 namelength = read4(ucfile);

	if(std::strcmp(argv[2], "-a")==0)
	{
		ucfile->seek(0x54); // Seek to the 'start' of the FLEX
		uint32 entries = read4(ucfile)-2;
		
		for(uint32 func=0; func<entries; ++func)
			printfunc(func, nameoffset, ucfile);
	}
	
	uint32 func = std::strtol(argv[2], 0, 0);
	
	printfunc(func, nameoffset, ucfile);
	
	return 0;
}

// Overload Table
void printoverloads(IFileDataSource *ucfile, int endpos)
{
	std::printf ("Overload Table:\n");

	int f, i = 0;
	uint32 all_mask = 0;

	while ((ucfile->getPos()) < endpos) {
		uint32 mask;
		char classname[9];

		mask = ucfile->read4();
		ucfile->read(classname, 9);

		if (classname[0]) {
			std::printf ("%04d: %8s ", i, classname);

			for (f=0; f<32; f++) std::printf (" %i",(mask>>f) & 1);

			std::printf ("\n");
		}
		all_mask |= mask;
		i++;
	}
	std::printf ("\n%i functions ", i);

	std::printf ("\nAll functions: ");

	for (f=0; f<32; f++) std::printf (" %i",(all_mask>>f) & 1);

	std::printf ("\n\nEvents used:\n");
	for (f=0; f<32; f++) {
		if ((all_mask>>f) & 1) std::printf (" %i: %s\n", f, convert->event_names()[f]);
	}
}
