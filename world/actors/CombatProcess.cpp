/*
Copyright (C) 2004 The Pentagram team

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

#include "CombatProcess.h"
#include "Actor.h"
#include "CurrentMap.h"
#include "World.h"
#include "UCList.h"
#include "LoopScript.h"
#include "WeaponInfo.h"
#include "AnimationTracker.h"
#include "Kernel.h"
#include "DelayProcess.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CombatProcess,Process);

CombatProcess::CombatProcess() : Process()
{

}

CombatProcess::CombatProcess(Actor* actor_)
{
	assert(actor_);
	item_num = actor_->getObjId();

	type = 0x00F2; // CONSTANT !
	target = 0;
	fixedTarget = 0;
}

bool CombatProcess::run(const uint32 /*framenum*/)
{
	Actor* a = World::get_instance()->getNPC(item_num);
	if (!(a->getFlags() & Item::FLG_FASTAREA)) return false;

	Actor* t = World::get_instance()->getNPC(target);

	if (!t || !isValidTarget(t)) {
		// no target? try to find one

		target = seekTarget();

		if (!target) {
			// still nothing? wait a bit (1-3 seconds)
			waitFor(Kernel::get_instance()->addProcess(
						new DelayProcess(30+(std::rand()%60))));
			return false;
		}

		pout << "[COMBAT " << item_num << "] target found: "
			 << target << std::endl;
	}

	if (inAttackRange()) {
		pout << "[COMBAT " << item_num << "] target in range" << std::endl;

		// attack
		waitFor(a->doAnim(Animation::attack, a->getDir()));
		return false;
	}

	waitFor(Kernel::get_instance()->addProcess(
				new DelayProcess(30+(std::rand()%60))));

	return false;
}

ObjId CombatProcess::getTarget()
{
	Actor* t = World::get_instance()->getNPC(target);

	if (!t || !isValidTarget(t))
		target = 0;

	return target;
}

void CombatProcess::setTarget(ObjId newtarget)
{
	if (fixedTarget == 0) {
		fixedTarget = newtarget; // want to prevent seekTarget from changing it
	}

	target = newtarget;
}

bool CombatProcess::isValidTarget(Actor* target)
{
	assert(target);

	// not in the fastarea
	if (!(target->getFlags() & Item::FLG_FASTAREA)) return false;

	// dead actors don't make good targets
	if (target->getActorFlags() & Actor::ACT_DEAD) return false;

	// feign death only works on undead and demons
	if (target->getActorFlags() & Actor::ACT_FEIGNDEATH) {

		Actor* a = World::get_instance()->getNPC(item_num);
		if (!a) return false; // uh oh

		if ((a->getDefenseType() & WeaponInfo::DMG_UNDEAD) || 
			(a->getShape() == 96)) return false; // CONSTANT!
	}

	// otherwise, ok
	return true;
}

bool CombatProcess::isEnemy(Actor* target)
{
	assert(target);

	Actor* a = World::get_instance()->getNPC(item_num);
	if (!a) return false; // uh oh

	return ((a->getEnemyAlignment() & target->getAlignment()) != 0);
}

ObjId CombatProcess::seekTarget()
{
	Actor* a = World::get_instance()->getNPC(item_num);
	if (!a) return 0; // uh oh

	if (fixedTarget) {
		Actor* t = World::get_instance()->getNPC(fixedTarget);
		if (isValidTarget(t))
			return fixedTarget; // no need to search
	}

	UCList itemlist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE);
	CurrentMap* cm = World::get_instance()->getCurrentMap();
	cm->areaSearch(&itemlist, script, sizeof(script), a, 768, false);

	for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
		Actor* t = World::get_instance()->getNPC(itemlist.getuint16(i));

		if (t && isValidTarget(t) && isEnemy(t)) {
			// found target
			return itemlist.getuint16(i);
		}
	}

	// no suitable targets
	return 0;
}

bool CombatProcess::inAttackRange()
{
	Actor* a = World::get_instance()->getNPC(item_num);
	AnimationTracker tracker(a, Animation::attack, a->getDir(), 0);

	while (tracker.step()) {
		if (tracker.hitSomething()) break;
	}

	ObjId hit = tracker.hitSomething();
	if (hit == target) return true;

	return false;
}

void CombatProcess::dumpInfo()
{
	Process::dumpInfo();
	pout << "Target: " << target << std::endl;
}

void CombatProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	ods->write2(target);
}

bool CombatProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	target = ids->read2();

	return true;
}
