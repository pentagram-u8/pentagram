/*
Copyright (C) 2003-2004 The Pentagram team

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
#include "IDataSource.h"
#include "ODataSource.h"
#include "MemoryManager.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(Process);

DEFINE_CUSTOM_MEMORY_ALLOCATION(Process, MemoryManager::processAllocator);

Process::Process(ObjId it, uint16 ty)
	: pid(0xFFFF), active(false), suspended(false), terminated(false),
	terminate_deferred(false), item_num(it), type(ty), result(0)
{
	Kernel::get_instance()->assignPID(this);
}

void Process::fail()
{
	assert(!terminated);

	Kernel *kernel = Kernel::get_instance();

	// fail all waiting processes
	for (std::vector<ProcId>::iterator i = waiting.begin();
		 i != waiting.end(); ++i)
	{
		Process *p = kernel->getProcess(*i);
		if (p && !p->terminated) {
			if (p->type == 1)
				p->wakeUp(0); // CHECKME: this probably isn't right
			else
				p->fail();
		}
	}
	waiting.clear(); // to prevent terminate() from waking them again

	terminate();
}

void Process::terminate()
{
	assert(!terminated);

	Kernel *kernel = Kernel::get_instance();

	// wake up waiting processes
	for (std::vector<ProcId>::iterator i = waiting.begin();
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

	Kernel::get_instance()->setNextProcess(this);
}

void Process::waitFor(ProcId pid_)
{
	if (pid_) {
		Kernel *kernel = Kernel::get_instance();
		
		// add this process to waiting list of process pid_
		Process *p = kernel->getProcess(pid_);
		assert(p);
		p->waiting.push_back(pid);
	}

	suspended = true;
}

void Process::waitFor(Process* proc)
{
	ProcId pid_ = 0;
	if (proc) pid_ = proc->getPid();

	waitFor(pid_);
}

void Process::suspend()
{
	suspended = true;
}

void Process::dumpInfo()
{
	pout << "Process " << getPid() << " class "
		 << GetClassType().class_name << ", item " << item_num
		 << ", type " << std::hex << type << std::dec << ", status ";
	if (active) pout << "A";
	if (suspended) pout << "S";
	if (terminated) pout << "T";
	if (terminate_deferred) pout << "t";
	if (!waiting.empty()) {
		pout << ", notify: ";
		for (std::vector<ProcId>::iterator i = waiting.begin();
			 i != waiting.end(); ++i)
		{
			if (i != waiting.begin()) pout << ", ";
			pout << *i;
		}
	}
	pout << std::endl;
}

void Process::save(ODataSource* ods)
{
	writeProcessHeader(ods);
	saveData(ods); // virtual
}

void Process::writeProcessHeader(ODataSource* ods)
{
	const char* cname = GetClassType().class_name; // virtual
	uint16 clen = strlen(cname);

	ods->write2(clen);
	ods->write(cname, clen);
}

void Process::saveData(ODataSource* ods)
{
	ods->write2(pid);
	uint8 a = active ? 1 : 0;
	uint8 s = suspended ? 1 : 0;
	uint8 t = terminated ? 1 : 0;
	uint8 td = terminate_deferred ? 1 : 0;
	ods->write1(a);
	ods->write1(s);
	ods->write1(t);
	ods->write1(td);
	ods->write2(item_num);
	ods->write2(type);
	ods->write4(result);
	ods->write4(waiting.size());
	for (unsigned int i = 0; i < waiting.size(); ++i)
		ods->write2(waiting[i]);
}

bool Process::loadData(IDataSource* ids, uint32 version)
{
	pid = ids->read2();
	active = (ids->read1() != 0);
	suspended = (ids->read1() != 0);
	terminated = (ids->read1() != 0);
	terminate_deferred = (ids->read1() != 0);
	item_num = ids->read2();
	type = ids->read2();
	result = ids->read4();
	uint32 waitcount = ids->read4();
	waiting.resize(waitcount);
	for (unsigned int i = 0; i < waitcount; ++i)
		waiting[i] = ids->read2();

	return true;
}
