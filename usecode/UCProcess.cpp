/*
Copyright (C) 2002-2003 The Pentagram team

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

#include "UCProcess.h"
#include "UCMachine.h"
#include "Usecode.h"

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(UCProcess,Process);

UCProcess::	UCProcess(Usecode* usecode_)
	: item_num(0), type(0), usecode(usecode_), classid(0)
{
	classid = 0xFFFF;
	ip = 0xFFFF;
	bp = 0x0000;
}

UCProcess::~UCProcess()
{

}

void UCProcess::load(uint16 classid_, uint16 offset_, uint32 this_ptr,
					 int thissize, const uint8* args, int argsize)
{
	if (usecode->get_class_size(classid_) == 0)
		perr << "Class is empty..." << std::endl;

	classid = 0xFFFF;
	ip = 0xFFFF;
	bp = 0x0000;
	uint16 thissp = 0;

	// first, push the derefenced this pointer
	if (this_ptr != 0 && thissize > 0) {
		stack.addSP(-thissize);
		UCMachine::get_instance()->
			dereferencePointer(this_ptr, const_cast<uint8*>(stack.access()),
							   thissize);
		thissp = stack.getSP();
	}

	// next, push the arguments
	stack.push(args, argsize);

	// then, push the new this pointer
	if (thissp) {
		stack.push4(UCMachine::stackToPtr(pid, thissp));
	} else {
		stack.push4(0);
	}


	// finally, call the specified function
	call(classid_, offset_);
}

bool UCProcess::run(const uint32 /*framenum*/)
{
	if (suspended)
		return false;

	// pass to UCMachine for execution

	return UCMachine::get_instance()->execProcess(this);
}

void UCProcess::call(uint16 classid_, uint16 offset_)
{
	stack.push2(classid); // BP+04 prev class
	stack.push2(ip);      // BP+02 prev IP
	stack.push2(bp);      // BP+00 prev BP

	classid = classid_;
	ip = offset_;
	bp = static_cast<uint16>(stack.getSP()); // TRUNCATES!
}

bool UCProcess::ret()
{
	stack.moveSP(bp);

	bp = stack.pop2();
	ip = stack.pop2();
	classid = stack.pop2();

	if (ip == 0xFFFF && classid == 0xFFFF)
		return true;
	else
		return false;
}
