/*
 *	disasm.cc - Disassembler for U8's usecode
 *
 *  Copyright (C) 2001-2003 Willem Jan Palenstijn and The Pentagram Team
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
#include "util.h"

// define to drop debugging output everywhere
//#define DISASM_DEBUG

#ifdef DISASM_DEBUG
#include <iomanip>
#endif

/* A temporary opcode class as I slowly transition to returning pure Node* from
	the relevant readOp functions */
class TempOp
{
	public:
		TempOp(const uint32 offset_=0, const sint32 opcode_=0,
			const uint32 nextoffset_=0,
			const sint32 i0_=0, const sint32 i1_=0, const sint32 i2_=0,
			const sint32 i3_=0, const sint32 i4_=0, const string &str_=string())
			: offset(offset_), nextoffset(nextoffset_), i0(i0_), i1(i1_), i2(i2_),
			i3(i3_), i4(i4_), str(str_),
			sp_size(0), ret_size(0), opcode(opcode_) {};

		~TempOp() {};

		void op(const uint32 opcode_) { opcode = opcode_; };
		uint32 op() const { return opcode; };

		uint32 offset;
		uint32 nextoffset;
		sint32 i0, i1, i2, i3, i4;
		string str;

		// added for calli preprocessor
		uint32 sp_size;
		uint32 ret_size;

	protected:
		uint32 opcode;
};

/* curOffset is the current offset from the start of the current data section
   thus the specialised read functions */

uint32 curOffset;

inline uint32 read1(IDataSource *ucfile) { curOffset+=1; return ucfile->read1(); };
inline uint32 read2(IDataSource *ucfile) { curOffset+=2; return ucfile->read2(); };
inline uint32 read4(IDataSource *ucfile) { curOffset+=4; return ucfile->read4(); };
/* read string until null terninator */
inline std::string readstr(IDataSource *ucfile)
{
	string s;
	while(char c = static_cast<char>(read1(ucfile)))
		s += c;
//	for (uint32 i=0; i < 8; ++i)
//			 op.str += static_cast<char>(read1(ucfile));
//			if(read1(ucfile)!=0) assert(false); // trailing 0
	return s;
}
/* read 'n' characters into a string */
inline std::string readnstr(IDataSource *ucfile, uint32 n)
{
	string s;
	while(n-->0)
		s += static_cast<char>(read1(ucfile));
//	for (uint32 i=0; i < 8; ++i)
//			 op.str += static_cast<char>(read1(ucfile));
//			if(read1(ucfile)!=0) assert(false); // trailing 0
	return s;
}

std::map<sint32, string> ScriptExpressions;

map<uint32, uint32> EventMap;

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

#include "u8/ConvertUsecodeU8.h"
#include "crusader/ConvertUsecodeCrusader.h"

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
void printoverloads(IDataSource *ucfile, uint32 endpos);


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
string	outputdir;
bool	print_globals=false;
bool	strings_only=false;

bool crusader=false;

void readglobals(IDataSource *ucfile)
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
		con.Printf("[%04X %02X] (%s)\n", i->first, i->second.size, i->second.name.c_str());
}

/* Yes, this is icky and evil. *grin* But it works without modifying anything. */
#ifdef FOLD
	#define con_Printf if(print_disasm && !strings_only) con.Printf
	#define con_Printf_s if(print_disasm && strings_only) con.Printf("\n"); if(print_disasm) con.Printf
//	#define con_Printf con.Printf
#else
	#define con_Printf if (!strings_only) con.Printf
	#define con_Printf_s if (strings_only) con.Printf("\n"); con.Printf
#endif


// should be passed to readfunction, except that would make it dependaing upon Fold.cpp
// which is not what I'm intending atm.
#ifdef FOLD
#include "Folder.h"
Folder *folder = new Folder();
#endif

void just_print(TempOp &op, IDataSource *ucfile);
bool readfunction(IDataSource *ucfile, const char *name, const UsecodeHeader &uch, const uint32 func)
{
	std::string str;

	if (curOffset >= uch.maxOffset) return false;

	con_Printf("Func_%X", curOffset);

	std::map<uint32, uint32>::iterator it = EventMap.find(curOffset);
	if (it != EventMap.end())
	{
		if (it->second < 0x20)
			con_Printf(" (Event %X) %s::%s", it->second, name, convert->event_names()[it->second]);
		else 
			con_Printf(" (Event %X) %s::ordinal%02X", it->second, name, it->second);
	}
	else 
		con_Printf("  %s::%04X", name, curOffset);

	con_Printf(":\n");

	std::vector<DebugSymbol> debugSymbols;
	uint32 dbg_symbol_offset=0;
	bool done = false;
	
	#ifdef FOLD
	folder->SelectUnit(func);
	#endif
	
	while (!done)
	{
		//if (!ucfile->good())
		if(ucfile->getPos()>=ucfile->getSize())
			return false;

		TempOp op;
		#ifdef FOLD
		//foldops.push_back(FoldOp(op.offset, op.op(), op.nextoffset, op.i0, op.i1, op.i2, op.i3, op.i4, op.str));
		folder->fold(convert->readOp(ucfile, dbg_symbol_offset, debugSymbols, done));
		#else
		convert->readOp(op, ucfile, dbg_symbol_offset, debugSymbols, done);
		#endif
		just_print(op, ucfile);
		
	}
	//printbytes(ucfile, 20);
	return true;
}

void just_print(TempOp &op, IDataSource *ucfile)
{
		// point to the location of the opcode we just grabbed
		con_Printf("    %04X:", op.offset);

		con_Printf(" %02X\t", op.op());

		switch(op.op()) {

		// Poping to variables
		case 0x00:
			con_Printf("pop byte\t%s", print_bp(op.i0));
			break;
		case 0x01:
			con_Printf("pop\t\t%s", print_bp(op.i0));
			break;
		case 0x02:
			con_Printf("pop dword\t%s", print_bp(op.i0));
			break;
		case 0x03:
			con_Printf("pop huge\t%s %i", print_bp(op.i0), op.i1);
			break;

		case 0x08:
			con_Printf("pop res");
			break;
		case 0x09:
			con_Printf("pop element\t%s (%02X) slist==%02X", print_bp(op.i0), op.i1, op.i2);
			break;

		// Constant pushing
		case 0x0A:
			con_Printf("push byte\t%02Xh", op.i0);
			break;
		case 0x0B:
			con_Printf("push\t\t%04Xh", op.i0);
			break;
		case 0x0C:
			con_Printf("push dword\t%08Xh", op.i0);
			break;
		case 0x0D:
			
			if (strings_only)
			{
				con_Printf_s("%s\n\n",op.str.c_str());
			}
			else
			{
				con_Printf("push string\t\"%s\"", op.str.c_str());
			}
			break;
		case 0x0E:
			con_Printf("create list\t%02X (%02X)", op.i1, op.i0);
			break;

		// Usecode function and intrinsic calls
		case 0x0F:
			con_Printf("calli\t\t%02Xh %04Xh (%s)", op.i0, op.i1, convert->intrinsics()[op.i1]);
			break;
		case 0x11:
			con_Printf("call\t\t%04X:%04X (%s)", op.i0, op.i1,
				convert->UsecodeFunctionAddressToString(op.i0, op.i1, ucfile, crusader).c_str());
			break;
		case 0x12:
			con_Printf("pop\t\ttemp");
			break;

		case 0x14:
			con_Printf("add");
			break;
		case 0x15:
			con_Printf("add dword");
			break;
		case 0x16:
			con_Printf("concat");
			break;
		case 0x17:
			con_Printf("append");
			break;
		case 0x19:
			con_Printf("append slist\t(%02X)", op.i0);
			break;
		case 0x1A:
			con_Printf("remove slist\t(%02X)", op.i0);
			break;
		case 0x1B:
			con_Printf("remove list\t(%02X)", op.i0);
			break;
		case 0x1C:
			con_Printf("sub");
			break;
		case 0x1D:
			con_Printf("sub dword");
			break;
		case 0x1E:
			con_Printf("mul");
			break;
		case 0x1F:
			con_Printf("mul dword");
			break;
		case 0x20:
			con_Printf("div");
			break;
		case 0x21:
			con_Printf("div dword");
			break;
		case 0x22:
			con_Printf("mod");
			break;
		case 0x23:
			con_Printf("mod dword");
			assert(false); // Guessed opcode
			break;
		case 0x24:
			con_Printf("cmp");
			break;
		case 0x25:
			con_Printf("cmp dword");
			assert(false); // Guessed opcode
			break;
		case 0x26:
			con_Printf("strcmp");
			break;
		case 0x28:
			con_Printf("lt");
			break;
		case 0x29:
			con_Printf("lt dword");
			break;
		case 0x2A:
			con_Printf("le");
			break;
		case 0x2B:
			con_Printf("le dword");
			break;
		case 0x2C:
			con_Printf("gt");
			break;
		case 0x2D:
			con_Printf("gt dword");
			break;
		case 0x2E:
			con_Printf("ge");
			break;
		case 0x2F:
			con_Printf("ge dword");
			break;
		case 0x30:
			con_Printf("not");
			break;
		case 0x31:
			con_Printf("not dword");
			break;
		case 0x32:
			con_Printf("and");
			break;
		case 0x33:
			con_Printf("and dword");
			break;
		case 0x34:
			con_Printf("or");
			break;
		case 0x35:
			con_Printf("or dword");
			break;
		case 0x36:
			con_Printf("ne");
			break;
		case 0x37:
			con_Printf("ne dword");
			break;

		case 0x38:
			con_Printf("in list\t\t%02X slist==%02X", op.i0, op.i1);
			break;

		case 0x39:
			con_Printf("bit_and");
			break;
		case 0x3A:
			con_Printf("bit_or");
			break;
		case 0x3B:
			con_Printf("bit_not");
			break;
		case 0x3C:
			con_Printf("lsh");
			break;
		case 0x3D:
			con_Printf("rsh");
			break;

		case 0x3E:
			con_Printf("push byte\t%s", print_bp(op.i0));
			break;
		case 0x3F:
			con_Printf("push\t\t%s", print_bp(op.i0));
			break;
		case 0x40:
			con_Printf("push dword\t%s", print_bp(op.i0));
			break;
		case 0x41:
			con_Printf("push string\t%s", print_bp(op.i0));
			break;
		case 0x42:
			con_Printf("push list\t%s (%02X)", print_bp(op.i0), op.i1);
			break;
		case 0x43:
			con_Printf("push slist\t%s", print_bp(op.i0));
			break;
		case 0x44:
			con_Printf("push element\t(%02X) slist==%02X", op.i0, op.i1);
			break;
		case 0x45:
			con_Printf("push huge\t%02X %02X", op.i0, op.i1);
			break;
		case 0x4B:
			con_Printf("push addr\t%s", print_bp(op.i0));
			break;
		case 0x4C:
			con_Printf("push indirect\t%02Xh bytes", op.i0);
			break;
		case 0x4D:
			con_Printf("pop indirect\t%02Xh bytes", op.i0);
			break;

		case 0x4E:
			con_Printf("push\t\tglobal [%04X %02X] (%s)", op.i0, op.i1,
				GlobalNames[op.i0].name.c_str());
			break;
		case 0x4F:
			con_Printf("pop\t\tglobal [%04X %02X] (%s)", op.i0, op.i1,
				GlobalNames[op.i0].name.c_str());
			break;

		case 0x50:
			con_Printf("ret");
			break;
		case 0x51:
			con_Printf("jne\t\t%04Xh\t(to %04X)", op.i0, op.nextoffset + static_cast<short>(op.i0));
			break;
		case 0x52:
			con_Printf("jmp\t\t%04Xh\t(to %04X)", op.i0, op.nextoffset + static_cast<short>(op.i0));
			break;

		case 0x53:
			con_Printf("suspend");
			break;

		case 0x54:
			con_Printf("implies\t\t%02X %02X", op.i0, op.i1);
			break;

		case 0x57:
			con_Printf("spawn\t\t%02X %02X %04X:%04X (%s)",
				   op.i0, op.i1, op.i2, op.i3, convert->UsecodeFunctionAddressToString(op.i2, op.i3, ucfile, crusader).c_str());
			break;
		case 0x58:
			con_Printf("spawn inline\t%04X:%04X+%04X=%04X %02X %02X (%s+%04X)",
				   op.i0, op.i1, op.i2, op.i1+op.i2, op.i3, op.i4,
				   convert->UsecodeFunctionAddressToString(op.i0, op.i1, ucfile, crusader).c_str(), op.i2);
			break;
		case 0x59:
			con_Printf("push\t\tpid");
			break;

		case 0x5A:
			con_Printf("init\t\t%02X", op.i0);
			break;

		case 0x5B:
			con_Printf ("line number\t%i (%04Xh)", op.i0, op.i0);
			break;
		case 0x5C:
			con_Printf("symbol info\toffset %04Xh = \"%s\"", op.i0, op.str.c_str());
			break;

		case 0x5D:
			con_Printf("push byte\tretval");
			break;
		case 0x5E:
			con_Printf("push\t\tretval");
			break;
		case 0x5F:
			con_Printf("push dword\tretval");
			break;

		case 0x60:
			con_Printf("word to dword");
			break;
		case 0x61:
			con_Printf("dword to word");
			break;

		case 0x62:
			con_Printf("free string\t%s", print_bp(op.i0));
			break;
		case 0x63:
			con_Printf("free slist\t%s", print_bp(op.i0));
			break;
		case 0x64:
			con_Printf("free list\t%s", print_bp(op.i0));
			break;
		case 0x65:
			con_Printf("free string\t%s", print_sp(op.i0));
			break;
		case 0x66:
			con_Printf("free list\t%s", print_sp(op.i0));
			break;
		case 0x67:
			con_Printf("free slist\t%s", print_sp(op.i0));
			break;
		case 0x69:
			con_Printf("push strptr\t%s", print_bp(op.i0));
			break;
		case 0x6B:
			con_Printf("str to ptr");
			break;

		case 0x6C:
			con_Printf("param pid chg\t%s %02X", print_bp(op.i0), op.i1);
			break;

		case 0x6D:
			con_Printf("push dword\tprocess result");
			break;
		case 0x6E:
			con_Printf("add sp\t\t%s%02Xh", op.i0>0x7F?"-":"", op.i0>0x7F?0x100-op.i0:op.i0);
			break;
		case 0x6F:
			con_Printf("push addr\t%s", print_sp(0x100 - op.i0));
			break;

		case 0x70:
			con_Printf("loop\t\t%s %02X %02X", print_bp(op.i0), op.i1, op.i2);
			break;
		case 0x73:
			con_Printf("loopnext");
			break;
		case 0x74:
			con_Printf("loopscr\t\t%02X \"%c\" - %s", op.i0, static_cast<char>(op.i0),ScriptExpressions[op.i0].c_str());
			break;

		case 0x75:
			con_Printf("foreach list\t%s (%02X) %04X", print_bp(op.i0), op.i1, op.i2);
			break;

		case 0x76:
			con_Printf("foreach slist\t%s (%02X) %04X", print_bp(op.i0), op.i1, op.i2);
			break;

		case 0x77:
			con_Printf("set info");
			break;

		case 0x78:
			con_Printf("process exclude");
			break;

		case 0x79:
			con_Printf("global_address\t%04X", op.i0);
			break;

		case 0x7A:
			con_Printf("end");

			if(op.str.size())
			{
				con_Printf("\tsize of dbg symbols %04X", op.str.size());
			}

			break;

		// can't happen.
		default:
			con_Printf("db\t\t%02x", op.op());
			assert(false);
		}
		con_Printf("\n");
}


void printfunc(const uint32 func, const uint32 nameoffset, IDataSource *ucfile)
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

	con_Printf_s("Usecode class %d (%04X %s)\n", func, func, namebuf);

	convert->readevents(ucfile, uch);

	while (readfunction(ucfile, namebuf, uch, func));

	#ifdef FOLD
	folder->FinalUnit();
	//fold(func);
	//folder->print_unk(pout);
	folder->print_asm(con);
	folder->print_unk(con);
	//delete folder;
	//folder = new Folder();
	//clearfolding(); // clear in case we need to do another function
	#endif
}
#undef con_Printf // undef the evil define

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
			perr << gamelanguage << " entry point for " << *it << " not found" << std::endl;
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
		perr << "Usage: " << argv[0] << " <file> [<function number>|-a] {--game [u8|crusader]} {--lang [english|german|french|japanese]} {--odir <directory>}" << endl;
		perr << "or" << endl;
		perr << "Usage: " << argv[0] << " <file> -l" << endl;
		perr << "or" << endl;
		perr << "Usage: " << argv[0] << " <file> --globals {--game [u8|crusader]}" << endl;
		// Overload Table
		perr << "or" << endl;
		perr << "Usage: " << argv[0] << " <file> -overload" << endl;
		return 1;
	}

	con.DisableWordWrap();
	
	Args parameters;
	
	parameters.declare("--lang",    &gamelanguage,  "unknown");
	parameters.declare("--game",    &gametype,      "none");
	parameters.declare("--odir",    &outputdir,     "");
	parameters.declare("--globals", &print_globals, true);
	#ifdef FOLD
	parameters.declare("--disasm",  &print_disasm,  true);
	#endif
	parameters.declare("--strings", &strings_only,  true);

	parameters.process(argc, argv);

	// Filename with stripped path
	char *stripped_filename = argv[1];

	// Search for last ':', '/' or '\\'
	for(uint32 i = 0; argv[1][i]; ++i)
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
			case 'j': case 'J':
				gamelanguage = "japanese";
				break;
			default:
				perr << "Could not determine game language. Defaulting to english." << std::endl;
				gamelanguage = "english";
		}
	}
	if ( (gamelanguage != "german") && (gamelanguage != "english") && (gamelanguage != "french") && (gamelanguage != "japanese"))
	{
		perr << "Warning: unknown language specified (" << gamelanguage << ")." << std::endl;
	}

	// Create filesystem object
	FileSystem filesys(true);

	// Load usecode file
	IDataSource *ucfile = filesys.ReadFile(argv[1]);

	// Uh oh, couldn't load it
	if(ucfile==0)
	{
		perr << "Error reading file \"" << argv[1] << "\"" << endl;
		return 1;
	}

	// Force crusader mode if we are reading overload.dat
	if (!Pentagram::strcasecmp(stripped_filename, "overload.dat"))
	{
		pout << "Using \"overload.dat\". Forcing crusader gametype." << std::endl;
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
	pout << "Game type: " << gametype << std::endl <<
		"Language: " << gamelanguage << std::endl;

	if(outputdir.size())
		pout << "Output Directory: `" << outputdir << "`" << endl;
	
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
	if (!Pentagram::strcasecmp(argv[2], "-l")) {

		pout << "Listing classes..." << endl << endl;

		// Read num entries
		ucfile->seek( 0x54);
		uint32 entries = read4(ucfile)-2;

		ucfile->seek(0x80 + 8);
		sint32 nameoffset = read4(ucfile);
		/*sint32 namelength =*/ read4(ucfile);

		pout.setf(std::ios::uppercase);

		con.Printf("Class         Name     Offset     Routines   MaxOffset  Offset     ExternTab  FixupTab\n");

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

			//cout << "Usecode function " << func << " (0x" << std::hex << func
			//	<< std::dec << ") (" << namebuf << ")" << endl;

			ucfile->seek(offset);
			UsecodeHeader uch;
			convert->readheader(ucfile, uch, curOffset);
			
			con.Printf("%4d (0x%04X) %-8s 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08d\n",
				func, func, namebuf, offset, uch.routines, uch.maxOffset,
				uch.offset, uch.externTable, uch.fixupTable, uch.fixupTable - uch.externTable);
			
			++actual_num;
		}
		pout << endl << actual_num << " Usecode classes" << endl;
		return 0;
	}
	// Overload Table
	else if (!Pentagram::strcasecmp(argv[2], "-overload")) {
		uint32 end;

		// it's overload.dat
		if (!Pentagram::strcasecmp(stripped_filename, "overload.dat"))
		{
			end = ucfile->getSize();
			ucfile->seek(0);
		}
		// We want usecode entry 1
		else 
		{
			ucfile->seek(0x88);
			uint32 offset = ucfile->read4();
			end = ucfile->read4();
			ucfile->seek(offset);
			end += offset;
		}

		printoverloads(ucfile, end);
		return 0;
	}

	pout.setf(std::ios::uppercase);
	pout << std::hex;

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
	/*uint32 namelength =*/ read4(ucfile);

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
void printoverloads(IDataSource *ucfile, uint32 endpos)
{
	con.Printf ("Overload Table:\n");

	uint32 f, i = 0;
	uint32 all_mask = 0;

	while ((ucfile->getPos()) < endpos) {
		uint32 mask;
		char classname[9];

		mask = ucfile->read4();
		ucfile->read(classname, 9);

		if (classname[0]) {
			con.Printf ("%04d: %8s ", i, classname);

			for (f=0; f<32; f++) con.Printf (" %i",(mask>>f) & 1);

			con.Printf ("\n");
		}
		all_mask |= mask;
		i++;
	}
	con.Printf ("\n%i functions ", i);

	con.Printf ("\nAll functions: ");

	for (f=0; f<32; f++) con.Printf (" %i",(all_mask>>f) & 1);

	con.Printf ("\n\nEvents used:\n");
	for (f=0; f<32; f++) {
		if ((all_mask>>f) & 1) con.Printf (" %i: %s\n", f, convert->event_names()[f]);
	}
}
