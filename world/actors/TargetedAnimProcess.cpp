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

#include "TargetedAnimProcess.h"
#include "AnimationTracker.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(TargetedAnimProcess,ActorAnimProcess);

TargetedAnimProcess::TargetedAnimProcess() : ActorAnimProcess()
{

}

TargetedAnimProcess::TargetedAnimProcess(Actor* actor_, Animation::Sequence action, uint32 dir_, sint32 coords[3]) : ActorAnimProcess(actor_, action, dir_)
{
	x = coords[0];
	y = coords[1];
	z = coords[2];
}

bool TargetedAnimProcess::init()
{
	if (!ActorAnimProcess::init())
		return false;

	tracker->setTargetedMode(x, y, z);
	return true;
}


void TargetedAnimProcess::saveData(ODataSource* ods)
{
	ActorAnimProcess::saveData(ods);

	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(z));

}

bool TargetedAnimProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!ActorAnimProcess::loadData(ids, version)) return false;

	x = ids->read4();
	y = ids->read4();
	z = ids->read4();

	return true;
}
