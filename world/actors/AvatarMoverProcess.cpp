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
	mouseState[0] = MBS_HANDLED; mouseState[1] = MBS_HANDLED;
}


AvatarMoverProcess::~AvatarMoverProcess()
{

}

bool AvatarMoverProcess::run(const uint32 framenum)
{
	const unsigned int fallanim = 44; //!! constants (move)
	const unsigned int fallanim2 = 14;
	const unsigned int standupanim = 4;
	const unsigned int walkanim = 0;
	const unsigned int runanim = 1;
	const unsigned int stepanim = 12;
	const unsigned int standanim = 2;
	const unsigned int jumpanim = 17;
	const unsigned int jumpupanim = 3;
	const unsigned int runningjumpanim = 10;

	GUIApp* guiapp = GUIApp::get_instance();

	// in stasis, so don't move
	if (guiapp->isAvatarInStasis()) {
		return false;
	}

	// only run once per frame
	if (framenum == lastframe) return false;
	lastframe = framenum;

	// We're here because the Avatar isn't doing anything right now,
	// which needs to be remedied :-)

	MainActor* avatar = World::get_instance()->getMainActor();
	uint32 lastanim = avatar->getLastAnim();
	uint32 direction = avatar->getDir();

	// If Avatar fell down, stand up.
	if (lastanim == fallanim || lastanim == fallanim2) {
		waitFor(avatar->doAnim(standupanim, direction));
		return false;
	}

	int mx, my;
	guiapp->getMouseCoords(mx, my);
	unsigned int mouselength = guiapp->getMouseLength(mx,my);

	// adjust to world direction
	unsigned int mousedir = (guiapp->getMouseDirection(mx,my)+7)%8;

	// check mouse state to see what needs to be done

	if (!(mouseState[1] & MBS_HANDLED) &&
		SDL_GetTicks() - lastMouseDown[1] > 200) //!! constant
	{
		mouseState[1] |= MBS_HANDLED;
	}

	if (mouseState[1] & MBS_DOWN || !(mouseState[1] & MBS_HANDLED))
	{
		// right mouse button down, but maybe not long enough to really
		// start walking. Check if we need to turn.
		// CHECKME: currently, first turn in the right direction
		if (mousedir != direction) {
			pout << "AvatarMover: turn" << std::endl;
			waitFor(avatar->doAnim(standanim, mousedir));
			return false;
		}

	}

	if (mouseState[0] & MBS_DOWN || !(mouseState[0] & MBS_HANDLED))
	{
		mouseState[0] |= MBS_HANDLED;
		mouseState[1] |= MBS_HANDLED;
		// We got a left mouse down.
		// Note that this automatically means right was down at the time too.

		// jump.

		// check if we need to do a running jump
		if (lastanim == runanim || lastanim == runningjumpanim) {
			pout << "AvatarMover: running jump" << std::endl;
			waitFor(avatar->doAnim(runningjumpanim, direction));
			return false;
		}

		if (mouselength > 0) {
			pout << "AvatarMover: jump" << std::endl;
			waitFor(avatar->doAnim(jumpanim, direction));
			return false;
		}

		// jumping straight up.
		// check if there's something we can climb up onto here

		waitFor(avatar->doAnim(jumpupanim, direction));
		return false;

		// TODO: post-patch targeted jumping
		// TODO: airwalk
		// TODO: climbing up onto things
		// CHECKME: check what needs to happen when keeping left pressed
	}

	if (mouseState[1] & MBS_DOWN && mouseState[1] & MBS_HANDLED)
	{
		// right mouse button is down long enough to act on it

		// if facing right direction, walk
		//!! TODO: check if you can actually take this step

		int anim = walkanim;
		if (mouselength == 0)
			anim = stepanim;
		else if (mouselength == 2)
			anim = runanim;

		pout << "AvatarMover: walk" << std::endl;

		waitFor(avatar->doAnim(anim, mousedir));
		return false;
	}

	// not doing anything in particular? stand
	// TODO: make sure falling works properly.
	if (lastanim != standanim) {
		waitFor(avatar->doAnim(standanim, direction));
		return false;
	}

	return false;
}

void AvatarMoverProcess::OnMouseDown(int button, int mx, int my)
{
	//TODO: mark button as down (and also store time, since we want
	// clicks to 'expire' if they're not handled in time)

	int bid = 0;

	if (button == GUIApp::BUTTON_LEFT) {
		bid = 0;
	} else if (button == GUIApp::BUTTON_RIGHT) {
		bid = 1;
	} else {
		CANT_HAPPEN_MSG("invalid MouseDown passed to AvatarMoverProcess");
	}

	lastMouseDown[bid] = SDL_GetTicks();
	mouseState[bid] |= MBS_DOWN;
	mouseState[bid] &= ~MBS_HANDLED;
}

void AvatarMoverProcess::OnMouseUp(int button)
{
	int bid = 0;

	if (button == GUIApp::BUTTON_LEFT) {
		bid = 0;
	} else if (button == GUIApp::BUTTON_RIGHT) {
		bid = 1;
	} else {
		CANT_HAPPEN_MSG("invalid MouseUp passed to AvatarMoverProcess");
	}

	mouseState[bid] &= ~MBS_DOWN;
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
