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

#include "EggHatcherProcess.h"
#include "Egg.h"
#include "MainActor.h"
#include "World.h"
#include "TeleportEgg.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(EggHatcherProcess,Process);

EggHatcherProcess::EggHatcherProcess()
{

}


EggHatcherProcess::~EggHatcherProcess()
{

}

void EggHatcherProcess::addEgg(uint16 egg)
{
	eggs.push_back(egg);
}

void EggHatcherProcess::addEgg(Egg* egg)
{
	assert(egg);
	eggs.push_back(egg->getObjId());
}

bool EggHatcherProcess::run(const uint32 framenum)
{
	bool nearteleporter = false;
	MainActor*av =p_dynamic_cast<MainActor*>(World::get_instance()->getNPC(1));
	assert(av);

	for (unsigned int i = 0; i < eggs.size(); i++) {
		uint16 eggid = eggs[i];
		Egg* egg=p_dynamic_cast<Egg*>(World::get_instance()->getObject(eggid));
		if (!egg) continue; // egg gone

		sint32 x,y,z;
		egg->getLocation(x,y,z);

		//! constants (is 8 the right scale?)
		sint32 x1 = x - 8 * egg->getXRange();
		sint32 x2 = x + 8 * egg->getXRange();
		sint32 y1 = y - 8 * egg->getYRange();
		sint32 y2 = y + 8 * egg->getYRange();

		// get avatar location
		sint32 ax,ay,az;
		av->getLocation(ax,ay,az);


		// 'justTeleported':
		// if the avatar teleports, set the 'justTeleported' flag.
		// if this is set, don't hatch any teleport eggs
		// unset it when you're out of range of any teleport eggs
		TeleportEgg* tegg = p_dynamic_cast<TeleportEgg*>(egg);

		// note that this prevents eggs with XRange==0||YRange==0 from hatching
		if (x1 <= ax && ax < x2 && y1 <= ay && ay < y2) {
			if (tegg && tegg->isTeleporter()) nearteleporter = true;

			if (tegg && av->hasJustTeleported()) continue;

			egg->hatch();
		}
	}

	if (!nearteleporter) av->setJustTeleported(false); // clear flag

	return false;
}
