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

#include "Kernel.h"
#include "Process.h"
#include "idMan.h"

#include "UCProcess.h"

#include "UCMachine.h" // only for usecodeStats.
#include "World.h" // only for worldStats

typedef std::list<Process *>::iterator ProcessIterator;

Kernel* Kernel::kernel = 0;

Kernel::Kernel()
{
	assert(kernel == 0);
	kernel = this;
	pIDs = new idMan(1,32767);
}

Kernel::~Kernel()
{
	kernel = 0;
}

uint16 Kernel::addProcess(Process* proc)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return 0;
	}

	// Get a pID
	proc->pid = pIDs->getNewID();

	perr << "[Kernel] Adding process " << proc
		 << ", pid = " << proc->pid << std::endl;

	processes.push_front(proc);
	proc->active = true;

	return proc->pid;
}

void Kernel::removeProcess(Process* proc)
{
	//! the way to remove processes has to be thought over sometime
	//! we probably want to flag them as terminated before actually
	//! removing/deleting it or something
	//! also have to look out for deleting processes while iterating
	//! over the list. (Hence the special 'erase' in runProcs below, which
	//! is very std::list-specific, incidentally)

	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc) {
			proc->active = false;
			
			perr << "[Kernel] Removing process " << proc << std::endl;

			processes.erase(it);

			// Clear pid
			pIDs->clearID(proc->pid);

			return;
		}
	}
}


//Q: is returning a 'dirty' flag really useful?
bool Kernel::runProcesses(uint32 framenum)
{
	if (processes.size() == 0) {
		return true;//
		perr << "Process queue is empty?! Aborting.\n";

		kernelStats();
		UCMachine::get_instance()->usecodeStats();
		World::get_instance()->worldStats();

		//! do this in a cleaner way
		exit(0);
	}

	bool dirty = false;
	ProcessIterator it = processes.begin();
	while (it != processes.end()) {
		Process* p = *it;
		if (!p->terminated)
			if (p->run(framenum)) dirty = true;
		if (p->terminated) {
			// process is killed, so remove it from the list
			it = processes.erase(it);

			// Clear pid
			pIDs->clearID(p->pid);

			//! is this the right place to delete processes?
			delete p;
		}
		else
			++it;
	}

	return dirty;
}

Process* Kernel::getProcess(uint16 pid)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process* p = *it;
		if (p->pid == pid)
			return p;
	}
	return 0;
}

void Kernel::kernelStats()
{
	pout << "Kernel memory stats:" << std::endl;
	pout << "Processes : " << processes.size() << "/32765" << std::endl;
}


uint32 Kernel::getNumProcesses(uint16 objid, uint16 processtype)
{
	uint32 count = 0;

	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		if ((objid == 0 || objid == p->item_num) &&
			(processtype == 6 || processtype == p->type))
			count++;
	}

	return count;
}

void Kernel::killProcesses(uint16 objid, uint16 processtype)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		if ((objid == 0 || objid == p->item_num) &&
			(processtype == 6 || processtype == p->type))
			p->terminate();
	}
}

uint32 Kernel::I_getNumProcesses(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(item);
	ARG_UINT16(type);

	return Kernel::get_instance()->getNumProcesses(item, type);
}

uint32 Kernel::I_resetRef(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(item);
	ARG_UINT16(type);

	Kernel::get_instance()->killProcesses(item, type);
	return 0;
}
