/*
Copyright (C) 2002-2004 The Pentagram team

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

#include "IDataSource.h"
#include "ODataSource.h"
#include "ItemFactory.h"

#include <map>

typedef std::list<Process *>::iterator ProcessIterator;

Kernel* Kernel::kernel = 0;

Kernel::Kernel() : loading(false)
{
	assert(kernel == 0);
	kernel = this;
	pIDs = new idMan(1,32767,128);
	current_process = processes.end();
	framenum = 0;
	paused = 0;
	runningprocess = 0;
	framebyframe = false;
}

Kernel::~Kernel()
{
	kernel = 0;

	delete pIDs;
}

void Kernel::reset()
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		delete (*it);
	}
	processes.clear();
	current_process = processes.begin();

	pIDs->clearAll();

	paused = 0;
	runningprocess = 0;

	// if we're in frame-by-frame mode, reset to a paused state
	if (framebyframe) paused = 1;
}

ProcId Kernel::assignPID(Process* proc)
{
	// to prevent new processes from getting a PID while loading
	if (loading) return 0xFFFF;

	// Get a pID
	proc->pid = pIDs->getNewID();

	return proc->pid;
}

ProcId Kernel::addProcess(Process* proc)
{
#if 0
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return 0;
	}
#endif

	assert(proc->pid != 0 && proc->pid != 0xFFFF);

#if 0
	perr << "[Kernel] Adding process " << proc
		 << ", pid = " << proc->pid << std::endl;
#endif

//	processes.push_back(proc);
//	proc->active = true;
	setNextProcess(proc);
	return proc->pid;
}

ProcId Kernel::addProcessExec(Process* proc)
{
#if 0
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		if (*it == proc)
			return 0;
	}
#endif

	assert(proc->pid != 0 && proc->pid != 0xFFFF);

#if 0
	perr << "[Kernel] Adding process " << proc
		 << ", pid = " << proc->pid << std::endl;
#endif

	processes.push_back(proc);
	proc->active = true;

	Process* oldrunning = runningprocess; runningprocess = proc;
	proc->run(framenum);
	runningprocess = oldrunning;

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
bool Kernel::runProcesses()
{
	if (paused) return false;

	framenum++;

	if (processes.size() == 0) {
		return true;
		perr << "Process queue is empty?! Aborting.\n";

		//! do this in a cleaner way
		exit(0);
	}
	bool dirty = false;
	current_process = processes.begin();
	while (current_process != processes.end()) {
		Process* p = *current_process;

		if (!p->terminated && p->terminate_deferred)
			p->terminate();
		if (!p->terminated && !p->suspended) {
			runningprocess = p;
			bool ret = p->run(framenum);
			runningprocess = 0;

			if (ret) dirty = true;
		}
		if (p->terminated) {
			// process is killed, so remove it from the list
			current_process = processes.erase(current_process);

			// Clear pid
			pIDs->clearID(p->pid);

			//! is this the right place to delete processes?
			delete p;
		}
		else
			++current_process;
	}

	if (framebyframe) pause();

	return dirty;
}

void Kernel::setNextProcess(Process* proc)
{
	if (current_process != processes.end() && *current_process == proc) return;

	if (proc->active) {
		for (ProcessIterator it = processes.begin();
			 it != processes.end(); ++it) {
			if (*it == proc) {
				processes.erase(it);
				break;
			}
		}
	} else {
		proc->active = true;
	}

	if (current_process == processes.end()) {
		processes.push_front(proc);
	} else {
		ProcessIterator t = current_process;
		++t;

		processes.insert(t, proc);
	}
}

Process* Kernel::getProcess(ProcId pid)
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
	pout << "Processes  : " << processes.size() << "/32765" << std::endl;
}

void Kernel::processTypes()
{
	pout << "Current process types:" << std::endl;
	std::map<std::string, unsigned int> processtypes;
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process* p = *it;
		processtypes[p->GetClassType().class_name]++;
	}
	std::map<std::string, unsigned int>::iterator iter;
	for (iter = processtypes.begin(); iter != processtypes.end(); ++iter) {
		pout << (*iter).first << ": " << (*iter).second << std::endl;
	}	
}

void Kernel::ConCmd_processTypes(const Console::ArgsType & /*args*/, const Console::ArgvType & /*argv*/)
{
	Kernel::get_instance()->processTypes();
}

void Kernel::ConCmd_listItemProcesses(const Console::ArgsType & /*args*/, const Console::ArgvType &argv)
{
	if (argv.size() != 2) {
		pout << "usage: listItemProcesses <itemnum>" << std::endl;
		return;
	}

	ObjId item = strtol(argv[1].c_str(), 0, 0);
	Kernel* kernel = Kernel::get_instance();

	pout << "Processes for item " << item << ":" << std::endl;
	for (ProcessIterator it = kernel->processes.begin();
		 it != kernel->processes.end(); ++it)
	{
		Process* p = *it;
		if (p->item_num == item)
			p->dumpInfo();
	}

}

uint32 Kernel::getNumProcesses(ObjId objid, uint16 processtype)
{
	uint32 count = 0;

	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		// Don't count us, we are not really here
		if (p->terminate_deferred) continue;

		if ((objid == 0 || objid == p->item_num) &&
			(processtype == 6 || processtype == p->type))
			count++;
	}

	return count;
}

Process* Kernel::findProcess(ObjId objid, uint16 processtype)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		// Don't count us, were are not really here
		if (p->terminate_deferred) continue;

		if ((objid == 0 || objid == p->item_num) &&
			(processtype == 6 || processtype == p->type))
		{
			return p;
		}
	}

	return 0;
}


void Kernel::killProcesses(ObjId objid, uint16 processtype, bool fail)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		if (p->item_num != 0 && (objid == 0 || objid == p->item_num) &&
			(processtype == 6 || processtype == p->type) && !p->terminated)
		{
			if (fail)
				p->fail();
			else
				p->terminate();
		}
	}
}

void Kernel::killProcessesNotOfType(ObjId objid, uint16 processtype, bool fail)
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		if (p->item_num != 0 && (objid == 0 || objid == p->item_num) &&
			(p->type != processtype) && !p->terminated)
		{
			if (fail)
				p->fail();
			else
				p->terminate();
		}
	}
}

void Kernel::save(ODataSource* ods)
{
	ods->write2(1); // kernel savegame version 1
	ods->write4(framenum);
	pIDs->save(ods);
	ods->write4(processes.size());
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		(*it)->save(ods);
	}
}

bool Kernel::load(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;

	framenum = ids->read4();

	if (!pIDs->load(ids)) return false;

	uint32 pcount = ids->read4();

	for (unsigned int i = 0; i < pcount; ++i) {
		Process* p = loadProcess(ids);
		if (!p) return false;
		processes.push_back(p);
	}

	return true;
}

Process* Kernel::loadProcess(IDataSource* ids)
{
	uint16 classlen = ids->read2();
	char* buf = new char[classlen+1];
	ids->read(buf, classlen);
	buf[classlen] = 0;

	std::string classname = buf;
	delete[] buf;

	std::map<std::string, ProcessLoadFunc>::iterator iter;
	iter = processloaders.find(classname);

	if (iter == processloaders.end()) {
		perr << "Unknown Process class: " << classname << std::endl;
		return 0;
	}


	loading = true;

	Process* p = (*(iter->second))(ids);

	loading = false;

	return p;
}

uint32 Kernel::I_getNumProcesses(const uint8* args, unsigned int /*argsize*/)
{
	ARG_OBJID(item);
	ARG_UINT16(type);

	return Kernel::get_instance()->getNumProcesses(item, type);
}

uint32 Kernel::I_resetRef(const uint8* args, unsigned int /*argsize*/)
{
	ARG_OBJID(item);
	ARG_UINT16(type);

	Kernel::get_instance()->killProcesses(item, type, true);
	return 0;
}
