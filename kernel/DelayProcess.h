/*
Copyright (C) 2004-2007 The Pentagram team

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
	explicit DelayProcess(int count_=0);
	virtual ~DelayProcess();

	ENABLE_RUNTIME_CLASSTYPE();

	virtual void run();

	virtual void dumpInfo();

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	int count;
};

#endif
