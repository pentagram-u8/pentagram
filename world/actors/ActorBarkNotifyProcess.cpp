/*
 *  Copyright (C) 2005-2007  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "ActorBarkNotifyProcess.h"
#include "Gump.h"
#include "DelayProcess.h"
#include "Actor.h"
#include "Animation.h"
#include "Kernel.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ActorBarkNotifyProcess,GumpNotifyProcess);

ActorBarkNotifyProcess::ActorBarkNotifyProcess()
	: GumpNotifyProcess()
{

}

ActorBarkNotifyProcess::ActorBarkNotifyProcess(uint16 it)
	: GumpNotifyProcess(it)
{

}

ActorBarkNotifyProcess::~ActorBarkNotifyProcess(void)
{

}


void ActorBarkNotifyProcess::run()
{
	Actor* a = getActor(item_num);
	if (!a) return;

	if (a->isDead() || !a->hasAnim(Animation::talk))
		return;

	bool doAnim = true;

	// if not standing or talking, don't do talk animation
	Animation::Sequence lastanim = a->getLastAnim();
	if (lastanim != Animation::stand && lastanim != Animation::talk)
		doAnim = false;
	else if (Kernel::get_instance()->getNumProcesses(item_num, 0x00F0) > 0)
		// if busy, don't do talk animation
		doAnim = false;

	// wait a short while (1-2.5 seconds) before doing the next animation
	// (or even if not doing the animation)
	Process* delayproc = new DelayProcess(30+(std::rand()%45));
	ProcId delaypid = Kernel::get_instance()->addProcess(delayproc);

	if (doAnim)
		a->doAnim(Animation::talk, 8);

	waitFor(delaypid);
}

void ActorBarkNotifyProcess::saveData(ODataSource* ods)
{
	GumpNotifyProcess::saveData(ods);
}

bool ActorBarkNotifyProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!GumpNotifyProcess::loadData(ids, version)) return false;

	return true;
}


