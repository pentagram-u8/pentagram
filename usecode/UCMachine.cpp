/*
Copyright (C) 2002,2003 The Pentagram team

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
#include "Usecode.h"
#include "Kernel.h"
#include "DelayProcess.h"

#include "CurrentMap.h"
#include "World.h"

#include "u8intrinsics.h"

#define LOGPF(X) pout.printf X
//#define LOGPF(X)

enum UCSegments {
	SEG_STACK      = 0x0000,
	SEG_STACK_FIRST= 0x0001,
	SEG_STACK_LAST = 0x7FFE,
	SEG_STRING     = 0x8000,
	SEG_LIST       = 0x8001, // I don't think this is used
	SEG_OBJ        = 0x8002,
	SEG_GLOBAL     = 0x8003
};

UCMachine* UCMachine::ucmachine = 0;

UCMachine::UCMachine() :
	globals(0x1000)
{
	assert(ucmachine == 0);
	ucmachine = this;

	// zero globals
	globals.push0(globals.getSize());
	pout << "Flag 5A: " << (int)(globals.access1(0x5A)) << std::endl;

	loop_list = 0;
	loop_index = 0;

	intrinsics = U8Intrinsics; //!...
}


UCMachine::~UCMachine()
{
	ucmachine = 0;
}

static const char *print_bp(const sint16 offset)
{
	static char str[32];
	snprintf(str, 32, "[BP%c%02Xh]", offset<0?'-':'+', 
				  offset<0?-offset:offset);
	return str;
}

static const char *print_sp(const sint16 offset)
{
	static char str[32];
	snprintf(str, 32, "[SP%c%02Xh]", offset<0?'-':'+', 
				  offset<0?-offset:offset);
	return str;
}

bool UCMachine::execProcess(UCProcess* p)
{
	static uint8 tempbuf[256];

	assert(p);

	IBufferDataSource cs(p->usecode->get_class(p->classid),
						 p->usecode->get_class_size(p->classid));
	cs.seek(p->ip);

	//! check if process is suspended? (or do that in UCProcess::run?)

	pout << std::hex << "running process " << p->pid << ", class " << p->classid << ", offset " << p->ip << std::dec << std::endl;

	bool cede = false;
	bool error = false;

	while(!cede && !error && !p->terminated)
	{
		//! guard against reading past end of class
		//! guard against other error conditions

		uint8 opcode = cs.read1();

		LOGPF(("sp = %02X; %04X: %02X\t",
			   p->stack.stacksize(), p->ip, opcode));

		sint8 si8a, si8b;
		uint8 ui8a;
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
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.pop2();
			p->stack.assign1(p->bp+si8a, static_cast<uint8>(ui16a));
			LOGPF(("pop byte\t%s = %02Xh", print_bp(si8a), ui16a));
			break;

		case 0x01:
			// 01 xx
			// pop 16 bit int into bp+xx
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.pop2();
			p->stack.assign2(p->bp+si8a, ui16a);
			LOGPF(("pop\t\t%s = %04Xh", print_bp(si8a), ui16a));
			break;

		case 0x02:
			// 02 xx
			// pop 32 bit int into bp+xx
			si8a = static_cast<sint8>(cs.read1());
			ui32a = p->stack.pop4();
			p->stack.assign4(p->bp+si8a, ui32a);
			LOGPF(("pop dword\t%s = %08Xh", print_bp(si8a), ui32a));
			break;

		case 0x03:
			// 03 xx yy
			// pop yy bytes into bp+xx
			{
				si8a = static_cast<sint8>(cs.read1());
				uint8 size = cs.read1();
				uint8 buf[256];
				p->stack.pop(buf, size);
				p->stack.assign(p->bp+si8a, buf, size);
				LOGPF(("pop huge\t%s %i", print_bp(si8a), size));
			}
			break;

		case 0x08:
			// 08
			// pop 32bits into result register
			//! what is this result register exactly??
			//! probably a member of the Process?
			//! is the result in 0x08 and 0x6D the same var?
			LOGPF(("pop dword\tprocess result"));
			p->result = p->stack.pop4();
			break;

		case 0x09:
			// 09 xx yy zz
			// pop yy bytes into an element of list bp+xx (or slist if zz set)
			si8a = static_cast<sint8>(cs.read1());
			ui32a = cs.read1();
			si8b = static_cast<sint8>(cs.read1());
			LOGPF(("assign element\t%s (%02X) (%02X)",
				   print_bp(si8a), si8b, si8b));
			ui16a = p->stack.pop2(); // index
			ui16b = p->stack.access2(p->bp+si8a);
			if (si8b) { // slist?
				// what special behaviour do we need here?
				// probably just that the overwritten element has to be freed?
				if (ui32a != 2) error = true; // um?
				listHeap[ui16b]->assign(ui16a, p->stack.access());
				p->stack.pop2(); // advance SP
			} else {
				listHeap[ui16b]->assign(ui16a, p->stack.access());
				p->stack.moveSP(ui32a);
			}
			break;

		// PUSH opcodes

		case 0x0A:
			// 0A xx
			// push sign-extended 8 bit xx onto the stack as 16 bit
			ui16a = static_cast<sint8>(cs.read1());
			p->stack.push2(ui16a);
			LOGPF(("push byte\t%04Xh", ui16a));
			break;

		case 0x0B:
			// 0B xx xx
			// push 16 bit xxxx onto the stack
			ui16a = cs.read2();
			p->stack.push2(ui16a);
			LOGPF(("push\t\t%04Xh", ui16a));
			break;

		case 0x0C:
			// 0C xx xx xx xx
			// push 32 bit xxxxxxxx onto the stack
			ui32a = cs.read4();
			p->stack.push4(ui32a);
			LOGPF(("push dword\t%08Xh", ui32a));
			break;

		case 0x0D:
			// 0D xx xx yy ... yy 00
			// push string (yy ... yy) of length xx xx onto the stack
			{
				ui16a = cs.read2();
				char *str = new char[ui16a+1];
				cs.read(str, ui16a);
				str[ui16a] = 0;
				LOGPF(("push string\t\"%s\"", str));
				ui16b = cs.read1();
				if (ui16b != 0) error = true;
				p->stack.push2(assignString(str));
				delete[] str;
			}
			break;

		case 0x0E:
			// 0E xx yy
			// pop yy values of size xx and push the resulting list
			{
				ui16a = cs.read1();
				ui16b = cs.read1();
				UCList* l = new UCList(ui16a, ui16b);
				for (unsigned int i = 0; i < ui16b; i++) {
					l->append(p->stack.access());
					p->stack.addSP(ui16a);
				}
				p->stack.push2(assignList(l));
				LOGPF(("create list\t%02X (%02X)", ui16b, ui16a));
			}
			break;

		// Usecode function and intrinsic calls


		case 0x0F:
			// 0F xx yyyy
			// intrinsic call. xx is number of argument bytes
			// (includes this pointer, if present)
			// NB: do not actually pop these argument bytes
			{
				//! TODO
				uint16 arg_bytes = cs.read1();
				uint16 func = cs.read2();
				LOGPF(("!calli\t\t%04Xh (%02Xh arg bytes)", func, arg_bytes));

				// !constants
				if (func >= 0x100 || intrinsics[func] == 0) {
//					temp32 = addProcess(new DelayProcess(4));
					temp32 = 0;
					perr << "Unhandled intrinsic called" << std::endl;
				} else {
					uint8 *argbuf = new uint8[arg_bytes];
					p->stack.pop(argbuf, arg_bytes);
					p->stack.addSP(-arg_bytes); // don't really pop the args

					temp32 = intrinsics[func](argbuf, arg_bytes);

					delete[] argbuf;
				}
			}
			break;


		case 0x11:
			// 11 xx xx yy yy
			// call the function at offset yy yy of class xx xx
			{
				uint16 new_classid = cs.read2();
				uint16 new_offset = cs.read2();
				LOGPF(("call\t\t%04X:%04X", new_classid, new_offset));

				p->ip = static_cast<uint16>(cs.getPos());	// Truncates!!
				p->call(new_classid, new_offset);

				// Update the code segment
				cs.load(p->usecode->get_class(p->classid),
						p->usecode->get_class_size(p->ip));
				cs.seek(p->ip);

				// Resume execution
			}
			break;

		case 0x12:
			// 12
			// pop 16bits into temp register
			temp32 = p->stack.pop2();
			LOGPF(("pop\t\ttemp = %04X", (temp32 & 0xFFFF)));
			break;

		case 0x13:
			// 13
			// pop 32bits into temp register
			// NB: 0x13 isn't used AFAIK, but this is a 'logical' guess
			temp32 = p->stack.pop4();
			LOGPF(("pop long\t\ttemp = %08X", temp32));
			break;

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


		case 0x16:
			// 16
			// pop two strings from the stack and push the concatenation
			// (free the originals? order?)
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			stringHeap[ui16b] += stringHeap[ui16a];
			freeString(ui16a);
			p->stack.push2(ui16b);
			LOGPF(("concat\t\t= %s", stringHeap[ui16b].c_str()));
			break;

		case 0x17:
			// 17
			// pop two lists from the stack and push the 'sum' of the lists
			// (free the originals? order?)
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			if (getList(ui16b) && getList(ui16a)) {
				getList(ui16b)->appendList(*getList(ui16a));
				freeList(ui16a);
				p->stack.push2(ui16b);
			} else {
				// at least one of the lists didn't exist. Error or not?
				// for now: if one exists, push that one.
				// if neither exists, push 0.

				if (getList(ui16a)) {
					p->stack.push2(ui16a);
				} else if (getList(ui16b)) {
					p->stack.push2(ui16b);
				} else {
					p->stack.push2(0);
				}
			}
			LOGPF(("append"));
			break;

		case 0x19:
			// 19 02
			// add two stringlists, removing duplicates
			ui32a = cs.read1();
			if (ui32a != 2) error = true;
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			listHeap[ui16b]->unionStringList(*listHeap[ui16a]);
			freeStringList(ui16a); // contents are actually freed in unionSL
			p->stack.push2(ui16b);
			LOGPF(("union slist\t(%02X)", ui32a));
			break;

		case 0x1A:
			// 1A
			// substract string list
			// NB: this one takes a length parameter in crusader. (not in U8)!!
			// (or rather, it seems it takes one after all? -wjp,20030511)
			ui32a = cs.read1(); // elementsize
			ui32a = 2;
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			listHeap[ui16b]->substractStringList(*listHeap[ui16a]);
			freeStringList(ui16a); // contents are actually freed in subSL
			p->stack.push2(ui16b);
			LOGPF(("remove slist\t(%02X)", ui32a));
			break;			

		case 0x1B:
			// 1B xx
			// pop two lists from the stack and remove the 2nd from the 1st
			// (free the originals? order?)
			// only occurs in crusader.
			ui32a = cs.read1(); // elementsize
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			listHeap[ui16b]->substractList(*listHeap[ui16a]);
			freeList(ui16a);
			p->stack.push2(ui16b);
			LOGPF(("remove list\t(%02X)", ui32a));
			break;

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


		case 0x26:
			// 26
			// compare two strings
			//! delete strings?
			ui16a = p->stack.pop2();
			ui16b = p->stack.pop2();
			if (stringHeap[ui16b] == stringHeap[ui16a])
				p->stack.push2(1);
			else
				p->stack.push2(0);
			freeString(ui16a);
			freeString(ui16b);
			LOGPF(("strcmp"));
			break;


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


		case 0x31:
			// 31
			// 32 bit boolean not (both input and output 32 bit?)
			ui32a = p->stack.pop4();
			if (!ui32a) {
				p->stack.push4(1);
			} else {
				p->stack.push4(0);
			}
			LOGPF(("not long"));
			break;

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

		case 0x33:
			// 33
			// 32 bit boolean and
			ui32a = p->stack.pop4();
			ui32b = p->stack.pop4();
			if (ui32a && ui32b) {
				p->stack.push4(1);
			} else {
				p->stack.push4(0);
			}
			LOGPF(("and long"));
			break;

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
			LOGPF(("or"));
			break;

		case 0x35:
			// 35
			// 32 bit boolean or
			ui32a = p->stack.pop4();
			ui32b = p->stack.pop4();
			if (ui32a || ui32b) {
				p->stack.push4(1);
			} else {
				p->stack.push4(0);
			}
			LOGPF(("or long"));
			break;

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


		case 0x38:
			// 38 xx yy
			// is element (size xx) in list? (or slist if yy is true)
			// free list/slist afterwards

			ui16a = cs.read1();
			ui32a = cs.read1();
			ui16b = p->stack.pop2();
			if (ui32a) { // stringlist
				if (ui16a != 2) error = true;
				if (listHeap[ui16b]->stringInList(p->stack.pop2()))
					p->stack.push2(1);
				else
					p->stack.push2(0);
				freeStringList(ui16b);
			} else {
				bool found = listHeap[ui16b]->inList(p->stack.access());
				p->stack.addSP(ui16a);
				if (found)
					p->stack.push2(1);
				else
					p->stack.push2(0);

				freeList(ui16b);
			}
			LOGPF(("in list\t\t%s slist==%02X", print_bp(ui16a), ui32a));
			break;

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
			si8a = static_cast<sint8>(cs.read1());
			ui16a = static_cast<sint8>(p->stack.access1(p->bp+si8a));
			p->stack.push2(ui16a);
			LOGPF(("push byte\t%s = %02Xh", print_bp(si8a), ui16a));
			break;

		case 0x3F:
			// 3F xx
			// push the value of the 16 bit local var xx
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->bp+si8a);
			p->stack.push2(ui16a);
			LOGPF(("push\t\t%s = %04Xh", print_bp(si8a), ui16a));
			break;

		case 0x40:
			// 40 xx
			// push the value of the 32 bit local var xx
			si8a = static_cast<sint8>(cs.read1());
			ui32a = p->stack.access4(p->bp+si8a);
			p->stack.push4(ui32a);
			LOGPF(("push dword\t%s = %08Xh", print_bp(si8a), ui32a));
			break;

		case 0x41:
			// 41 xx
			// push the string local var xx
			// duplicating the string?
			{
				si8a = static_cast<sint8>(cs.read1());
				ui16a = p->stack.access2(p->bp+si8a);
				p->stack.push2(duplicateString(ui16a));
				LOGPF(("push string\t%s", print_bp(si8a)));
			}
			break;

		case 0x42:
			// 42 xx yy
			// push the list (with yy size elements) at BP+xx
			// duplicating the list?
			{
				si8a = static_cast<sint8>(cs.read1());
				ui16a = cs.read1();
				ui16b = p->stack.access2(p->bp+si8a);
				UCList* l = new UCList(ui16a);
				if (getList(ui16b)) {
					l->copyList(*getList(ui16b));
				} else {
					// trying to push non-existant list. Error or not?
					perr << "Pushing non-existant slist" << std::endl;
					// error = true;
				}
				p->stack.push2(assignList(l));
				LOGPF(("push list\t%s", print_bp(si8a)));
			}
			break;

		case 0x43:
			// 43 xx
			// push the stringlist local var xx
			// duplicating the list, duplicating the strings in the list
			{
				si8a = static_cast<sint8>(cs.read1());
//!U8				ui16a = cs.read1();
				ui16a = 2;
				ui16b = p->stack.access2(p->bp+si8a);
				UCList* l = new UCList(ui16a);
				if (getList(ui16b)) {
					l->copyStringList(*getList(ui16b));
				} else {
					// trying to push non-existant list. Error or not?
					// (Devon's talk code seems to use it; so no error for now)
					perr << "Pushing non-existant slist" << std::endl;
					// error = true;
				}
				p->stack.push2(assignList(l));
				LOGPF(("push slist\t%s", print_bp(si8a)));
			}
			break;

		case 0x44:
			// 44 xx yy
			// push element from the second last var pushed onto the stack
			// (a list/slist), indexed by the last element pushed onto the list
			// (a byte/word). XX is the size of the types contained in the list
			// YY is true if it's a slist (for garbage collection)

			// duplicate string if YY? yy = 1 only occurs
			// in two places in U8: once it pops into temp afterwards,
			// once it is indeed freed. So, guessing we should duplicate.

			ui32a = cs.read1();
			ui32b = cs.read1();
			ui16a = p->stack.pop2(); // index
			ui16b = p->stack.pop2(); // list
			if (ui32b) {
				uint16 s = listHeap[ui16b]->getStringIndex(ui16a);
				p->stack.push2(duplicateString(s));
			} else {
				p->stack.push((*listHeap[ui16b])[ui16a], ui32a);
			}
			LOGPF(("push element\t%02X slist==%02X", ui32a, ui32b));
			break;

		case 0x45:
			// 45 xx yy
			// push huge of size yy from BP+xx
			si8a = static_cast<sint8>(cs.read1());
			ui16b = cs.read2();
			p->stack.push(p->stack.access(p->bp+si8a), ui16b);
			LOGPF(("push huge\t%s %02X", print_bp(si8a), ui16b));
			break;

		case 0x4B:
			// 4B xx
			// push 32 bit pointer address of BP+XX
			si8a = static_cast<sint8>(cs.read1());
			p->stack.push4(stackToPtr(p->pid, p->bp+si8a));
			LOGPF(("push addr\t%s", print_bp(si8a)));
			break;

		case 0x4C:
			// 4C xx
			// indirect push,
			// pops a 32 bit pointer off the stack and pushes xx bytes
			// from the location referenced by the pointer
			{
				ui16a = cs.read1();
				ui32a = p->stack.pop4();

				p->stack.addSP(-ui16a);
				if (!dereferencePointer(ui32a,
										const_cast<uint8*>(p->stack.access()),
										ui16a))
					error = true;
			}
			break;

		case 0x4D:
			// 4D xx
			// indirect pop
			// pops a 32 bit pointer off the stack and pushes xx bytes
			// from the location referenced by the pointer
			{
				ui16a = cs.read1();
				ui32a = p->stack.pop4();

				if (assignPointer(ui32a, p->stack.access(), ui16a)) {
					p->stack.addSP(ui16a);
				} else {
					error = true;
				}
			}
			break;

		case 0x4E:
			// 4E xx xx yy
			// push global xxxx size yy
			ui16a = cs.read2();
			ui16b = cs.read1();
			// get flagname for output?
			switch (ui16b) {
			case 1: // push a 1 byte flag as 2 bytes
				p->stack.push2(globals.access1(ui16a));
				break;
			default:
				p->stack.push(globals.access(ui16a), ui16b);
			}
			LOGPF(("push\t\tglobal [%04X %02X]", ui16a, ui16b));
			break;

		case 0x4F:
			// 4F xx xx yy
			// pop value into global xxxx size yy
			ui16a = cs.read2();
			ui16b = cs.read1();
			// get flagname for output?
			p->stack.pop(tempbuf, ui16b);
			globals.assign(ui16a, tempbuf, ui16b);
			LOGPF(("pop\t\tglobal [%04X %02X]", ui16a, ui16b));
			break;

		case 0x50:
			// 50
			// return from function

			if (p->ret()) { // returning from process
				// TODO
				LOGPF(("ret\t\tfrom process"));
				killProcess(p);

				// return value is going to be stored somewhere,
				// and some other process is probably waiting for it.
				// So, we can't delete ourselves just yet.
				// Question is of course when we can...
			} else {
				LOGPF(("ret\t\tto %04X:%04X", p->classid, p->ip));

				// return value is stored in temp32 register

				// Update the code segment
				cs.load(p->usecode->get_class(p->classid),
						p->usecode->get_class_size(p->ip));
				cs.seek(p->ip);
			}

			// Resume execution
			break;

		case 0x51:
			// 51 xx xx
			// relative jump to xxxx if false
			si16a = static_cast<sint16>(cs.read2());
			ui16b = p->stack.pop2();
			if (!ui16b) {
				cs.skip(si16a);
				LOGPF(("jne\t\t%04hXh\t(to %04X) (taken)", si16a,
					   cs.getPos()));
			} else {
				LOGPF(("jne\t\t%04hXh\t(to %04X) (not taken)", si16a,
					   cs.getPos()));
			}
			break;

		case 0x52:
			// 52 xx xx
			// relative jump to xxxx
			si16a = static_cast<sint16>(cs.read2());
			cs.skip(si16a);
			LOGPF(("jmp\t\t%04hXh\t(to %04X)", si16a, cs.getPos()));
			break;

		case 0x53:
			// 53
			// suspend
			LOGPF(("suspend"));
			cede=true; 
			break;

		case 0x54:
			// 54 01 01
			// implies
			// this seems to link two processes (two pids are popped)
			// the '01 01' variety most likely causes one process
			// to wait for the other to finish.
			// the first pid pushed is often the current pid in U8

			// question: can multiple processes be waiting for a single proc?
			// can a process be waiting for multiple processes?

			// 'implies' seems to push a value too, although it is very
			// often ignored. It looks like it's a pid, but which one?

			// additionally, it is possible that 'implies' puts the result
			// of a process in the 'process result' variable,
			// or more likely, when a process finishes, it sets the result
			// value of the processes that were waiting for it.
			// 0x6D (push process result) only seems to occur soon after
			// an 'implies'

			{
				cs.read2(); // skip the 01 01
				ui16a = p->stack.pop2();
				ui16b = p->stack.pop2();
				p->stack.push2(ui16a); //!! which pid do we need to push!?
				LOGPF(("implies"));

				Process *proc = Kernel::get_instance()->getProcess(ui16b);
				Process *proc2 = Kernel::get_instance()->getProcess(ui16a);
				if (proc && proc2) {
					proc->waitFor(ui16a);
				} else {

					// There seem to be cases where a process has terminated
					// before the caller has had the time to wait for it.
					// Does that mean we had to wait longer with terminating
					// the process? For now, just ignore the error.

					// (Example: AVATAR::Look(), the spawn FREE:29D8)

					perr << "Non-existant process PID in implies" << std::endl;
//					error = true;
				}
			}
			break;

		case 0x57:
			// 57 aa tt xx xx yy yy
			// spawn process function yyyy in class xxxx
			// aa = number of arg bytes pushed (not including this pointer which is 4 bytes)
			// tt = sizeof this pointer object
			// only remove the this pointer from stack (4 bytes)
			// put PID of spawned process in temp			
			{
				uint32 arg_bytes = cs.read1();
				uint32 this_size = cs.read1(); // Relevance?
				uint16 classid = cs.read2();
				uint16 offset = cs.read2();
				
				LOGPF(("spawn\t\t%02X %02X %04X:%04X",
					   arg_bytes, this_size, classid, offset));

				UCProcess* newproc = new UCProcess(p->usecode, classid,
												   offset, p->stack.access(),
												   arg_bytes + 4);
				// arg_bytes + 4 = arguments + this pointer
				//!! CHECKME
				//!! it looks like these arguments may have to be put
				//!! earlier on the stack than this
				//!! (i.e., even before the this pointer)
				//!! (check for instance METHOD::091C)

				temp32 = addProcess(newproc);

				//!! CHECKME
				//!! is order of execution of this process and the new one
				//!! relevant? It might be, since 0x6C and freeing opcodes
				//!! might be executed in the wrong order otherwise.
				//!! Currently, the spawned processes is added to the front
				//!! of the execution list, so it's guaranteed to be run
				//!! before the current process
			}
			cede = true; // give new process a chance to run
			break;


		case 0x58:
			// 58 xx xx yy yy zz zz tt uu
			// spawn inline process function yyyy in class xxxx at offset zzzz
			// tt = size of this pointer
			// uu = unknown (occuring values: 00, 02, 05)

			//! What to do with this new process?
			//! The inlined code does use 'var06' (this) sometimes, so
			//! maybe it should copy the this pointer from the current proc?
			{
				uint16 classid = cs.read2();
				uint16 offset = cs.read2();
				uint16 delta = cs.read2();
				uint32 this_size = cs.read1(); // relevance?
				uint32 unknown = cs.read1(); // ??
				
				LOGPF(("spawn inline\t%04X:%04X+%04X=%04X %02X %02X",
					   classid,offset,delta,offset+delta,this_size, unknown));

				UCProcess* newproc = new UCProcess(p->usecode, classid,
												   offset + delta,
												   p->stack.access4(p->bp+6));

				p->stack.push2(addProcess(newproc)); //! push pid of newproc?
			}
			cede = true;
			break;
	
		case 0x59:
			// 59
			// push process id
			p->stack.push2(p->pid);
			LOGPF(("push\t\tpid = %04Xh", p->pid));
			break;

		case 0x5A:
			// 5A xx
			// init function. xx = local var size
			// sets xx bytes on stack to 0, moving sp
			ui16a = cs.read1();
			LOGPF(("init\t\t%02X", ui16a));
			
			if (ui16a & 1) ui16a++; // 16-bit align
			if (ui16a > 0) {
				p->stack.push0(ui16a);
			}
			break;

		case 0x5D:
			// 5D
			// push 8 bit value returned from function call
			// (push temp8 as 16 bit value)
			p->stack.push2(static_cast<uint8>(temp32 & 0xFF));
			LOGPF(("push byte\tretval = %02X", (temp32 & 0xFF)));
			break;

		case 0x5E:
			// 5E
			// push 16 bit value returned from function call
			// (push temp16)
			p->stack.push2(static_cast<uint16>(temp32 & 0xFFFF));
			LOGPF(("push\t\tretval = %04X", (temp32 & 0xFFFF)));
			break;

		case 0x5F:
			// 5F
			// push 32 bit value returned from function call
			// (push temp32)
			p->stack.push4(temp32);
			LOGPF(("push long\t\tretval = %08X", temp32));
			break;

		case 0x60:
			// 60
			// convert 16-bit to 32-bit int (sign extend)
			si32a = static_cast<sint16>(p->stack.pop2());
			p->stack.push4(si32a);
			LOGPF(("int to long"));
			break;

		case 0x61:
			// 61
			// convert 32-bit to 16-bit int
			si16a = static_cast<sint16>(p->stack.pop4());
			p->stack.push2(si16a);
			LOGPF(("long to int"));
			break;

		case 0x62:
			// 62 xx
			// free the string in var BP+xx
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->bp+si8a);
			freeString(ui16a);
			LOGPF(("free string\t%s", print_bp(si8a)));
			break;

		case 0x63:
			// 63 xx
			// free the stringlist in var BP+xx
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->bp+si8a);
			freeStringList(ui16a);
			LOGPF(("free slist\t%s", print_bp(si8a)));
			break;

		case 0x64:
			// 64 xx
			// free the list in var BP+xx
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->bp+si8a);
			freeList(ui16a);
			LOGPF(("free list\t%s", print_bp(si8a)));
			break;
	
		case 0x65:
			// 65 xx
			// free the string at SP+xx
			// NB: sometimes there's a 32-bit string pointer at SP+xx
			//     However, the low word of this is exactly the 16bit ref
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->stack.getSP()+si8a);
			freeString(ui16a);
			LOGPF(("free string\t%s", print_sp(si8a)));
			break;

		case 0x66:
			// 66 xx
			// free the list at SP+xx
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->stack.getSP()+si8a);
			freeList(ui16a);
			LOGPF(("free list\t%s", print_sp(si8a)));
			break;

		case 0x67:
			// 67 xx
			// free the string list at SP+xx
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->stack.getSP()+si8a);
			freeStringList(ui16a);
			LOGPF(("free slist\t%s", print_sp(si8a)));
			break;

		case 0x69:
			// 69 xx
			// push the string in var BP+xx as 32 bit pointer			
			si8a = static_cast<sint8>(cs.read1());
			ui16a = p->stack.access2(p->bp+si8a);
			p->stack.push4(stringToPtr(ui16a));
			LOGPF(("str to ptr\t%s", print_bp(si8a)));
			break;

		case 0x6B:
			// 6B
			// pop a string and push 32 bit pointer to string
			ui16a = p->stack.pop2();
			p->stack.push4(stringToPtr(ui16a));
			LOGPF(("str to ptr"));
			break;

/*
		case 0x6C:
		

*/

		case 0x6D:
			// 6D
			// push 32bit result of process
			// (of which process? pop anything?)
			// (also see comment for 0x54 'implies')
			LOGPF(("push dword\tprocess result"));
			p->stack.push4(p->result);
			break;

		case 0x6E:
			// 6E xx
			// substract xx from stack pointer
			// (effect on SP is the same as popping xx bytes)
			si8a = static_cast<sint8>(cs.read1());
			p->stack.addSP(-si8a);
			LOGPF(("move sp\t\t%s%02Xh", si8a<0?"-":"", si8a<0?-si8a:si8a));
			break;


		case 0x6F:
			// 6F xx
			// push 32 pointer address of SP-xx
			si8a = static_cast<sint8>(cs.read1());
			p->stack.push4(stackToPtr(p->pid, static_cast<uint16>(p->stack.getSP() - si8a)));
			LOGPF(("push addr\t%s", print_sp(-si8a)));
			break;

		// loop-related opcodes
		// 0x70 has a different types:
		//    02: search the area around an object
		//    03: search the area around an object, recursing into containers
		//    04: search a container
		//    05: search a container, recursing into containers
		//    06: something about looking for items on top of another (??)
		// each of these types allocate a rather large area on the stack
		// we expect SP to be at the end of that area when 0x73 is executed
		// a 'loop script' (created by 0x74) is used to select items

		case 0x70:
			// 70 xx yy zz
			// loop something. Stores 'current object' in var xx
			// yy == num bytes in string
			// zz == type
			{
				si16a = cs.readXS(1);
				uint32 scriptsize = cs.read1();
				uint32 searchtype = cs.read1();

				ui16a = p->stack.pop2();
				ui16b = p->stack.pop2();

				//!! This may not be the way the original did things...

				// We'll first create a list of all matching items.
				// Store the id of this list in the last two bytes
				// of our stack area.
				// Behind that we'll store an index into this list.
				// This is followed by the variable in which to store the item
				// After that we store the loopscript length followed by
				// the loopscript itself.
				//   (Note that this puts a limit on the max. size of the
				//    loopscript of 0x20 bytes)

				if (scriptsize > 0x20) {
					perr << "Loopscript too long" << std::endl;
					error = true;
					break;
				}

				uint8* script = new uint8[scriptsize];
				p->stack.pop(script, scriptsize);

				uint32 stacksize = 0;
				bool recurse = false;
				// we'll put everything on the stack after stacksize is set

				UCList *itemlist = new UCList(2);

				World* world = World::get_instance();

				switch (searchtype) {
				case 2: case 3:
				{
					// area search (3 = recursive)
					stacksize = 0x34;
					if (searchtype == 3) recurse = true;

					// ui16a = item, ui16b = range
					Item* item= p_dynamic_cast<Item*>(world->getObject(ui16a));

					if (item) {
					    world->getCurrentMap()->areaSearch(itemlist, script,
														   scriptsize, item,
														   ui16b, recurse);
					} else {
						// return error or return empty list?
						perr << "Warning: invalid item passed to area search"
							 << std::endl;
					}
					break;
				}
				case 4: case 5:
				{
					// container search (4 = recursive)
					stacksize = 0x281;
					if (searchtype == 5) { stacksize += 2; recurse = true; }

					// ui16a = 0xFFFF (?), ui16b = container
					Container* container = p_dynamic_cast<Container*>
						(world->getObject(ui16b));

					if (ui16b != 0xFFFF) {
						perr << "Warning: non-FFFF value passed to "
							 << "container search" << std::endl;
					}

					if (container) {
						container->containerSearch(itemlist, script,
												   scriptsize, recurse);
					} else {
						// return error or return empty list?
						perr << "Warning: invalid container passed to "
							 << "container search" << std::endl;
					}
					break;
				}
				case 6:
				{
					stacksize = 0x3D;
					//!!!!!! fall-through for now
				}
				default:
					perr << "Unhandled search type " << searchtype <<std::endl;
					error = true;
					delete[] script;
					break;
				}

				p->stack.push0(stacksize - scriptsize - 8); // filler
				p->stack.push(script, scriptsize);
				p->stack.push2(scriptsize);
				p->stack.push2(static_cast<uint16>(si16a));
				p->stack.push2(0);
				uint16 itemlistID = assignList(itemlist);
				p->stack.push2(itemlistID);

				delete[] script;

				LOGPF(("!loop\t\t%s %02X %02X", print_bp(si16a),
					   scriptsize, searchtype));
			}
			// FALL-THROUGH to handle first item
		case 0x73:
			// 73
			// next loop object. pushes false if end reached
			{
				unsigned int sp = p->stack.getSP();
				uint16 itemlistID = p->stack.access2(sp);
				UCList* itemlist = getList(itemlistID);
				uint16 index = p->stack.access2(sp+2);
				si16a = static_cast<sint16>(p->stack.access2(sp+4));
//				uint16 scriptsize = p->stack.access2(sp+6);
//				const uint8* loopscript = p->stack.access(sp+8);

				perr << "si16a = " << si16a << std::endl;

				if (!itemlist) {
					perr << "Invalid item list in loopnext!" << std::endl;
					error = true;
					break;
				}

				// see if there are still valid items left
				bool valid;
				do {
					if (index >= itemlist->getSize()) {
						valid = false;
						break;
					}

					p->stack.assign(p->bp+si16a, (*itemlist)[index], 2);
					uint16 objid = p->stack.access2(p->bp+si16a);
					if (p_dynamic_cast<Item*>(World::get_instance()->
											  getObject(objid))) {
						valid = true;
					} else {
						valid = false;
						index++;
					}
					
				} while (!valid);

				if (!valid) {
					p->stack.push2(0); // end of loop
					freeList(itemlistID);
				} else {
					p->stack.push2(1);
					// increment index
					p->stack.assign2(sp+2, index+1);
				}

				if (opcode == 0x73) { // because of the fall-through
					LOGPF(("!loopnext"));
				}
			}
			break;

		case 0x74:
			// 74 xx
			// add xx to the current 'loopscript'
			ui8a = cs.read1();
			p->stack.push1(ui8a);
			LOGPF(("loopscr\t\t%02X \"%c\"", ui8a, static_cast<char>(ui8a)));
			break;

		case 0x75: case 0x76:
			// 75 xx yy zz zz
			// 76 xx yy zz zz
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

			// it seems as if there's no way provided to store index
			// and list. Assuming there are no nested loops, this isn't
			// a problem. If there -are- nested loops, we could use a stack
			// for these.
			// There may be problems with jumps from inside the loop to outside
			// Either these are forbidden, or we have to detect when jumping
			// to outside a loop? (yuck)
			// (this will be _very_ messy when combined with nested loops,
			//  let's hope it won't be necessary)

			// random idea: maybe the 0xFFFF on the stack is used to
			// indicate the start of a loop? Would be mildly ugly, but could
			// be useful for nested loops or loop-escaping jumps

			// other random idea: 0xFFFF could also be the loop index
			// to start with minus 1. (This would clean up the 'loop_index=0'
			// or 'loop_index++' distinction a bit)

			// 75 is for lists, 76 for slists
			// Only difference should be in the freeing afterwards.
			// Strings are _not_ duplicated when putting them in the loopvar
			// Lists _are_ freed afterwards


			si8a = cs.read1(); // loop variable
			ui32a = cs.read1(); // list size
			si16a = cs.read2(); // jump offset

			if (opcode == 0x76 && ui32a != 2) {
				error = true;
			}
			
			if (opcode == 0x75) {
				LOGPF(("for each\t%s (%02X) %04hX",
					   print_bp(si8a), ui32a, si16a));
			} else {
				LOGPF(("for each str\t%s (%02X) %04hX",
					   print_bp(si8a), ui32a, si16a));
			}
			
			if (loop_list == 0) { // not in loop yet
				ui16a = p->stack.pop2(); // 0xFFFF !?
				loop_list = p->stack.pop2(); // list to loop over
				loop_index = 0;
			} else {
				loop_index++;
			}
			
			if (loop_index >= listHeap[loop_list]->getSize()) {
				// loop done
				
				// free loop list
				if (opcode == 0x75) {
					freeList(loop_list);
				} else {
					freeStringList(loop_list);
				}
				
				// reset 'global' loop vars
				loop_list = 0;
				loop_index = 0;
				
				// jump out
				cs.skip(si16a);					
			}
			else
			{
				// loop iteration
				// place next element from list in [bp+si8a]
				// (not duplicating any strings)
				
				p->stack.push((*listHeap[loop_list])[loop_index], ui32a);
			}
			break;

		case 0x77:
			// 77
			// set info
			// assigns item number and ProcessType 
			p->item_num = p->stack.pop2();
			p->type = p->stack.pop2();
			LOGPF(("set info"));
			break;

/*
		case 0x78:
			// 78
			// process exclude
			// process gets 'exclusive access' to this object

			// This probably means that an object has to have an
			// 'exclusive_pid' variable. When starting to execute
			// a UCProcess, we have to check if the current item
			// is locked by another process?

			LOGPF(("process exclude"));
			break;

*/

		case 0x79: case 0x7A:
			// 7A
			// end of function
			// shouldn't happen
			//! 0x79 is U8 only. Should be removed
			LOGPF(("end"));
			perr.printf("end of function opcode reached!\n");
			error = true;
			break;

		case 0x5B: case 0x5C: // debugging
			perr.printf("unhandled opcode %02X\n", opcode);
			break;
		case 0x6C: // parameter passing?
			error = true;
			perr.printf("unhandled opcode %02X\n", opcode);
			break;
		case 0x78: // process exclude
			perr.printf("unhandled opcode %02X\n", opcode);
			break;
				
		default:
			perr.printf("unhandled opcode %02X\n", opcode);

		} // switch(opcode)

		LOGPF(("\n"));

		// write back IP
		p->ip = static_cast<uint16>(cs.getPos());	// TRUNCATES!

	} // while(!cede && !error)

	if (error) {
		perr << "Process " << p->pid << " caused an error. Killing process."
			 << std::endl;

		killProcess(p);
	}


	return false;
}


uint16 UCMachine::assignString(const char* str)
{
	static uint16 count = 0; // 0 is reserved

	// I'm not exactly sure if this is the most efficient way to do this

	//! infinite loop when too many strings

	// find unassigned element
	do {
		count++;
		if (count > 0xFFFE) count = 1;
	} while (stringHeap.find(count) != stringHeap.end());

	stringHeap[count] = str;

	return count;
}

uint16 UCMachine::duplicateString(uint16 str)
{
	return assignString(stringHeap[str].c_str());
}


uint16 UCMachine::assignList(UCList* l)
{
	static uint16 count = 0; // 0 is reserved

	// I'm not exactly sure if this is the most efficient way to do this

	//! infinite loop when too many lists

	// find unassigned element
	do {
		count++;
		if (count > 0xFFFE) count = 1;
	} while (listHeap.find(count) != listHeap.end());

	listHeap[count] = l;

	return count;
}

void UCMachine::freeString(uint16 s)
{
	//! There's still a semi-bug in some places that string 0 can be assigned
	//! (when something accesses stringHeap[0])
	//! This may not be desirable, but OTOH the created string will be
	//! empty, so not too much of a problem.
	std::map<uint16, std::string>::iterator iter = stringHeap.find(s);
	if (iter != stringHeap.end()) {
		stringHeap.erase(iter);
	}
}

void UCMachine::freeList(uint16 l)
{
	std::map<uint16, UCList*>::iterator iter = listHeap.find(l);
	if (iter != listHeap.end() && iter->second) {
		iter->second->free();
		delete iter->second;
		listHeap.erase(iter);
	}
}

void UCMachine::freeStringList(uint16 l)
{
	std::map<uint16, UCList*>::iterator iter = listHeap.find(l);
	if (iter != listHeap.end() && iter->second) {
		iter->second->freeStrings();
		delete iter->second;
		listHeap.erase(iter);
	}	
}

//static
uint32 UCMachine::listToPtr(uint16 l)
{
	uint32 ptr = SEG_LIST;
	ptr <<= 16;
	ptr += l;
	return ptr;
}

//static
uint32 UCMachine::stringToPtr(uint16 s)
{
	uint32 ptr = SEG_STRING;
	ptr <<= 16;
	ptr += s;
	return ptr;
}

//static
uint32 UCMachine::stackToPtr(uint16 pid, uint16 offset)
{
	uint32 ptr = SEG_STACK + pid;
	ptr <<= 16;
	ptr += offset;
	return ptr;
}

//static
uint32 UCMachine::globalToPtr(uint16 offset)
{
	uint32 ptr = SEG_GLOBAL;
	ptr <<= 16;
	ptr += offset;
	return ptr;
}

//static
uint32 UCMachine::objectToPtr(uint16 objID)
{
	uint32 ptr = SEG_OBJ;
	ptr <<= 16;
	ptr += objID;
	return ptr;
}

bool UCMachine::assignPointer(uint32 ptr, const uint8* data, uint32 size)
{
	// Only implemented the following:
	// * stack pointers
	// * global pointers


	//! range checking...

	uint16 segment =static_cast<uint16>(ptr >> 16);
	uint16 offset = static_cast<uint16>(ptr & 0xFFFF);

	if (segment >= SEG_STACK_FIRST && segment <= SEG_STACK_LAST)
	{
		UCProcess *proc = p_dynamic_cast<UCProcess*>
			(Kernel::get_instance()->getProcess(segment));
		
		// reference to the stack of pid 'segment'
		if (!proc) {
			// segfault :-)
			perr << "Trying to access stack of non-existent "
				 << "process (pid: " << std::hex << segment
				 << std::dec << ")" << std::endl;
			return false;
		} else {
			proc->stack.assign(offset, data, size);
		}
	}
	else if (segment == SEG_GLOBAL)
	{
		globals.assign(offset, data, size);
	}
	else
	{
		perr << "Trying to access segment " << std::hex
			 << segment << std::dec << std::endl;
		return false;
	}

	return true;
}

bool UCMachine::dereferencePointer(uint32 ptr, uint8* data, uint32 size)
{
	// this one is a bit tricky. There's no way we can support
	// all possible pointers, so we're just going to do a few:
	// * stack pointers
	// * object pointers, as long as xx == 02. (i.e., get objref)
	// * global pointers


	//! range checking...

	uint16 segment = static_cast<uint16>(ptr >> 16);
	uint16 offset = static_cast<uint16>(ptr & 0xFFFF);
	
	if (segment >= SEG_STACK_FIRST && segment <= SEG_STACK_LAST)
	{
		UCProcess *proc = p_dynamic_cast<UCProcess*>
			(Kernel::get_instance()->getProcess(segment));
		
		// reference to the stack of pid 'segment'
		if (!proc) {
			// segfault :-)
			perr << "Trying to access stack of non-existent "
				 << "process (pid: " << std::hex << segment
				 << std::dec << ")" << std::endl;
			return false;
		} else {
			std::memcpy(data, proc->stack.access(offset), size);
		}
	}
	else if (segment == SEG_OBJ)
	{
		if (size != 2) {
			perr << "Trying to read other than 2 bytes from objptr"
				 << std::endl;
			return false;
		} else {
			// push objref
			data[0] = static_cast<uint8>(offset);
			data[1] = static_cast<uint8>(offset>>8);
		}
	}
	else if (segment == SEG_GLOBAL)
	{
		std::memcpy(data, globals.access(offset), size);
	}
	else
	{
		perr << "Trying to access segment " << std::hex
			 << segment << std::dec << std::endl;
		return false;
	}
	return true;
}

//static
uint16 UCMachine::ptrToObject(uint32 ptr)
{
	//! This function is a bit of a misnomer, since it's more general than this

	uint16 segment = static_cast<uint16>(ptr >> 16);
	uint16 offset = static_cast<uint16>(ptr);
	if (segment >= SEG_STACK_FIRST && segment <= SEG_STACK_LAST)
	{
		UCProcess *proc = p_dynamic_cast<UCProcess*>
			(Kernel::get_instance()->getProcess(segment));

		// reference to the stack of pid 'segment'
		if (!proc) {
			// segfault :-)
			perr << "Trying to access stack of non-existent "
				 << "process (pid: " << std::hex << segment
				 << std::dec << ")" << std::endl;
			return 0;
		} else {
			return proc->stack.access2(offset);
		}
	}
	else if (segment == SEG_OBJ || segment == SEG_STRING)
	{
		return offset;
	}
	else if (segment == SEG_GLOBAL)
	{
		return ucmachine->globals.access2(offset);
	}
	else
	{
		perr << "Trying to access segment " << std::hex
			 << segment << std::dec << std::endl;
		return 0;
	}
}


uint16 UCMachine::addProcess(Process* p)
{
	return Kernel::get_instance()->addProcess(p);
}

void UCMachine::killProcess(Process* p)
{
	p->terminate();
}

void UCMachine::killProcess(uint16 pid)
{
	Kernel* k = Kernel::get_instance();

	Process* p = k->getProcess(pid);
	if (p)
		p->terminate();
}

void UCMachine::usecodeStats()
{
	pout << "Usecode Machine memory stats:" << std::endl;
//	pout << "Processes: " << processes.size() << "/32766" << std::endl;
	pout << "Strings  : " << stringHeap.size() << "/65534" << std::endl;

	std::map<uint16, std::string>::iterator iter;
	for (iter = stringHeap.begin(); iter != stringHeap.end(); ++iter)
		pout << iter->first << ":" << iter->second << std::endl;
	pout << "Lists    : " << listHeap.size() << "/65534" << std::endl;


}


uint32 UCMachine::I_AvatarCanCheat(const uint8* /*args*/, unsigned int /*argsize*/)
{
	return 1; // of course the avatar can cheat ;-)
}

uint32 UCMachine::I_dummyProcess(const uint8* /*args*/, unsigned int /*argsize*/)
{
	return UCMachine::get_instance()->addProcess(new DelayProcess(4));
}

uint32 UCMachine::I_getName(const uint8* /*args*/, unsigned int /*argsize*/)
{
	return UCMachine::get_instance()->assignString("Avatar");
}

uint32 UCMachine::I_numToStr(const uint8* args, unsigned int /*argsize*/)
{
	ARG_SINT16(num);

	char buf[16]; // a 16 bit int should easily fit
	sprintf(buf, "%d", num);

	return UCMachine::get_instance()->assignString(buf);
}

uint32 UCMachine::I_urandom(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(num);
	if (num <= 1) return 0;

	// return random integer between 0 (incl.) to num (excl.)

	return (std::rand() % num);
}

uint32 UCMachine::I_rndRange(const uint8* args, unsigned int /*argsize*/)
{
	ARG_SINT16(lo);
	ARG_SINT16(hi);

	// return random integer between lo (incl.) to hi (incl.)

	if (hi <= lo) return lo;

	return (lo + (std::rand() % (hi-lo+1)));
}



// major hack number 2

uint16 targetObject = 0;
class TargetProcess : public Process
{
public:
	virtual bool run(const uint32 /*framenum*/) {
		if (targetObject != 0) {
			result = targetObject;
			// we're leaking strings and memory here... (not that I care)
			pout << "Target result = " << result << std::endl;
			terminate();
		}
		return false;
	}
};


uint32 UCMachine::I_target(const uint8* /*args*/, unsigned int /*argsize*/)
{
	targetObject = 0;

	pout << std::endl << std::endl << "Target: (select an object)"
		 << std::endl;

	return UCMachine::get_instance()->addProcess(new TargetProcess());
}
