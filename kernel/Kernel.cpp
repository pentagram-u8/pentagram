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

#include "Process.h"
#include "Kernel.h"

typedef std::list<Process *>::iterator ProcessIterator;

Kernel* Kernel::kernel = 0;

Kernel::Kernel()
{
	kernel = this;
}

Kernel::~Kernel()
{
}

void Kernel::addProcess(Process* proc)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return;
	}

	perr << "[Kernel] Adding process " << proc << std::endl;

	processes.push_front(proc);
	proc->active = true;
}

void Kernel::removeProcess(Process* proc)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc) {
			proc->active = false;
			processes.erase(it);
			return;
		}
	}
}

bool Kernel::runProcesses(uint32 framenum)
{
	if (processes.size() == 0) {
		perr << "Process queue is empty?! Aborting.\n";

		//! do this in a cleaner way
		exit(0);
	}

	bool dirty = false;
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process* p = *it;
		if (!p->terminated)
			if (p->run(framenum)) dirty = true;
		if (p->terminated) {
			// process is killed, so remove it from the list
			it = processes.erase(it);

			//! is this the right place to delete processes?
			delete p;
		}
	}

	return dirty;
}
