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

#include "pent_include.h"

#include "AvatarMoverProcess.h"
#include "Animation.h"

#include "GUIApp.h"
#include "MainActor.h"
#include "World.h"
#include "Kernel.h"

#include "IDataSource.h"
#include "ODataSource.h"

#include "SDL.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarMoverProcess,Process);

AvatarMoverProcess::AvatarMoverProcess() : Process()
{
	lastframe = 0;
	mouseButton[0].state = MBS_HANDLED; mouseButton[1].state = MBS_HANDLED;
	combatRun = false;
}


AvatarMoverProcess::~AvatarMoverProcess()
{

}

bool AvatarMoverProcess::run(const uint32 framenum)
{
	GUIApp* guiapp = GUIApp::get_instance();

	// in stasis, so don't move
	if (guiapp->isAvatarInStasis()) {
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
	sint32 nextdir = direction;
	uint32 now = SDL_GetTicks();

	int mx, my;
	guiapp->getMouseCoords(mx, my);
	unsigned int mouselength = guiapp->getMouseLength(mx,my);

	// adjust to world direction
	sint32 mousedir = (guiapp->getMouseDirection(mx,my)+7)%8;

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		waitFor(avatar->doAnim(Animation::standUp, nextdir));
		return false;
	}

	// if we were blocking, and no longer holding the mouse, stop
	if (lastanim == Animation::startblock &&
		!(mouseButton[0].state & MBS_DOWN))
	{
		pout << "AvatarMover: combat stop blocking" << std::endl;
		waitFor(avatar->doAnim(Animation::stopblock, nextdir));
		return false;
	}


	// handle double click timeout
	for (unsigned int i = 0; i < 2; ++i) {
		if (!(mouseButton[i].state & MBS_HANDLED) &&
			now - mouseButton[i].curDown > DOUBLE_CLICK_TIMEOUT)
		{
			mouseButton[i].state |= MBS_HANDLED;
		}
	}

    // check mouse state to see what needs to be done
	if ((mouseButton[0].state & MBS_DOWN) ||
		!(mouseButton[0].state & MBS_HANDLED) ||
		(mouseButton[1].state & MBS_DOWN) ||
		!(mouseButton[1].state & MBS_HANDLED))
	{
		// mouse button down, so maybe turn

		// Note: don't need to turn if moving backward in combat stance
		// CHECKME: currently, first turn in the right direction
		if (mousedir != direction && !(
				(abs(mousedir - direction) == 4 &&
				 Animation::isCombatAnim(lastanim) &&
				 mouselength != 2) //hack...
				))
		{
			pout << "AvatarMover: combat turn" << std::endl;
			nextdir = mousedir;

			if (lastanim == Animation::walk || lastanim == Animation::run &&
				(abs(mousedir - direction) + 1 % 8 <= 2))
			{
				// don't need to explicitly do a turn animation
			} else {
				nextanim = Animation::combat_stand;
			}

			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, nextdir));
			return false;
		}
	}

	if ((mouseButton[0].state & MBS_DOWN) &&
		(mouseButton[0].state & MBS_HANDLED))
	{
		// left click-and-hold = block
		pout << "AvatarMover: combat block" << std::endl;

		if (lastanim == Animation::startblock)
			return false;

		waitFor(avatar->doAnim(Animation::startblock, nextdir));
		return false;
	}

	if ((mouseButton[0].state & MBS_DOWN) &&
		!(mouseButton[0].state & MBS_HANDLED) &&
		mouseButton[0].curDown-mouseButton[0].lastDown <= DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[0].state |= MBS_HANDLED;

		// double left click = attack
		pout << "AvatarMover: combat attack" << std::endl;

		waitFor(avatar->doAnim(Animation::attack, nextdir));
		return false;
	}

	if ((mouseButton[1].state & MBS_DOWN) &&
		!(mouseButton[1].state & MBS_HANDLED) &&
		mouseButton[1].curDown-mouseButton[1].lastDown <= DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[1].state |= MBS_HANDLED;

		// double right click = kick
		pout << "AvatarMover: combat kick" << std::endl;

		waitFor(avatar->doAnim(Animation::kick, nextdir));
		return false;
	}

	if ((mouseButton[1].state & MBS_DOWN) &&
		(mouseButton[1].state & MBS_HANDLED))
	{
		// right mouse button is down long enough to act on it
		// if facing right direction, walk
		//!! TODO: check if you can actually take this step

		if (lastanim == Animation::run) {
			// want to run while in combat mode?
			// first sheath weapon
			nextanim = Animation::readyWeapon;
		} else if (mousedir == direction)
			nextanim = Animation::advance;
		else
			nextanim = Animation::retreat;

		if (mouselength == 2)
		{
			// Take a step before running
			nextanim = Animation::walk;
			combatRun = true;
			avatar->toggleInCombat();
		}

		pout << "AvatarMover: combat walk" << std::endl;

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
		return false;
	}


	// not doing anything in particular? stand
	// TODO: make sure falling works properly.
	if (lastanim != Animation::combat_stand) {
		nextanim = Animation::combat_stand;
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
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
	sint32 nextdir = direction;

	int mx, my;
	guiapp->getMouseCoords(mx, my);
	unsigned int mouselength = guiapp->getMouseLength(mx,my);

	// adjust to world direction
	sint32 mousedir = (guiapp->getMouseDirection(mx,my)+7)%8;

	// User toggled combat while in combatRun
	if (avatar->isInCombat())
	{
		combatRun = false;
		avatar->toggleInCombat();
	}

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::die || lastanim == Animation::fallBackwards) {
		waitFor(avatar->doAnim(Animation::standUp, nextdir));
		return false;
	}

    // check mouse state to see what needs to be done
	if (!(mouseButton[0].state & MBS_HANDLED) &&
		SDL_GetTicks() - mouseButton[0].curDown > DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[0].state |= MBS_HANDLED;
	}

	if (!(mouseButton[1].state & MBS_HANDLED) &&
		SDL_GetTicks() - mouseButton[1].curDown > DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[1].state |= MBS_HANDLED;
	}

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
			// Note that this automatically means right was down at the time too.

			nextanim = Animation::jumpUp;
			if (mouselength > 0) {
				nextanim = Animation::jump;
			}
			// check if there's something we can climb up onto here

			Animation::Sequence climbanim = Animation::climb72;
			while (climbanim >= Animation::climb16)
			{
				if(avatar->tryAnim(climbanim, nextdir) == Animation::SUCCESS)
				{
					nextanim = climbanim;
				}
				climbanim = (Animation::Sequence) (climbanim - 1);
			}

			// We must be facing the correct direction
			if (mousedir != nextdir)
			{
				nextanim = Animation::stand;
			}

			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, nextdir));
			return false;
		}
	}

	if ((mouseButton[1].state & MBS_DOWN) ||
		!(mouseButton[1].state & MBS_HANDLED))
	{
		// right mouse button down, but maybe not long enough to really
		// start walking. Check if we need to turn.
		// Note: don't need to turn if moving backward in combat stance
		// CHECKME: currently, first turn in the right direction
		if (mousedir != direction && !(
				(abs(mousedir - direction) == 4 &&
				 Animation::isCombatAnim(lastanim) &&
				 mouselength != 2) //hack...
				))
		{
			pout << "AvatarMover: turn" << std::endl;
			nextdir = mousedir;

			if (lastanim == Animation::walk || lastanim == Animation::run &&
				(abs(mousedir - direction) + 1 % 8 <= 2))
			{
				// don't need to explicitly do a turn animation
			} else {
				turnToDirection(nextdir);
				return false;
//				nextanim = Animation::stand;
			}
			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, nextdir));
			return false;
		}
	}

	if (!(mouseButton[0].state & MBS_HANDLED) &&
		(mouseButton[1].state & MBS_DOWN))
	{
		mouseButton[0].state |= MBS_HANDLED;
		// We got a left mouse down.
		// Note that this automatically means right was down at the time too.

		nextanim = Animation::jumpUp;

		// check if we need to do a running jump
		if (lastanim == Animation::run || lastanim == Animation::runningJump) {
			pout << "AvatarMover: running jump" << std::endl;
			nextanim = Animation::runningJump;
		} else if (mouselength > 0) {
			pout << "AvatarMover: jump" << std::endl;
			nextanim = Animation::jump;
		}
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
		return false;

		// TODO: post-patch targeted jumping
		// TODO: airwalk
		// TODO: climbing up onto things
		// CHECKME: check what needs to happen when keeping left pressed
	}

	if ((mouseButton[1].state & MBS_DOWN) &&
		(mouseButton[1].state & MBS_HANDLED))
	{
		// right mouse button is down long enough to act on it
		// if facing right direction, walk
		//!! TODO: check if you can actually take this step

		nextanim = Animation::step;

		//Not perfect, but don't think anyone will ever notice or care.
		if ((avatar->tryAnim(nextanim, nextdir) == Animation::END_OFF_LAND) &&
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

		pout << "AvatarMover: walk" << std::endl;

		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
		return false;
	}

	// not doing anything in particular? stand
	// TODO: make sure falling works properly.
	if (lastanim != Animation::stand) {
		nextanim = Animation::stand;
		nextanim = Animation::checkWeapon(nextanim, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
	}

	return false;
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
		if (combatRun)
		{
			MainActor* avatar = World::get_instance()->getMainActor();
			avatar->toggleInCombat();
			combatRun = false;
		}
	} else {
		CANT_HAPPEN_MSG("invalid MouseUp passed to AvatarMoverProcess");
	}

	mouseButton[bid].state &= ~MBS_DOWN;
}


void AvatarMoverProcess::saveData(ODataSource* ods)
{
	ods->write2(1); // version
	Process::saveData(ods);
}

bool AvatarMoverProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	return true;
}
