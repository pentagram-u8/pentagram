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
#include "QuickAvatarMoverProcess.h"

#include "MainActor.h"
#include "World.h"
#include "CurrentMap.h"
#include "Kernel.h"
#include "GUIApp.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(QuickAvatarMoverProcess,Process);

ProcId QuickAvatarMoverProcess::amp[6] = { 0, 0, 0, 0, 0, 0 };
bool QuickAvatarMoverProcess::clipping = false;
bool QuickAvatarMoverProcess::quarter = false;

QuickAvatarMoverProcess::QuickAvatarMoverProcess() : Process(1), dx(0), dy(0), dz(0), dir(0)
{
}

QuickAvatarMoverProcess::QuickAvatarMoverProcess(int x, int y, int z, int _dir) : Process(1), dx(x), dy(y), dz(z), dir(_dir)
{
	QuickAvatarMoverProcess::terminateMover(dir);
	assert(dir < 6);
	amp[dir] = getPid();
}

QuickAvatarMoverProcess::~QuickAvatarMoverProcess()
{
}

bool QuickAvatarMoverProcess::run(const uint32 /*framenum*/)
{
	if (GUIApp::get_instance()->isAvatarInStasis())
	{
		terminate();
		return false;
	}

	MainActor * avatar = World::get_instance()->getMainActor();
	sint32 x,y,z;
	avatar->getLocation(x,y,z);
	sint32 ixd,iyd,izd;
	avatar->getFootpadWorld(ixd, iyd, izd);

	CurrentMap* cm = World::get_instance()->getCurrentMap();

	sint32 dx = this->dx;
	sint32 dy = this->dy;
	sint32 dz = this->dz;

	for (int j = 0; j < 3; j++)
	{
		dx = this->dx;
		dy = this->dy;
		dz = this->dz;

		if (j == 1) dx = 0;
		else if (j == 2) dy = 0;

		if (quarter)
		{
			dx /= 4;
			dy /= 4;
			dz /= 4;
		}

		bool ok = false;

		while (dx || dy || dz) {

			if (!clipping || cm->isValidPosition(x+dx,y+dy,z+dz,ixd,iyd,izd,1,0,0))
			{
				if (clipping && !dz)
				{
					if (cm->isValidPosition(x+dx,y+dy,z-8,ixd,iyd,izd,1,0,0) &&
							!cm->isValidPosition(x,y,z-8,ixd,iyd,izd,1,0,0))
					{
						dz = -8;
					}
					else if (cm->isValidPosition(x+dx,y+dy,z-16,ixd,iyd,izd,1,0,0) &&
							!cm->isValidPosition(x,y,z-16,ixd,iyd,izd,1,0,0))
					{
						dz = -16;
					}
					else if (cm->isValidPosition(x+dx,y+dy,z-24,ixd,iyd,izd,1,0,0) &&
							!cm->isValidPosition(x,y,z-24,ixd,iyd,izd,1,0,0))
					{
						dz = -24;
					}
					else if (cm->isValidPosition(x+dx,y+dy,z-32,ixd,iyd,izd,1,0,0) &&
							!cm->isValidPosition(x,y,z-32,ixd,iyd,izd,1,0,0))
					{
						dz = -32;
					}
				}
				ok = true;
				break;
			}
			else if (cm->isValidPosition(x+dx,y+dy,z+dz+8,ixd,iyd,izd,1,0,0))
			{
				dz+=8;
				ok = true;
				break;
			}
			dx/=2;
			dy/=2;
			dz/=2;
		}
		if (ok) break;
	}

	// Yes, i know, not entirely correct
	avatar->collideMove(x+dx,y+dy,z+dz, false, true);
	return true;
}

void QuickAvatarMoverProcess::terminate()
{
	Process::terminate();
	amp[dir] = 0;
}

void QuickAvatarMoverProcess::terminateMover(int _dir)
{
	assert(_dir < 6);

	Kernel * kernel = Kernel::get_instance();

	QuickAvatarMoverProcess * p =
		p_dynamic_cast<QuickAvatarMoverProcess *>(kernel->getProcess(amp[_dir]));

	if (p && !(p->flags & PROC_TERMINATED))
		p->terminate();
}

void QuickAvatarMoverProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write4(dir);
	// don't save more information. We plan to terminate upon load
}

bool QuickAvatarMoverProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	// small safety precaution
	dir = ids->read4();
	if (dir < 6)
		amp[dir] = 0;
	else
		return false;

	terminateDeferred(); // Don't allow this process to continue
	return true;
}
