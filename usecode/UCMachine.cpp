/*
Copyright (C) 2002 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"

#include "UCMachine.h"
#include "UCProcess.h"

#define LOGPF(X) pout.printf X

UCMachine::UCMachine()
{
}


UCMachine::~UCMachine()
{
}

static const char *print_bp(const int offset)
{
	static char str[32];
	std::snprintf(str, 32, "[BP%c%02Xh]", offset>0x7F?'-':'+', offset>0x7F?0x100-offset:offset);
	return str;
}


bool UCMachine::ExecProcess(UCProcess* p)
{
	assert(p);

	pout << "running process " << p->pid << ", class " << p->classid << ", offset " << p->cs.getPos() << std::endl;

	bool cede = false;

	while(!cede)
	{
		uint8 opcode = p->cs.read1();

		LOGPF(("sp = %02X; %04X: %02X", p->stack.stacksize(),
			   p->cs.getPos(), opcode));

		sint8 si8a;
		uint16 ui16a, ui16b;
		uint32 ui32a, ui32b;
		sint16 si16a, si16b;
		sint32 si32a, si32b;

		switch(opcode)
		{

		// POP opcodes
		case 0x00:
			// 00 xx
			// pop 16 bit int, and assign LS 8 bit int into bp+xx
			si8a = static_cast<sint8>(p->cs.read1());
			ui16a = p->stack.pop2();
			p->stack.assign1(p->bp+si8a, ui16a);
			LOGPF(("pop byte\t%s = %02Xh", print_bp(si8a), ui16a));
			break;

		case 0x01:
			// 01 xx
			// pop 16 bit int into bp+xx
			si8a = static_cast<sint8>(p->cs.read1());
			ui16a = p->stack.pop2();
			p->stack.assign2(p->bp+si8a, ui16a);
			LOGPF(("pop\t\t%s = %04Xh", print_bp(si8a), ui16a));
			break;

		case 0x02:
			// 02 xx
			// pop 32 bit int into bp+xx
			si8a = static_cast<sint8>(p->cs.read1());
			ui32a = p->stack.pop4();
			p->stack.assign4(p->bp+si8a, ui32a);
			LOGPF(("pop dword\t%s = %08Xh", print_bp(si8a), ui32a));
			break;

		case 0x03:
			// 03 xx yy
			// pop yy bytes into bp+xx
			{
				si8a = static_cast<sint8>(p->cs.read1());
				uint8 size = p->cs.read1();
				uint8 buf[256];
				p->stack.pop(buf, size);
				p->stack.assign(p->bp+si8a, buf, size);
				LOGPF(("pop huge\t%s %i", print_bp(si8a), size));
			}
			break;

/*
		case 0x08:
			// 08
			// pop 16bits into result register
			printf("pop res");
			break;
		case 0x09:
			// 09 xx yy zz
			// pop yy bytes into an element of list bp+xx (or slist if zz is set).
			i0 = read1(in); i1 = read1(in); i2 = read1(in);
			printf("pop element\t%s (%02X) slist==%02X", print_bp(i0), i1, i2);
			break;
*/


		// PUSH opcodes

		case 0x0A:
			// 0A xx
			// push sign-extended 8 bit xx onto the stack as 16 bit
			ui16a = static_cast<sint8>(p->cs.read1());
			p->stack.push2(ui16a);
			LOGPF(("push byte\t%04Xh", ui16a));
			break;

		case 0x0B:
			// 0B xx xx
			// push 16 bit xxxx onto the stack
			ui16a = p->cs.read2();
			p->stack.push2(ui16a);
			LOGPF(("push\t\t%04Xh", ui16a));
			break;

		case 0x0C:
			// 0C xx xx xx xx
			// push 32 bit xxxxxxxx onto the stack
			ui32a = p->cs.read4();
			p->stack.push4(ui32a);
			LOGPF(("push dword\t%08Xh", ui32a));
			break;

/*
		case 0x0D:
			// 0D xx xx yy ... yy 00
			// push string (yy ... yy) of length xx xx onto the stack
			i0 = read2(in);
			str0 = "";
			while ((i1 = read1(in))) str0 += static_cast<char>(i1);
			printf("push string\t\"%s\"", str0.c_str());
			break;
		case 0x0E:
			// 0E xx yy
			// pop yy values of size xx from the stack and push the resulting list
			i0 = read1(in);
			i1 = read1(in);
			printf("create list\t%02X (%02X)", i1, i0);
			break;
*/

		// Usecode function and intrinsic calls

		case 0x0F:
			// 0F xx yyyy
			// intrinsic call. xx is number of argument bytes
			// (includes this pointer, if present)
			{
				// TODO
				uint16 arg_bytes = p->cs.read1();
				uint16 func = p->cs.read2();
				LOGPF(("!calli\t\t%04Xh (%02Xh arg bytes)", func, arg_bytes));
			}
			break;

		case 0x11:
			// 11 xx xx yy yy
			// call the function at offset yy yy of class xx xx
			{
				uint16 new_classid = p->cs.read2();
				uint16 new_offset = p->cs.read2();
				LOGPF(("call\t\t%04X:%04X", new_classid, new_offset));

				// Push old stuff
				p->stack.push2(p->classid);		// BP+04 Prev class
				p->stack.push2(p->cs.getPos());	// BP+02 Prev offset
				p->stack.push2(p->bp);			// BP+00 Prev BP

				// Update bp
				p->bp = p->stack.getSP();

				// Update the classid
				p->classid = new_classid;

				// Update the code segment
				p->cs.load(p->usecode->get_classdata(new_classid),
						   p->usecode->get_classdatasize(new_classid));
				p->cs.seek(new_offset);

				// Resume execution
			}
			break;

/*
		case 0x12:
			// 12
			// pop 16bits into temp register
			printf("pop\t\ttemp");
			break;
*/

		// Arithmetic

		case 0x14:
			// 14
			// 16 bit add
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			p->stack.push2(static_cast<uint16>(si16a + si16b));
			LOGPF(("add"));
			break;

		case 0x15:
			// 15
			// 32 bit add
			si32a = static_cast<sint32>(p->stack.pop4());
			si32b = static_cast<sint32>(p->stack.pop4());
			p->stack.push4(static_cast<uint32>(si32a + si32b));
			LOGPF(("add long"));
			break;

/*
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
			i0 = read1(in);
			printf("append slist\t(%02X)", i0);
			break;
		case 0x1A:
			// 1A
			// pop two string lists from the stack and remove the 2nd from the 1st
			i0 = read1(in);
			printf("remove slist\t(%02X)", i0);
			break;
		case 0x1B:
			// 1B
			// pop two lists from the stack and remove the 2nd from the 1st
			i0 = read1(in);
			printf("remove list\t(%02X)", i0);
			break;
*/

		case 0x1C:
			// 1C
			// subtract two 16 bit integers
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			p->stack.push2(static_cast<uint16>(si16b - si16a)); // !! order?
			LOGPF(("sub"));
			break;

		case 0x1D:
			// 1D
			// subtract two 32 bit integers
			si32a = static_cast<sint16>(p->stack.pop4());
			si32b = static_cast<sint16>(p->stack.pop4());
			p->stack.push4(static_cast<uint32>(si32b - si32a)); // !! order?
			LOGPF(("sub long"));
			break;

		case 0x1E:
			// 1E
			// multiply two 16 bit integers
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			p->stack.push2(static_cast<uint16>(si16a * si16b));
			LOGPF(("mul"));
			break;

		case 0x1F:
			// 1F
			// multiply two 32 bit integers
			si32a = static_cast<sint16>(p->stack.pop4());
			si32b = static_cast<sint16>(p->stack.pop4());
			p->stack.push4(static_cast<uint32>(si32a * si32b));
			LOGPF(("mul long"));
			break;

		case 0x20:
			// 20
			// divide two 16 bit integers    (order?)
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16a != 0) {
				p->stack.push2(static_cast<uint16>(si16b / si16a));
			} else {
				perr.printf("division by zero.\n");
				p->stack.push2(0);
			}
			LOGPF(("div"));
			break;

		case 0x21:
			// 21
			// divide two 32 bit integers    (order?)
			si32a = static_cast<sint16>(p->stack.pop4());
			si32b = static_cast<sint16>(p->stack.pop4());
			if (si32a != 0) {
				p->stack.push4(static_cast<uint32>(si32b / si32a));
			} else {
				perr.printf("division by zero.\n");
				p->stack.push4(0);
			}
			LOGPF(("div"));
			break;

		case 0x22:
			// 22
			// 16 bit mod    (order?)
			// is this a C-style %?
			// or return values between 0 and si16a-1 ?
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16a != 0) {
				p->stack.push2(static_cast<uint16>(si16b % si16a));
			} else {
				perr.printf("division by zero.\n");
				p->stack.push2(0);
			}
			LOGPF(("mod"));
			break;

		case 0x23:
			// 23
			// 32 bit mod   (order)?
			// also see 0x22
			si32a = static_cast<sint16>(p->stack.pop4());
			si32b = static_cast<sint16>(p->stack.pop4());
			if (si32a != 0) {
				p->stack.push4(static_cast<uint32>(si32b % si32a));
			} else {
				perr.printf("division by zero.\n");
				p->stack.push4(0);
			}
			LOGPF(("mod long"));
			break;

		case 0x24:
			// 24
			// 16 bit cmp
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16a == si16b) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("cmp"));
			break;

		case 0x25:
			// 25
			// 32 bit cmp
			si32a = static_cast<sint32>(p->stack.pop4());
			si32b = static_cast<sint32>(p->stack.pop4());
			if (si32a == si32b) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("cmp long"));
			break;

/*
		case 0x26:
			// 26
			// compare two strings
			printf("strcmp");
			break;
*/

		case 0x28:
			// 28
			// 16 bit less-than
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16b < si16a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("lt"));
			break;

		case 0x29:
			// 29
			// 32 bit less-than
			si32a = static_cast<sint32>(p->stack.pop4());
			si32b = static_cast<sint32>(p->stack.pop4());
			if (si32b < si32a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("cmp long"));
			break;

		case 0x2A:
			// 2A
			// 16 bit less-or-equal
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16b <= si16a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("le"));
			break;

		case 0x2B:
			// 2B
			// 32 bit less-or-equal
			si32a = static_cast<sint32>(p->stack.pop4());
			si32b = static_cast<sint32>(p->stack.pop4());
			if (si32b <= si32a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("le long"));
			break;

		case 0x2C:
			// 2C
			// 16 bit greater-than
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16b > si16a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("gt"));
			break;

		case 0x2D:
			// 2D
			// 32 bit greater-than
			si32a = static_cast<sint32>(p->stack.pop4());
			si32b = static_cast<sint32>(p->stack.pop4());
			if (si32b > si32a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("gt long"));
			break;

		case 0x2E:
			// 2E
			// 16 bit greater-or-equal
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16b >= si16a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("ge"));
			break;

		case 0x2F:
			// 2F
			// 32 bit greater-or-equal
			si32a = static_cast<sint32>(p->stack.pop4());
			si32b = static_cast<sint32>(p->stack.pop4());
			if (si32b >= si32a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("ge long"));
			break;

		case 0x30:
			// 30
			// 16 bit boolean not
			ui16a = p->stack.pop2();
			if (!ui16a) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("not"));
			break;

/*
		case 0x31:
			// 31
			// 32 bit boolean not
			printf("not dword");
			break;
*/

		case 0x32:
			// 32
			// 16 bit boolean and
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			if (ui16a && ui16b) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("and"));
			break;

/*
		case 0x33:
			// 33
			// 32 bit boolean and
			printf("and dword");
			break;
*/

		case 0x34:
			// 34
			// 16 bit boolean or
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			if (ui16a || ui16b) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("and"));
			break;

/*
		case 0x35:
			// 35
			// 32 bit boolean or
			printf("or dword");
			break;
*/

		case 0x36:
			// 36
			// 16 bit not-equal
			si16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			if (si16a != si16b) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("ne"));
			break;

		case 0x37:
			// 37
			// 32 bit not-equal
			si32a = static_cast<sint16>(p->stack.pop4());
			si32b = static_cast<sint16>(p->stack.pop4());
			if (si32a != si32b) {
				p->stack.push2(1);
			} else {
				p->stack.push2(0);
			}
			LOGPF(("ne long"));
			break;

/*
		case 0x38:
			// 38 xx yy
			// searches for an element in list xx (or slist if yy is true)
			i0 = read1(in); i1 = read1(in);
			printf("in list\t\t%s slist==%02X", print_bp(i0), i1);
			break;
*/

		case 0x39:
			// 39
			// 16 bit bitwise and
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			p->stack.push2(ui16a & ui16b);
			LOGPF(("bit_and"));
			break;

		case 0x3A:
			// 3A
			// 16 bit bitwise or
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			p->stack.push2(ui16a | ui16b);
			LOGPF(("bit_or"));
			break;

		case 0x3B:
			// 3B
			// 16 bit bitwise not
			ui16a = p->stack.pop2();
			p->stack.push2(~ui16a);
			LOGPF(("bit_not"));
			break;

		case 0x3C:
			// 3C
			// 16 bit left shift
			// operand order?
			ui16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			p->stack.push2(static_cast<uint16>(si16b >> ui16a));
			LOGPF(("lsh"));
			break;

		case 0x3D:
			// 3D
			// 16 bit right shift
			// !! sign-extend or not?
			// operand order?
			ui16a = static_cast<sint16>(p->stack.pop2());
			si16b = static_cast<sint16>(p->stack.pop2());
			p->stack.push2(static_cast<uint16>(si16b >> ui16a));
			LOGPF(("rsh"));
			break;

		case 0x3E:
			// 3E xx
			// push the value of the 8 bit local var xx
			si8a = static_cast<sint8>(p->cs.read1());
			ui16a = static_cast<sint8>(p->stack.access1(p->bp+si8a));
			p->stack.push2(ui16a);
			LOGPF(("push byte\t%s = %02Xh", print_bp(si8a), ui16a));
			break;

		case 0x3F:
			// 3F xx
			// push the value of the 16 bit local var xx
			si8a = static_cast<sint8>(p->cs.read1());
			ui16a = p->stack.access2(p->bp+si8a);
			p->stack.push2(ui16a);
			LOGPF(("push\t\t%s = %04Xh", print_bp(si8a), ui16a));
			break;

		case 0x40:
			// 40 xx
			// push the value of the 32 bit local var xx
			si8a = static_cast<sint8>(p->cs.read1());
			ui32a = p->stack.access4(p->bp+si8a);
			p->stack.push4(ui32a);
			LOGPF(("push dword\t%s = %08Xh", print_bp(si8a), ui32a));
			break;

/*
		case 0x41:
			// 41 xx
			// push the string local var xx
			printf("push string\t%s", print_bp(read1(in)));
			break;
		case 0x42:
			// 42 xx yy
			// push the list (with yy size elements) at BP+xx
			i0 = read1(in); i1 = read1(in);
			printf("push list\t%s (%02X)", print_bp(i0), i1);
			break;
		case 0x43:
			// 43 xx
			// push the stringlist local var xx
			printf("push slist\t%s", print_bp(read1(in)));
			break;
		case 0x44:
			// 44 xx yy
			// push element from the second last var pushed onto the stack
			// (a list/slist), indexed by the last element pushed onto the list
			// (a byte/word). XX is the size of the types contained in the list
			// YY is true if it's a slist (for garbage collection)
			i0 = read1(in); i1 = read1(in);
			printf("push element\t%s slist==%02X", print_bp(i0), i1);
			break;
		case 0x45:
			// 45
			// push huge
			i0 = read1(in); i1 = read1(in);
			printf("push huge\t%02X %02X", i0, i1);
			break;
		case 0x4B:
			// 4B xx
			// push 32 pointer address of BP+XX
			printf("push addr\t%s", print_bp(read1(in)));
			break;
*/

/*
		case 0x4C:
			// 4C xx
			// indirect push,
			// pops a 32 bit pointer off the stack and pushes xx bytes
			// from the location referenced by the pointer
			{
				// TODO
				uint8 count = ds.read1();
				uint16 offset = stack.pop2();
				uint16 segment = stack.pop2();
				// Push the bytes
				if (count == 2) stack.push2(0xCCCC);
				else stack.push0(count);
				LOGPF(("!push indirect\t%02Xh bytes", count));
			}
			break;
		case 0x4D:
			// 4D xx
			// indirect pop,
			// pops a 32 bit pointer off the stack and then pops xx bytes
			// into the location referenced by the pointer
			{
				// TODO
				uint8 count = ds.read1();
				uint16 offset = stack.pop2();
				uint16 segment = stack.pop2();
				// Pop the bytes
				stack.addSP(count);
				LOGPF(("!pop indirect\t%02Xh bytes", count));
			}
			break;

		case 0x4E:
			// 4E xx xx xx
			// push global xx xx xx
			i0 = read2(in); i1 = read1(in);
			flag = i0 + (i1<<16);
			printf("push\t\tglobal [%04X %02X] (%s)", i0, i1, FlagName[flag].c_str());
			break;
		case 0x4F:
			// 4F xx xx xx
			// pop value into global xx xx xx
			i0 = read2(in); i1 = read1(in);
			flag = i0 + (i1<<16);
			printf("pop\t\tglobal [%04X %02X] (%s)", i0, i1, FlagName[flag].c_str());
			break;

*/

/*
  0x50 - 0x76 ...
*/

		case 0x77:
			// 77
			// set info
			// assigns item number and ProcessType 
			p->item_num = p->stack.pop2();
			p->type = p->stack.pop2();
			LOGPF(("set info"));
			break;

		default:
			perr.printf("unhandled opcode %02X\n", opcode);
			cede = true;
		}
	}




	return false;
}
