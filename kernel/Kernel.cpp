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
#include "IDataSource.h"
#include "ODataSource.h"
#include "ItemFactory.h"

#include "Actor.h"

#include "CoreApp.h" // only for getFrameNum

//#define DUMP_PROCESSTYPES

#ifdef DUMP_PROCESSTYPES
#include <map>
#endif


typedef std::list<Process *>::iterator ProcessIterator;

Kernel* Kernel::kernel = 0;

Kernel::Kernel() : loading(false)
{
	assert(kernel == 0);
	kernel = this;
	pIDs = new idMan(1,32767,128);
	current_process = processes.end();
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
}

uint16 Kernel::assignPID(Process* proc)
{
	// to prevent new processes from getting a PID while loading
	if (loading) return 0xFFFF;

	// Get a pID
	proc->pid = pIDs->getNewID();

	return proc->pid;
}

uint16 Kernel::addProcess(Process* proc)
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

uint16 Kernel::addProcessExec(Process* proc)
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

	proc->run(CoreApp::get_instance()->getFrameNum());
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

		//! do this in a cleaner way
		exit(0);
	}

	bool dirty = false;
	current_process = processes.begin();
	while (current_process != processes.end()) {
		Process* p = *current_process;

		if (p->terminate_deferred)
			p->terminate();
		if (!p->terminated && !p->suspended)
			if (p->run(framenum)) dirty = true;
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
#ifdef DUMP_PROCESSTYPES
	std::map<std::string, unsigned int> processtypes;
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it) {
		Process* p = *it;
		processtypes[p->GetClassType().class_name]++;
	}
#endif

	pout << "Kernel memory stats:" << std::endl;
	pout << "Processes  : " << processes.size() << "/32765" << std::endl;
#ifdef DUMP_PROCESSTYPES
	std::map<std::string, unsigned int>::iterator iter;
	for (iter = processtypes.begin(); iter != processtypes.end(); ++iter) {
		pout << (*iter).first << ": " << (*iter).second << std::endl;
	}
#endif
}


uint32 Kernel::getNumProcesses(uint16 objid, uint16 processtype)
{
	if(objid==0 && processtype==6)
		return processes.size();
	
	uint32 count = 0;

	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		// Don't count us, were are not really here
		if (p->terminate_deferred) continue;

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

void Kernel::killObjectProcesses()
{
	for (ProcessIterator it = processes.begin(); it != processes.end(); ++it)
	{
		Process* p = *it;

		if (p->item_num != 0)
			p->terminate();
	}
}

void Kernel::save(ODataSource* ods)
{
	ods->write2(1); // kernel savegame version 1
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
