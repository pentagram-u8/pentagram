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

#include "IDataSource.h"
#include "ODataSource.h"

#include "SDL.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarMoverProcess,Process);

AvatarMoverProcess::AvatarMoverProcess() : Process()
{
	lastframe = 0;
	mouseButton[0].state = MBS_HANDLED; mouseButton[1].state = MBS_HANDLED;
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

	if (avatar->isInCombat())
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
	uint32 direction = avatar->getDir();
	uint32 nextdir = direction;
	uint32 now = SDL_GetTicks();

	int mx, my;
	guiapp->getMouseCoords(mx, my);
	unsigned int mouselength = guiapp->getMouseLength(mx,my);

	// adjust to world direction
	uint32 mousedir = (guiapp->getMouseDirection(mx,my)+7)%8;

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::fall || lastanim == Animation::fallBackwards) {
		nextanim = Animation::checkWeapon(Animation::standUp, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
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
			nextanim = Animation::run;

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
	uint32 direction = avatar->getDir();
	uint32 nextdir = direction;

	int mx, my;
	guiapp->getMouseCoords(mx, my);
	unsigned int mouselength = guiapp->getMouseLength(mx,my);

	// adjust to world direction
	uint32 mousedir = (guiapp->getMouseDirection(mx,my)+7)%8;

	// If Avatar has fallen down, stand up.
	if (lastanim == Animation::fall || lastanim == Animation::fallBackwards) {
		nextanim = Animation::checkWeapon(Animation::standUp, lastanim);
		waitFor(avatar->doAnim(nextanim, nextdir));
		return false;
	}

    // check mouse state to see what needs to be done
	if (!(mouseButton[1].state & MBS_HANDLED) &&
		SDL_GetTicks() - mouseButton[1].lastDown > DOUBLE_CLICK_TIMEOUT)
	{
		mouseButton[1].state |= MBS_HANDLED;
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
			} else if (avatar->isInCombat())
			{
				nextanim = Animation::combat_stand;
			}
			else
			{
				nextanim = Animation::stand;
			}
			nextanim = Animation::checkWeapon(nextanim, lastanim);
			waitFor(avatar->doAnim(nextanim, nextdir));
			return false;
		}
	}

	if ((mouseButton[0].state & MBS_DOWN) ||
		!(mouseButton[0].state & MBS_HANDLED))
	{
		mouseButton[0].state |= MBS_HANDLED;
		mouseButton[1].state |= MBS_HANDLED;
		// We got a left mouse down.
		// Note that this automatically means right was down at the time too.

		nextanim = Animation::jumpUp;

		// jump.

		// check if we need to do a running jump
		if (lastanim == Animation::run || lastanim == Animation::runningJump) {
			pout << "AvatarMover: running jump" << std::endl;
			nextanim = Animation::runningJump;
		} else if (mouselength > 0) {
			// jumping straight up.
			// check if there's something we can climb up onto here
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
		if (mouselength == 1)
			nextanim = Animation::walk;

#if 0
		if (avatar->isInCombat()) {
			if (lastanim == Animation::run) {
				// want to while in combat mode?
				// first sheath weapon
				nextanim = Animation::unreadyWeapon;
			} else if (mousedir == direction)
				nextanim = Animation::advance;
			else
				nextanim = Animation::retreat;
		}
#endif

		if (mouselength == 2)
			nextanim = Animation::run;

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

void AvatarMoverProcess::OnMouseDown(int button, int mx, int my)
{
	//TODO: mark button as down (and also store time, since we want
	// clicks to 'expire' if they're not handled in time)

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
