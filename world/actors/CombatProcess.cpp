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
#include "PathfinderProcess.h"

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
	combatmode = CM_WAITING;
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
		combatmode = CM_WAITING;
	}

	int targetdir = getTargetDirection();
	if (a->getDir() != targetdir) {
		turnToDirection(targetdir);
		return false;
	}

	if (inAttackRange()) {
		combatmode = CM_ATTACKING;

		pout << "[COMBAT " << item_num << "] target in range" << std::endl;

		// attack
		ProcId attackanim = a->doAnim(Animation::attack, a->getDir());

		// wait a while, depending on dexterity, before attacking again
		int dex = a->getDex();
		if (dex < 25) {
			int recoverytime = 3 * (25 - dex);
			Process* waitproc = new DelayProcess(recoverytime);
			ProcId waitpid = Kernel::get_instance()->addProcess(waitproc);
			waitproc->waitFor(attackanim);
			waitFor(waitpid);
		} else {
			waitFor(attackanim);
		}

		return false;
	} else if (combatmode != CM_PATHFINDING) {
		// not in range? See if we can get in range

		Process* pfproc = new PathfinderProcess(a, target);
		
		waitFor(Kernel::get_instance()->addProcess(pfproc));
		combatmode = CM_PATHFINDING;
		return false;
	}

	waitFor(Kernel::get_instance()->addProcess(
				new DelayProcess(20+(std::rand()%40))));
	combatmode = CM_WAITING;

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
		if (t && isValidTarget(t))
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

int CombatProcess::getTargetDirection()
{
	Actor* a = World::get_instance()->getNPC(item_num);
	Actor* t = World::get_instance()->getNPC(target);

	return a->getDirToItemCentre(*t);
}

void CombatProcess::turnToDirection(int direction)
{
	Actor* a = World::get_instance()->getNPC(item_num);
	int curdir = a->getDir();
	int step = 1;
	if ((curdir - direction + 8) % 8 < 4) step = -1;
	Animation::Sequence turnanim = Animation::combat_stand;

	ProcId prevpid = 0;
	bool done = false;

	for (int dir = curdir; !done; ) {
		ProcId animpid = a->doAnim(turnanim, dir);

		if (dir == direction) done = true;

		if (prevpid) {
			Process* proc = Kernel::get_instance()->getProcess(animpid);
			assert(proc);
			proc->waitFor(prevpid);
		}

		prevpid = animpid;

		dir = (dir + step + 8) % 8;
	}

	if (prevpid) waitFor(prevpid);	
}

bool CombatProcess::inAttackRange()
{
	Actor* a = World::get_instance()->getNPC(item_num);
	AnimationTracker tracker;
	if (!tracker.init(a, Animation::attack, a->getDir(), 0))
		return false;

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
	Process::saveData(ods);

	ods->write2(target);
	ods->write2(fixedTarget);
	ods->write1(static_cast<uint8>(combatmode));
}

bool CombatProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	target = ids->read2();
	fixedTarget = ids->read2();
	combatmode = static_cast<CombatMode>(ids->read1());

	return true;
}
