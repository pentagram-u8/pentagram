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
#include "Actor.h"

#include "Kernel.h"
#include "UCMachine.h"
#include "World.h"
#include "ActorAnimProcess.h"

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(Actor,Container);

Actor::Actor()
	: animproc(0)
{

}

Actor::~Actor()
{

}


uint32 Actor::I_doAnim(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR(actor);
	ARG_UINT16(anim);
	ARG_UINT16(dir); // seems to be 1-8
	ARG_UINT16(unk1); // this is almost always 10000 in U8.Maybe speed-related?
	ARG_UINT16(unk2); // appears to be 0 or 1. Some flag?
	if (!actor) return 0;

	if (dir < 1 || dir > 8) {
		perr << "Actor::doAnim: Invalid direction (" << dir << ")" <<std::endl;
		return 0;
	}

	Process *p = new ActorAnimProcess(actor, anim, dir-1);

	return Kernel::get_instance()->addProcess(p);
}
