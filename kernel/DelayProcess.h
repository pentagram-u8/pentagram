/*
Copyright (C) 2003 The Pentagram team

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

#ifndef DELAYPROCESS_H
#define DELAYPROCESS_H

#include "Process.h"
#include "IDataSource.h"
#include "ODataSource.h"

// a process that waits a number of ticks before terminating

class DelayProcess : public Process {
public:
	explicit DelayProcess(int count_=0) : Process(), count(count_) { }
	virtual ~DelayProcess() { }

	virtual bool run(const uint32 /*framenum*/) { if (--count == 0) terminate(); return false; }

	bool loadData(IDataSource* ids)
	{
		uint16 version = ids->read2();
		if (version != 1) return false;
		if (!Process::loadData(ids)) return false;

		count = static_cast<int>(ids->read4());

		return true;
	}

protected:

	virtual void saveData(ODataSource* ods)
	{
		ods->write2(1); //version
		Process::saveData(ods);
		ods->write4(static_cast<uint32>(count));
	}

	int count;
};

#endif
