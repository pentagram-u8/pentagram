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

#include "UCProcess.h"
#include "UCMachine.h"
#include "Usecode.h"

UCProcess::UCProcess(Usecode* usecode_, uint32 classid_,
					 uint32 offset_, uint32 this_ptr) :
	pid(0xFFFF), item_num(0), type(0), usecode(usecode_),
	classid(classid_), suspended(false)
{
	if (usecode->get_class_size(classid) == 0)
		perr << "Class is empty..." << std::endl;

	classid = 0xFFFF;
	ip = 0xFFFF;
	bp = 0x0000;

	stack.push4(this_ptr); // BP+06 this pointer

	call(classid_, offset_);
}


UCProcess::~UCProcess()
{

}

bool UCProcess::run(const uint32 framenum)
{
	// pass to UCMachine for execution

	return UCMachine::get_instance()->execProcess(this);
}

void UCProcess::call(uint32 classid_, uint32 offset_)
{
	stack.push2(classid); // BP+04 prev class
	stack.push2(ip);      // BP+02 prev IP
	stack.push2(bp);      // BP+00 prev BP

	classid = classid_;
	ip = offset_;
	bp = stack.getSP();
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