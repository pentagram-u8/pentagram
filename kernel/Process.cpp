/*
Copyright (C) 2003 The Pentagram team

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

#include "Process.h"
#include "Kernel.h"

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE_BASE_CLASS(Process);

void Process::terminate()
{
	Kernel *kernel = Kernel::get_instance();

	// wake up waiting processes
	for (std::vector<uint16>::iterator i = waiting.begin();
		 i != waiting.end(); ++i)
	{
		Process *p = kernel->getProcess(*i);
		if (p)
			p->wakeUp(result);
	}
	waiting.clear();

	terminated = true;
}

void Process::wakeUp(uint32 result_)
{
	result = result_;

	suspended = false;
}

void Process::waitFor(uint16 pid_)
{
	Kernel *kernel = Kernel::get_instance();

	// add this process to waiting list of process pid_
	Process *p = kernel->getProcess(pid_);
	assert(p);
	p->waiting.push_back(pid);

	suspended = true;
}
