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

#ifndef PROCESS_H
#define PROCESS_H

#include <vector>

class Process {
public:

	friend class Kernel;

	// returns true if screen needs to be repainted
	virtual bool run(const uint32 framenum) = 0;

	Process() : 
		pid(0xFFFF), active(false), suspended(false), terminated(false),
		item_num(0), type(0), result(0)
	{ }
	virtual ~Process() { }

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	bool is_active() const { return active; }

	virtual void terminate();
	void waitFor(uint16 pid);
	void wakeUp(uint32 result);

protected:
	// process id
	uint16 pid;

	bool active; // is the process in the run-list?
	bool suspended; // suspended? (because it's waiting for something)
	                //! (this may have to be a count instead, if a process
	                //! is waiting for more than one others)
	bool terminated;

	// item we are assigned to
	uint16 item_num;
	uint16 type;

	// process result
	uint32 result;

	// processes waiting for this one to finish
	// when this process terminates, awaken them and pass them the result val.
	std::vector<uint16> waiting;
};


#endif
