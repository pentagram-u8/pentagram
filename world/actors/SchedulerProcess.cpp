/*
Copyright (C) 2005 The Pentagram team

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

#include "SchedulerProcess.h"
#include "Actor.h"
#include "GUIApp.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(SchedulerProcess,Process);

SchedulerProcess::SchedulerProcess() : Process()
{
	lastRun = 0;
	nextActor = 0;
	type = 0x245; // CONSTANT!
}

void SchedulerProcess::run()
{
	if (nextActor != 0) {
		// doing a scheduling run at the moment

		Actor* a = getActor(nextActor);
		if (a) {
			// CHECKME: is this the right time to pass? CONSTANT
			uint32 stime = GUIApp::get_instance()->getGameTimeInSeconds()/60;
			ProcId schedpid = a->callUsecodeEvent_schedule(stime);
			if (schedpid) waitFor(schedpid);
		}

		nextActor++;
		if (nextActor == 256) { // CONSTANT
			nextActor = 0; // done
#if 0
			pout << "Scheduler: finished run at "
				 << Kernel::get_instance()->getFrameNum() << std::endl;
#endif
		}

		return;
	}

	// CONSTANT!
	uint32 currenthour = GUIApp::get_instance()->getGameTimeInSeconds()/900;

	if (currenthour > lastRun) {
		// schedule a new scheduling run
		lastRun = currenthour;
		nextActor = 1;
#if 0
		pout << "Scheduler:  " << Kernel::get_instance()->getFrameNum()
			 << std::endl;
#endif
	}
}

void SchedulerProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write4(lastRun);
	ods->write2(nextActor);
}

bool SchedulerProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	lastRun = ids->read4();
	nextActor = ids->read2();

	return true;
}
