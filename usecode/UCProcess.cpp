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

UCProcess::UCProcess(Usecode* usecode_, uint32 classid_, uint32 offset_) :
	pid(0xFFFF), item_num(0), type(0), usecode(usecode_),
	classid(classid_), cs(0,0), suspended(false)
{
	if (usecode->get_class_size(classid) == 0)
		perr << "Class is empty..." << std::endl;

	// setup code segment
	cs.load(usecode->get_class(classid),
			usecode->get_class_size(classid));
	// seek to correct offset
	cs.seek(offset_);

	// setup stack (push a 'last-stack-frame marker')
	stack.push4(0x11223344);	// BP+06 This pointer
	stack.push2(0xFFFF);		// BP+04 Prev CLASS
	stack.push2(0xFFFF);		// BP+02 Prev IP
	stack.push2(0x0000);		// BP+00 Prev BP

	bp = stack.getSP();
}


UCProcess::~UCProcess()
{

}

bool UCProcess::run(const uint32 framenum)
{
	// pass to UCMachine for execution

	return UCMachine::get_instance()->execProcess(this);
}
