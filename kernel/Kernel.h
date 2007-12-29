/*
Copyright (C) 2002-2005 The Pentagram team

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

#ifndef KERNEL_H
#define KERNEL_H

#include <list>
#include <map>

#include "intrinsics.h"

class Process;
class idMan;
class IDataSource;
class ODataSource;

typedef Process* (*ProcessLoadFunc)(IDataSource*, uint32 version);
typedef std::list<Process*>::const_iterator ProcessIter;

class Kernel {
public:
	Kernel();
	~Kernel();

	static Kernel* get_instance() { return kernel; }

	void reset();

	ProcId addProcess(Process *proc); // returns pid of new process

	//! add a process and run it immediately
	//! \return pid of process
	ProcId addProcessExec(Process *proc); 

	void removeProcess(Process *proc);
	void runProcesses();
	Process* getProcess(ProcId pid);

	ProcId assignPID(Process* proc);

	void setNextProcess(Process *proc);
	Process* getRunningProcess() const { return runningprocess; }

	// objid = 0 means any object, type = 6 means any type
	uint32 getNumProcesses(ObjId objid, uint16 processtype);

	//! find a (any) process of the given objid, processtype
	Process* findProcess(ObjId objid, uint16 processtype);

	//! kill (fail) processes of a certain object and/or of a certain type
	//! \param objid the object, or 0 for any object (except objid 0)
	//! \param type the type, or 6 for any type
	//! \param fail if true, fail the processes instead of terminating them
	void killProcesses(ObjId objid, uint16 processtype, bool fail);

	//! kill (fail) processes of a certain object and not of a certain type
	//! \param objid the object, or 0 for any object (except objid 0)
	//! \param type the type not to kill
	//! \param fail if true, fail the processes instead of terminating them
	void killProcessesNotOfType(ObjId objid, uint16 processtype, bool fail);

	//! get an iterator of the process list.
	ProcessIter getProcessBeginIterator() { return processes.begin(); }
	ProcessIter getProcessEndIterator() { return processes.end(); }

	void kernelStats();
	void processTypes();

	void save(ODataSource* ods);
	bool load(IDataSource* ids, uint32 version);

	void pause() { paused++; }
	void unpause() { if (paused > 0) paused--; }
	bool isPaused() const { return paused > 0; }

	void setFrameByFrame(bool fbf) { framebyframe = fbf; }
	bool isFrameByFrame() const { return framebyframe; }

	void addProcessLoader(std::string classname, ProcessLoadFunc func)
		{ processloaders[classname] = func; }

	uint32 getFrameNum() const { return framenum; };

	//! "Kernel::processTypes" console command
	static void ConCmd_processTypes(const Console::ArgvType &argv);
	//! "Kernel::listProcesses" console command
	static void ConCmd_listProcesses(const Console::ArgvType &argv);
	//! "Kernel::processInfo" console command
	static void ConCmd_processInfo(const Console::ArgvType &argv);

	//! "Kernel::toggleFrameByFrame" console command
	static void ConCmd_toggleFrameByFrame(const Console::ArgvType &argv);
	//! "Kernel::advanceFrame" console command
	static void ConCmd_advanceFrame(const Console::ArgvType &argv);

	INTRINSIC(I_getNumProcesses);
	INTRINSIC(I_resetRef);
private:
	Process* loadProcess(IDataSource* ids, uint32 version);

	std::list<Process*> processes;
	idMan	*pIDs;

	std::list<Process*>::iterator current_process;

	std::map<std::string, ProcessLoadFunc> processloaders;

	bool loading;

	uint32 framenum;
	unsigned int paused;
	bool framebyframe;

	Process* runningprocess;

	static Kernel* kernel;
};

// a bit of a hack to prevent having to write a load function for
// every process
template<class T>
struct ProcessLoader {
	static Process* load(IDataSource* ids, uint32 version) {
		T* p = new T();
		bool ok = p->loadData(ids, version);
		if (!ok) {
			delete p;
			p = 0;
		}
		return p;
	}
};


#endif
