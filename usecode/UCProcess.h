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

#ifndef UCPROCESS_H
#define UCPROCESS_H

#include <list>
#include "Process.h"
#include "UCStack.h"

class Usecode;
class IDataSource;
class ODataSource;


// probably won't inherit from Process directly in the future
class UCProcess : public Process
{
	friend class UCMachine;
	friend class Kernel;
public:
	UCProcess();
	UCProcess(uint16 classid_, uint16 offset_, uint32 this_ptr = 0,
			  int thissize = 0, const uint8* args = 0, int argsize = 0);
    ~UCProcess();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	virtual void run();

	virtual void terminate();

	void freeOnTerminate(uint16 index, int type);

	void setReturnValue(uint32 retval) { temp32 = retval; }

	//! dump some info about this process to pout
	virtual void dumpInfo();

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	void load(uint16 classid_, uint16 offset_, uint32 this_ptr = 0,
			  int thissize = 0, const uint8* args = 0, int argsize = 0);
	void call(uint16 classid_, uint16 offset_);
	bool ret();

	// stack base pointer
	uint16 bp;

	Usecode* usecode;

	uint16 classid;
	uint16 ip;

	uint32 temp32;

	// data stack
	UCStack stack;

	// "Free Me" list
	std::list<std::pair<uint16, int> > freeonterminate;
};

#endif
