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

#include "process.h"
#include "kernel.h"

typedef std::list<Process *>::iterator ProcessIterator;

Kernel::Kernel()
{
}

Kernel::~Kernel()
{
}

void Kernel::AddProcess(Process* proc)
{
	ProcessIterator it = processes.find (proc);

	if (it == processes.end()) {
		// POUT("Adding process");
		processes.insert(proc);
		proc->active = true;
	}
}

void Kernel::RemoveProcess(Process* proc)
{
	ProcessIterator it = processes.find (proc);

	if (it != processes.end()) {
        // POUT("Removing process");
		(*it)->active = false;
		processes.erase(it);
	}    
}

bool Kernel::RunProcesses(uint32 framenum)
{
    bool dirty = false;
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if ((*it)->Run(framenum)) dirty = true;
	}

	return dirty;
}
