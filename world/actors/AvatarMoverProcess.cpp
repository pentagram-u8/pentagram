/*
Copyright (C) 2003-2005 The Pentagram team

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

#include "AvatarMoverProcess.h"
#include "Animation.h"

#include "GUIApp.h"
#include "MainActor.h"
#include "World.h"
#include "GameMapGump.h"
#include "Kernel.h"
#include "ActorAnimProcess.h"
#include "TargetedAnimProcess.h"
#include "ShapeInfo.h"

#include "IDataSource.h"
#include "ODataSource.h"

#include "SDL.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarMoverProcess,Process);

AvatarMoverProcess::AvatarMoverProcess() : Process()
{
	lastframe = 0;
	lastAttack = 0;
	idleTime = 0;
	lastHeadShakeAnim = Animation::lookLeft;
	mouseButton[0].state = MBS_HANDLED | MBS_RELHANDLED;
	mouseButton[1].state = MBS_HANDLED | MBS_RELHANDLED;
	combatRun = false;
	type = 1; // CONSTANT! (type 1 = persistent)
}


AvatarMoverProcess::~AvatarMoverProcess()
{

}

bool AvatarMoverProcess::run(const uint32 framenum)
{
	// small hack: when attacking, we set lastAttack to 0xFFFFFFFF and then
	// wait for the attack animation to finish. We then set lastAttack to
	// the time the attack finished here.
	if (lastAttack == 0xFFFFFFFF)
		lastAttack = framenum;



	GUIApp* guiapp = GUIApp::get_instance();

	// in stasis, so don't move
	if (guiapp->isAvatarInStasis()) {
		idleTime = 0;
		return false;
	}

	// only run once per frame
	if (framenum == lastframe) return false;
	lastframe = framenum;

	MainActor* avatar = World::get_instance()->getMainActor();

	if (avatar->isInCombat() && !combatRun)
		return handleCombatMode();
	else
		return handleNormalMode();
}

bool AvatarMoverProcess::handleCombatMode()
{
	GUIApp* guiapp = GUIApp::get_instance();
	MainActor* avatar = World::get_instance()->getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	Animation::Sequence nextanim = Animation::walk;
	sint32 direction = avatar->getDir();
	uint32 now = SDL_GetTicks();

	int mx, my;
	guiapp->getMouseCoords(mx, my);
	unsigned int mouselength = guiapp->getMouseLength(mx,my);

	// adjust to world direction
	sint32 mousedir = (guiapp->getMouseDirection(mx,my)+7)%8;

	// never idle when in combat
	idleTime = 0;

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		waitFor(avatar->doAnim(Animation::standUp, mousedir));
		return false;
	}

	// if we were blocking, and no longer holding the mouse, stop
	if (lastanim == Animation::startblock &&
		!(mouseButton[0].state & MBS_DOWN))
	{
		pout << "AvatarMover: combat stop blocking" << std::endl;
		waitFor(avatar->doAnim(Animation::stopblock, direction));
		return false;
	}

	bool m0clicked = false;
	bool m1clicked = false;

	if (!(mouseButton[0].state & MBS_HANDLED) &&
		now - mouseButton[0].curDown > DOUBLE_CLICK_TIMEOUT)
	{
		m0clicked = true;
		mouseButton[0].state |= MBS_HANDLED;
	}

	if (!(mouseButton[1].state & MBS_HANDLED) &&
		now - mouseButton[1].curDown > DOUBLE_CLICK_TIMEOUT)
	{
		m1clicked = true;
		mouseButton[1].state |= MBS_HANDLED;
	}

	if (!(mouseButton[0].state & MBS_RELHANDLED))
		mouseButton[0].state |= MBS_RELHANDLED;

	if (!(mouseButton[1].state & MBS_RELHANDLED))
		mouseButton[1].state |= MBS_RELHANDLED;

	if ((mouseButton[0].state & MBS_DOWN) &&
		(mouseButton[0].state & MBS_HANDLED) && mouseButton[0].lastDown > 0)
	{
		// left click-and-hold = block
		if (lastanim == Animation::startblock)
			return false;

		pout << "AvatarMover: combat block" << std::endl;

		if (checkTurn(mousedir, false)) return false;

		waitFor(avatar->doAnim(Animation::startblock, mousedir));
		return false;
	}

	if ((mouseButton[0].state & MBS_DOWN) &&
		!(mouseButton[0].state & MBS_HANDLED) &&
		mouseButton[0].curDown-mouseButton[0].lastDown <= DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[0].state |= MBS_HANDLED;
		mouseButton[0].lastDown = 0;

		if (canAttack()) {
			// double left click = attack
			pout << "AvatarMover: combat attack" << std::endl;

			if (checkTurn(mousedir, true)) return false;

			waitFor(avatar->doAnim(Animation::attack, mousedir));
			lastAttack = 0xFFFFFFFF;
		}

		return false;
	}

	if ((mouseButton[1].state & MBS_DOWN) &&
		!(mouseButton[1].state & MBS_HANDLED) &&
		mouseButton[1].curDown-mouseButton[1].lastDown <= DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[1].state |= MBS_HANDLED;
		mouseButton[1].lastDown = 0;

		Gump* desktopgump = GUIApp::get_instance()->getDesktopGump();
		if (desktopgump->TraceObjId(mx,my) == 1) {
			// double right click on avatar = toggle combat mode
			avatar->toggleInCombat();
			waitFor(avatar->doAnim(Animation::unreadyWeapon, direction));
			return false;
		}

		if (canAttack()) {
			// double right click = kick
			pout << "AvatarMover: combat kick" << std::endl;

			if (checkTurn(mousedir, false)) return false;
			
			waitFor(avatar->doAnim(Animation::kick, mousedir));
			lastAttack = 0xFFFFFFFF;
		}

		return false;
	}

	if ((mouseButton[1].state & MBS_DOWN) &&
		(mouseButton[1].state & MBS_HANDLED))
	{
		// right mouse button is down long enough to act on it
		// if facing right direction, walk
		//!! TODO: check if you can actually take this step

		if (checkTurn(mousedir, true)) return false;

		sint32 nextdir = mousedir;

		if (lastanim == Animation::run) {
			// want to run while in combat mode?
			// first sheath weapon
			nextanim = Animation::readyWeapon;
		} else if (abs(direction - mousedir) == 4) {
			nextanim = Animation::retreat;
			nextdir = direction;
		} else
			nextanim = Animation::advance;

		if (mouselength == 2)
		{
			// Take a step before running
			nextanim = Animation::walk;
			combatRun = true;
			avatar->toggleInCombat();
		}

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
		return false;
	}

	// if clicked, turn in mouse direction
	if (m0clicked || m1clicked)
		if (checkTurn(mousedir, false)) return false;

	// not doing anything in particular? stand
	// TODO: make sure falling works properly.
	if (lastanim != Animation::combat_stand) {
		nextanim = Animation::combat_stand;
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
	}

	return false;
}

bool AvatarMoverProcess::handleNormalMode()
{
	GUIApp* guiapp = GUIApp::get_instance();
	MainActor* avatar = World::get_instance()->getMainActor();
	Animation::Sequence lastanim = avatar->getLastAnim();
	Animation::Sequence nextanim = Animation::walk;
	sint32 direction = avatar->getDir();
	uint32 now = SDL_GetTicks();

	int mx, my;
	guiapp->getMouseCoords(mx, my);
	unsigned int mouselength = guiapp->getMouseLength(mx,my);

	// adjust to world direction
	sint32 mousedir = (guiapp->getMouseDirection(mx,my)+7)%8;

	// Store current idle time. (Also see end of function.)
	uint32 currentIdleTime = idleTime;
	idleTime = 0;

	// User toggled combat while in combatRun
	if (avatar->isInCombat())
	{
		combatRun = false;
		avatar->toggleInCombat();
	}

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		pout << "AvatarMover: standing up" << std::endl;
		waitFor(avatar->doAnim(Animation::standUp, direction));
		return false;
	}

	// If still in combat stance, sheathe weapon
	if (Animation::isCombatAnim(lastanim)) {
		pout << "AvatarMover: sheating weapon" << std::endl;
		waitFor(avatar->doAnim(Animation::unreadyWeapon, direction));
		return false;
	}

	bool m1clicked = false;

    // check mouse state to see what needs to be done
	if (!(mouseButton[0].state & MBS_HANDLED) &&
		now - mouseButton[0].curDown > DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[0].state |= MBS_HANDLED;
	}

	if (!(mouseButton[1].state & MBS_HANDLED) &&
		now - mouseButton[1].curDown > DOUBLE_CLICK_TIMEOUT)
	{
		m1clicked = true;
		mouseButton[1].state |= MBS_HANDLED;
	}

	// see if mouse was just released
	if (!(mouseButton[0].state & MBS_RELHANDLED))
		mouseButton[0].state |= MBS_RELHANDLED; // don't care about left

	if (!(mouseButton[1].state & MBS_RELHANDLED)) {
		mouseButton[1].state |= MBS_RELHANDLED;

		// if we were running in combat mode, slow to a walk, draw weapon
		if (combatRun)
		{
			MainActor* avatar = World::get_instance()->getMainActor();
			avatar->toggleInCombat();
			combatRun = false;
			ProcId walkpid = avatar->doAnim(Animation::walk, direction);
			ProcId drawpid = avatar->doAnim(Animation::readyWeapon, direction);
			Process* drawproc = Kernel::get_instance()->getProcess(drawpid);
			drawproc->waitFor(walkpid);
			waitFor(drawpid);
			return false;
		}

		// if we were running, slow to a walk before stopping
		if (lastanim == Animation::run) {
			ProcId walkpid = avatar->doAnim(Animation::walk, direction);
			ProcId standpid = avatar->doAnim(Animation::stand, direction);
			Process* standproc = Kernel::get_instance()->getProcess(standpid);
			standproc->waitFor(walkpid);
			waitFor(standpid);
			return false;
		}

		// TODO: if we were hanging, fall

		// otherwise, stand
		waitFor(avatar->doAnim(Animation::stand, direction));
		return false;		
	}

	// both mouse buttons down
	if (!(mouseButton[0].state & MBS_HANDLED) &&
		!(mouseButton[1].state & MBS_HANDLED))
	{
		// notice these are all unsigned.
		uint32 down = mouseButton[1].curDown;
		if (mouseButton[0].curDown < down)
		{
			down = down - mouseButton[0].curDown;
		}
		else
		{
			down = mouseButton[0].curDown - down;
		}
		
		if (down < DOUBLE_CLICK_TIMEOUT)
		{
			mouseButton[0].state |= MBS_HANDLED;
			mouseButton[1].state |= MBS_HANDLED;
			// We got a left mouse down.
			// Note that this automatically means right was down too.

			if (checkTurn(mousedir, false)) return false;

			nextanim = Animation::jumpUp;
			if (mouselength > 0) {
				nextanim = Animation::jump;
			}
			// check if there's something we can climb up onto here

			Animation::Sequence climbanim = Animation::climb72;
			while (climbanim >= Animation::climb16)
			{
				if (avatar->tryAnim(climbanim, direction)==Animation::SUCCESS)
				{
					nextanim = climbanim;
				}
				climbanim = static_cast<Animation::Sequence>(climbanim - 1);
			}

			if (nextanim == Animation::jump || nextanim == Animation::jumpUp)
			{
				jump(nextanim, direction);
				return false;
			}

			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, direction));
			return false;
		}
	}

	if (!(mouseButton[0].state & MBS_HANDLED) &&
		(mouseButton[1].state & MBS_DOWN))
	{
		mouseButton[0].state |= MBS_HANDLED;
		// We got a left mouse down.
		// Note that this automatically means right was down at the time too.

		if (checkTurn(mousedir, false)) return false;

		nextanim = Animation::jumpUp;

		// check if we need to do a running jump
		if (lastanim == Animation::run || lastanim == Animation::runningJump) {
			pout << "AvatarMover: running jump" << std::endl;
			jump(Animation::runningJump, direction);
			return false;
		} else if (mouselength > 0) {
			pout << "AvatarMover: jump" << std::endl;
			jump(Animation::jump, direction);
			return false;
		}
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, direction));
		return false;

		// TODO: climbing up onto things?
		// CHECKME: check what needs to happen when keeping left pressed
	}

	if ((mouseButton[1].state & MBS_DOWN) &&
		!(mouseButton[1].state & MBS_HANDLED) &&
		mouseButton[1].curDown-mouseButton[1].lastDown <= DOUBLE_CLICK_TIMEOUT)
	{
		Gump* desktopgump = GUIApp::get_instance()->getDesktopGump();
		if (desktopgump->TraceObjId(mx,my) == 1) {
			// double right click on avatar = toggle combat mode
			mouseButton[1].state |= MBS_HANDLED;
			mouseButton[1].lastDown = 0;

			avatar->toggleInCombat();
			waitFor(avatar->doAnim(Animation::readyWeapon, direction));
			return false;
		}
	}

	if ((mouseButton[1].state & MBS_DOWN) &&
		(mouseButton[1].state & MBS_HANDLED))
	{
		// right mouse button is down long enough to act on it
		// if facing right direction, walk
		//!! TODO: check if you can actually take this step

		nextanim = Animation::step;

		//Not perfect, but don't think anyone will ever notice or care.
		if ((avatar->tryAnim(nextanim, mousedir) == Animation::END_OFF_LAND) &&
			(lastanim != Animation::keepBalance))
		{
			nextanim = Animation::keepBalance;
		}

		if (mouselength == 1)
			nextanim = Animation::walk;

		if (mouselength == 2)
		{
			if (lastanim == Animation::run
				|| lastanim == Animation::runningJump
				|| lastanim == Animation::walk)
				nextanim = Animation::run;
			else				
				nextanim = Animation::walk;
		}

		bool moving = (nextanim == Animation::run ||
					   nextanim == Animation::walk);

		if (checkTurn(mousedir, moving)) return false;

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, mousedir));
		return false;
	}

	if (m1clicked)
		if (checkTurn(mousedir, false)) return true;

	// doing another animation?
	if (Kernel::get_instance()->getNumProcesses(1, 0x00F0))
		return false;

	// idle
	idleTime = currentIdleTime + 1;

	// currently shaking head?
	if (lastanim == Animation::lookLeft || lastanim == Animation::lookRight) {
		if ((std::rand() % 1500) + 30 < idleTime) {
			lastHeadShakeAnim = lastanim;
			waitFor(avatar->doAnim(Animation::stand, direction));
			idleTime = 0;
			return false;
		}
	} else {
		if ((std::rand() % 3000) + 150 < idleTime) {
			if (std::rand() % 5 == 0)
				nextanim = lastHeadShakeAnim;
			else
				if (lastHeadShakeAnim == Animation::lookLeft)
					nextanim = Animation::lookRight;
				else
					nextanim = Animation::lookLeft;
			waitFor(avatar->doAnim(nextanim, direction));
			idleTime = 0;
		}
	}

	return false;
}

void AvatarMoverProcess::jump(Animation::Sequence action, int direction)
{
	GUIApp* guiapp = GUIApp::get_instance();
	MainActor* avatar = World::get_instance()->getMainActor();
	int mx, my;
	guiapp->getMouseCoords(mx, my);

	// running jump
	if (action == Animation::runningJump)
	{
		waitFor(avatar->doAnim(action, direction));
		return;
	}

	// airwalk
	if ((avatar->getActorFlags() & Actor::ACT_AIRWALK) &&
		action == Animation::jump)
	{
		waitFor(avatar->doAnim(Animation::airwalkJump, direction));
		return;
	}


	//! TODO: add gameplay option: targetedJump or not

	sint32 coords[3];
	GameMapGump * gameMap = guiapp->getGameMapMapGump();
	// We need the Gump's x/y for TraceCoordinates
	gameMap->ParentToGump(mx,my);
	ObjId targetId = gameMap->TraceCoordinates(mx,my,coords);
	Item * target = World::get_instance()->getItem(targetId);
	
	if (target && target->getShapeInfo()->is_land())
	{	// Original also only lets you jump at the Z_FACE
		Process *p = new TargetedAnimProcess(avatar, Animation::jumpUp,
											 direction, coords);
		waitFor(Kernel::get_instance()->addProcess(p));
		return;
	}
	else
	{
		waitFor(avatar->doAnim(Animation::shakeHead, direction));
		return;
	}
}

void AvatarMoverProcess::turnToDirection(int direction)
{
	MainActor* avatar = World::get_instance()->getMainActor();
	int curdir = avatar->getDir();
	int step;
	bool combat = avatar->isInCombat() && !combatRun;
	Animation::Sequence turnanim;
	Animation::Sequence standanim = Animation::stand;

	if ((curdir - direction + 8) % 8 < 4) {
		step = -1;
		turnanim = Animation::lookLeft;
	} else {
		step = 1;
		turnanim = Animation::lookRight;
	}

	if (combat) {
		turnanim = Animation::combat_stand;
		standanim = Animation::combat_stand;
	}

	ProcId prevpid = 0;

	for (int dir = curdir; dir != direction; ) {
		ProcId animpid = avatar->doAnim(turnanim, dir);

		if (prevpid) {
			Process* proc = Kernel::get_instance()->getProcess(animpid);
			assert(proc);
			proc->waitFor(prevpid);
		}

		prevpid = animpid;

		dir = (dir + step + 8) % 8;
	}

	ProcId animpid = avatar->doAnim(standanim, direction);

	if (prevpid) {
		Process* proc = Kernel::get_instance()->getProcess(animpid);
		assert(proc);
		proc->waitFor(prevpid);
	}
	
	waitFor(animpid);
}

bool AvatarMoverProcess::checkTurn(int direction, bool moving)
{
	MainActor* avatar = World::get_instance()->getMainActor();
	int curdir = avatar->getDir();
	bool combat = avatar->isInCombat() && !combatRun;
	Animation::Sequence lastanim = avatar->getLastAnim();

	// Note: don't need to turn if moving backward in combat stance
	// CHECKME: currently, first turn in the right direction
	if (direction != curdir && !(
			combat && abs(direction - curdir) == 4))
	{
		if (moving &&
			(lastanim == Animation::walk || lastanim == Animation::run ||
			 lastanim == Animation::combat_stand) &&
			(abs(direction - curdir) + 1 % 8 <= 2))
		{
			// don't need to explicitly do a turn animation
			return false;
		}

		if (moving && lastanim == Animation::run) {
			// slow down to a walk first
			waitFor(avatar->doAnim(Animation::walk, curdir));
			return true;
		}

		turnToDirection(direction);
		return true;
	}

	return false;
}

bool AvatarMoverProcess::canAttack()
{
	MainActor* avatar = World::get_instance()->getMainActor();
	return (lastframe > lastAttack + (25 - avatar->getDex()));
}

void AvatarMoverProcess::OnMouseDown(int button, int mx, int my)
{
	int bid = 0;

	switch (button) {
	case BUTTON_LEFT:
	{
		bid = 0;
		break;
	}
	case BUTTON_RIGHT:
	{
		bid = 1;
		break;
	}
	default:
		CANT_HAPPEN_MSG("invalid MouseDown passed to AvatarMoverProcess");
		break;
	};

	mouseButton[bid].lastDown = mouseButton[bid].curDown;
	mouseButton[bid].curDown = SDL_GetTicks();
	mouseButton[bid].state |= MBS_DOWN;
	mouseButton[bid].state &= ~MBS_HANDLED;
}

void AvatarMoverProcess::OnMouseUp(int button)
{
	int bid = 0;

	if (button == BUTTON_LEFT) {
		bid = 0;
	} else if (button == BUTTON_RIGHT) {
		bid = 1;
	} else {
		CANT_HAPPEN_MSG("invalid MouseUp passed to AvatarMoverProcess");
	}

	mouseButton[bid].state &= ~MBS_DOWN;
	mouseButton[bid].state &= ~MBS_RELHANDLED;
}


void AvatarMoverProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write4(lastAttack);
	ods->write4(idleTime);
	ods->write2(static_cast<uint8>(lastHeadShakeAnim));
}

bool AvatarMoverProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	lastAttack = ids->read4();
	idleTime = ids->read4();
	lastHeadShakeAnim = static_cast<Animation::Sequence>(ids->read2());

	return true;
}
