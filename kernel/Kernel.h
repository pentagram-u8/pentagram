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

#ifndef KERNEL_H
#define KERNEL_H

#include <list>
#include <map>

#include "intrinsics.h"

class Process;
class idMan;
class IDataSource;
class ODataSource;

typedef Process* (*ProcessLoadFunc)(IDataSource*);

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
	bool runProcesses();
	Process* getProcess(ProcId pid);

	ProcId assignPID(Process* proc);

	void setNextProcess(Process *proc);
	Process* getRunningProcess() const { return runningprocess; }

	// only UCProcesses; objid = 0 means any object, type = 6 means any type
	uint32 getNumProcesses(ObjId objid, uint16 processtype);
	void killProcesses(ObjId objid, uint16 processtype);
	void killObjectProcesses();

	void kernelStats();

	void save(ODataSource* ods);
	bool load(IDataSource* ids);

	void pause() { paused++; }
	void unpause() { if (paused > 0) paused--; }
	bool isPaused() const { return paused > 0; }

	void addProcessLoader(std::string classname, ProcessLoadFunc func)
		{ processloaders[classname] = func; }

	uint32 getFrameNum() const { return framenum; };
	
	INTRINSIC(I_getNumProcesses);
	INTRINSIC(I_resetRef);
private:
	Process* loadProcess(IDataSource* ids);

	std::list<Process*> processes;
	idMan	*pIDs;

	std::list<Process*>::iterator current_process;

	std::map<std::string, ProcessLoadFunc> processloaders;

	bool loading;

	uint32 framenum;
	unsigned int paused;

	Process* runningprocess;

	static Kernel* kernel;
};

// a bit of a hack to prevent having to write a load function for
// every process
template<class T>
struct ProcessLoader {
	static Process* load(IDataSource* ids) {
		T* p = new T();
		bool ok = p->loadData(ids);
		if (!ok) {
			delete p;
			p = 0;
		}
		return p;
	}
};


#endif
