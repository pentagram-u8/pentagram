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

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarMoverProcess,Process);

AvatarMoverProcess::AvatarMoverProcess() : Process()
{
	lastframe = 0;
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

	// in stasis, so don't move
	if (GUIApp::get_instance()->isAvatarInStasis()) {
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

#if 0
	// NOTE: this is some testing code that 'polls' the mouse directly,
	// which is not how things should be, IMO (-wjp)

	// Now check movement input
	GUIApp* guiapp = GUIApp::get_instance();
	bool leftdown = guiapp->isMouseDown(GUIApp::BUTTON_LEFT);
	bool rightdown = guiapp->isMouseDown(GUIApp::BUTTON_RIGHT);
	int mouselength = guiapp->getMouseLength();
	int mousedir = (guiapp->getMouseDirection()+7)%8; // adjust to world dir.

	//!! TODO: combat
	//!! TODO: button-press delays

	if (rightdown && !leftdown) { // normal movement
		//!! CHECKME: currently, first turn in the right direction
		if (mousedir != direction) {
			waitFor(avatar->doAnim(standanim, mousedir));
			return false;
		}

		// if facing right direction, walk
		//!! TODO: check if you can actually take this step

		int anim = walkanim;
		if (mouselength == 0)
			anim = stepanim;
		else if (mouselength == 2)
			anim = runanim;

		waitFor(avatar->doAnim(anim, mousedir));
		return false;
	} else if (rightdown && leftdown) { // jumping
		//!! TODO: fix this... (direction, type of jump, climbing...)

		if (lastanim == runanim || lastanim == runningjumpanim) {
			waitFor(avatar->doAnim(runningjumpanim, direction));
			return false;
		}

		if (mouselength > 0) {
			waitFor(avatar->doAnim(jumpanim, direction));
			return false;
		}

		waitFor(avatar->doAnim(jumpupanim, direction));
		return false;
	}
#endif

	// not doing anything in particular? stand
	if (lastanim != standanim) {
		waitFor(avatar->doAnim(standanim, direction));
		return false;
	}

	return false;
}

void AvatarMoverProcess::OnMouseDown(int button)
{
	//TODO: mark button as down (and also store time, since we want
	// clicks to 'expire' if they're not handled in time)

	if (button == GUIApp::BUTTON_LEFT) perr << "AMP: left down" << std::endl;
	if (button == GUIApp::BUTTON_RIGHT) perr << "AMP: right down" << std::endl;

}

void AvatarMoverProcess::OnMouseUp(int button)
{
	if (button == GUIApp::BUTTON_LEFT) perr << "AMP: left up" << std::endl;
	if (button == GUIApp::BUTTON_RIGHT) perr << "AMP: right up" << std::endl;

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
