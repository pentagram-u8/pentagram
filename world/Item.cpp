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

#include "Item.h"
#include "GUIApp.h"
#include "Usecode.h"
#include "GameData.h"
#include "UCMachine.h"
#include "UCList.h"
#include "World.h"
#include "DelayProcess.h"
#include "Container.h"
#include "Kernel.h"
#include "ObjectManager.h"

#include "MainShapeFlex.h"
#include "GumpShapeFlex.h"
#include "Shape.h"
#include "ShapeInfo.h"
#include "ItemFactory.h"
#include "CurrentMap.h"
#include "UCStack.h"
#include "Direction.h"
#include "MissileProcess.h"
#include "BarkGump.h"
#include "AskGump.h"
#include "GumpNotifyProcess.h"
#include "ContainerGump.h"
#include "GameMapGump.h"
#include "WorldPoint.h"
#include "GravityProcess.h"
#include "LoopScript.h"
#include "IDataSource.h"
#include "ODataSource.h"

#include <cstdlib>

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Item,Object);

Item::Item()
	: shape(0), frame(0), x(0), y(0), z(0),
	  flags(0), quality(0), npcnum(0), mapnum(0),
	  extendedflags(0), parent(0), 
	  cachedShape(0), cachedShapeInfo(0), gump(0), gravitypid(0),
	  glob_next(0),last_setup(0)
{

}


Item::~Item()
{

}

void Item::setLocation(sint32 X, sint32 Y, sint32 Z)
{
	x = X;
	y = Y;
	z = Z;
}

void Item::move(sint32 X, sint32 Y, sint32 Z, bool forcemapupdate)
{
	//constant
	if (!(flags & (FLG_CONTAINED | FLG_EQUIPPED | FLG_ETHEREAL))
		&& (forcemapupdate || (x / 512 != X / 512) || (y / 512 != Y / 512))) {

		// if item isn't contained or equipped, it's in CurrentMap

		World::get_instance()->getCurrentMap()->removeItemFromList(this,x,y);
		setLocation(X, Y, Z);
		World::get_instance()->getCurrentMap()->addItem(this);
	}
	else
	{
    	setLocation(X, Y, Z);
	}

	// Call just moved
	callUsecodeEvent_justMoved();
}

void Item::getLocation(sint32& X, sint32& Y, sint32& Z) const
{
	X = x;
	Y = y;
	Z = z;
}

sint32 Item::getZ() const
{
	return z;
}

void Item::getLocationAbsolute(sint32& X, sint32& Y, sint32& Z) const
{
	if (parent) 
		parent->getLocationAbsolute(X,Y,Z);
	else
	{
		X = x;
		Y = y;
		Z = z;
	}
}

void Item::getGumpLocation(sint32& X, sint32& Y) const
{
	if (!parent) return;

	X = y & 0xFF;
	Y = (y >> 8) & 0xFF;
}

void Item::setGumpLocation(sint32 X, sint32 Y)
{
	if (!parent) return;

	y = (X & 0xFF) + ((Y & 0xFF) << 8);
}

void Item::getCentre(sint32& X, sint32& Y, sint32& Z) const
{
	ShapeInfo *shapeinfo = getShapeInfoNoCache();
	if (flags & FLG_FLIPPED)
	{
		X = x - shapeinfo->y * 16;
		Y = y - shapeinfo->x * 16;
	}
	else
	{
		X = x - shapeinfo->x * 16;
		Y = y - shapeinfo->y * 16;
	}

	Z = z + shapeinfo->z * 4;
}

// note that this is in different units than location
void Item::getFootpad(sint32& X, sint32& Y, sint32& Z) const
{
	Z = getShapeInfoNoCache()->z;

	if (getFlags() & Item::FLG_FLIPPED) {
		X = getShapeInfoNoCache()->y;
		Y = getShapeInfoNoCache()->x;
	} else {
		X = getShapeInfoNoCache()->x;
		Y = getShapeInfoNoCache()->y;
	}
	
}

bool Item::overlaps(Item& item2) const
{
	sint32 x1a,y1a,z1a,x1b,y1b,z1b;
	sint32 x2a,y2a,z2a,x2b,y2b,z2b;
	getLocation(x1b,y1b,z1a);
	item2.getLocation(x2b,y2b,z2a);

	sint32 xd,yd,zd;
	getFootpad(xd,yd,zd);
	x1a = x1b - 32 * xd;
	y1a = y1b - 32 * yd;
	z1b = z1a + 8 * zd;

	item2.getFootpad(xd,yd,zd);
	x2a = x2b - 32 * xd;
	y2a = y2b - 32 * yd;
	z2b = z2a + 8 * zd;

	if (x1b <= x2a || x2b <= x1a) return false;
	if (y1b <= y2a || y2b <= y1a) return false;
	if (z1b <= z2a || z2b <= z1a) return false;
	return true;
}

bool Item::overlapsxy(Item& item2) const
{
	sint32 x1a,y1a,z1a,x1b,y1b;
	sint32 x2a,y2a,z2a,x2b,y2b;
	getLocation(x1b,y1b,z1a);
	item2.getLocation(x2b,y2b,z2a);

	sint32 xd,yd,zd;
	getFootpad(xd,yd,zd);
	x1a = x1b - 32 * xd;
	y1a = y1b - 32 * yd;

	item2.getFootpad(xd,yd,zd);
	x2a = x2b - 32 * xd;
	y2a = y2b - 32 * yd;

	if (x1b <= x2a || x2b <= x1a) return false;
	if (y1b <= y2a || y2b <= y1a) return false;
	return true;
}

bool Item::isOn(Item& item2) const
{
	sint32 x1a,y1a,z1a,x1b,y1b;
	sint32 x2a,y2a,z2a,x2b,y2b,z2b;
	getLocation(x1b,y1b,z1a);
	item2.getLocation(x2b,y2b,z2a);

	sint32 xd,yd,zd;
	getFootpad(xd,yd,zd);
	x1a = x1b - 32 * xd;
	y1a = y1b - 32 * yd;

	item2.getFootpad(xd,yd,zd);
	x2a = x2b - 32 * xd;
	y2a = y2b - 32 * yd;
	z2b = z2a + 8 * zd;

	if (x1b <= x2a || x2b <= x1a) return false;
	if (y1b <= y2a || y2b <= y1a) return false;
	if (z2b == z1a) return true;
	return false;
}

bool Item::canExistAt(sint32 x, sint32 y, sint32 z) const
{
	sint32 xd, yd, zd;
	getFootpad(xd, yd, zd);
	xd *= 32; yd *= 32; zd *= 8; //!! constants
	CurrentMap* cm = World::get_instance()->getCurrentMap();
#if 0
	if (getShape() == 264) { // spiky sphere
		perr << "Spiky Sphere: Trying (" << x-xd << "," << y-yd << "," << z 
			 << ")-(" << x << "," << y << "," << z+zd << ")" << std::endl;
	}
#endif
	return cm->isValidPosition(x, y, z, xd, yd, zd, getObjId(), 0, 0);
}


ShapeInfo* Item::getShapeInfo()
{
	if (!cachedShapeInfo) cachedShapeInfo = GameData::get_instance()->getMainShapes()->getShapeInfo(shape);
	return cachedShapeInfo;
}

ShapeInfo* Item::getShapeInfoNoCache() const
{
	if (cachedShapeInfo) return cachedShapeInfo;
	return GameData::get_instance()->getMainShapes()->getShapeInfo(shape);
}

Shape* Item::getShapeObject()
{
	if (!cachedShape) cachedShape = GameData::get_instance()->getMainShapes()->getShape(shape);
	return cachedShape;
}

Shape* Item::getShapeObjectNoCache() const
{
	if (cachedShape) return cachedShape;
	return GameData::get_instance()->getMainShapes()->getShape(shape);
}

uint16 Item::getFamily()
{
	return static_cast<uint16>(getShapeInfo()->family);
}

uint32 Item::getTotalWeight()
{
	return getShapeInfo()->weight;
}

bool Item::checkLoopScript(const uint8* script, uint32 scriptsize)
{
	// if really necessary this could be made static to prevent news/deletes
	UCStack stack(0x40); // 64bytes should be plenty of room

	unsigned int i = 0;

	uint16 ui16a, ui16b;

	stack.push2(1); // default to true if script is empty

	while (i < scriptsize) {
		switch(script[i]) {
		case LS_TOKEN_FALSE: // false
			stack.push2(0); break;

		case LS_TOKEN_TRUE: // true
			stack.push2(1); break;

		case LS_TOKEN_END: // end
			ui16a = stack.pop2();
			return (ui16a != 0);

		case LS_TOKEN_INT: // int
			//! check for i out of bounds
			ui16a = script[i+1] + (script[i+2]<<8);
			stack.push2(ui16a);
			i += 2;
			break;

		case LS_TOKEN_AND: // and
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a != 0 && ui16b != 0)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_OR: // or
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a != 0 || ui16b != 0)
				stack.push2(1);
			else
				stack.push2(0);
			break;			

		case LS_TOKEN_NOT: // not
			ui16a = stack.pop2();
			if (ui16a != 0)
				stack.push2(0);
			else
				stack.push2(1);
			break;

		case LS_TOKEN_STATUS: // item status
			stack.push2(getFlags());
			break;

		case LS_TOKEN_Q: // item quality
			stack.push2(getQuality());
			break;

		case LS_TOKEN_NPCNUM: // npc num
			stack.push2(getNpcNum());
			break;

		case LS_TOKEN_EQUAL: // equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16a == ui16b)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_GREATER: // greater than
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b > ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_LESS: // less than
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b < ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_GEQUAL: // greater or equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b >= ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_LEQUAL: // smaller or equal
			ui16a = stack.pop2();
			ui16b = stack.pop2();
			if (ui16b <= ui16a)
				stack.push2(1);
			else
				stack.push2(0);
			break;

		case LS_TOKEN_FAMILY: // item family
			stack.push2(getFamily());
			break;

		case LS_TOKEN_SHAPE: // item shape
			stack.push2(static_cast<uint16>(getShape()));
			break;

		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		{
			bool match = false;
			int count = script[i] - '@';
			for (int j = 0; j < count; j++) {
				//! check for i out of bounds
				if (getShape() == static_cast<uint32>(script[i+1] + (script[i+2]<<8)))
					match = true;
				i += 2;
			}
			if (match)
				stack.push2(1);
			else
				stack.push2(0);
		}
		break;

		case LS_TOKEN_FRAME: // item frame
			stack.push2(static_cast<uint16>(getFrame()));
			break;

		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		{
			bool match = false;
			int count = script[i] - '`';
			for (int j = 0; j < count; j++) {
				//! check for i out of bounds
				if (getFrame() == static_cast<uint32>(script[i+1] + (script[i+2]<<8)))
					match = true;
				i += 2;
			}
			if (match)
				stack.push2(1);
			else
				stack.push2(0);
		}
		break;

		default:
			perr.printf("Unknown loopscript opcode %02X\n", script[i]);
		}

		i++;
	}
	perr.printf("Didn't encounter $ in loopscript\n");
	return false;
}


sint32 Item::collideMove(sint32 dx, sint32 dy, sint32 dz, bool teleport, bool force)
{
	extern bool hitting();
	World *world = World::get_instance();
	CurrentMap *map = world->getCurrentMap();

	sint32 end[3] = { dx, dy, dz };

	sint32 start[3];
	if (parent)
	{
		// If we are moving from a container, only check the destination
		start[0] = end[0];
		start[1] = end[1];
		start[2] = end[2];
	}
	else
	{
		// Otherwise check from where we are to where we want to go
		getLocation(start[0], start[1], start[2]);
	}

	sint32 dims[3];
	getFootpad(dims[0], dims[1], dims[2]);
	dims[0] *= 32; dims[1] *= 32; dims[2] *= 8;

	// Do the sweep test
	std::list<CurrentMap::SweepItem> collisions;
	std::list<CurrentMap::SweepItem>::iterator it;
	map->sweepTest(start, end, dims, objid, false, &collisions);

	// Ok, now to work out what to do

	// if we are contained, we always teleport
	if (teleport || parent)
	{
		// If teleporting and not force, work out if we can reach the end
		if (!force) 
		{
			for (it = collisions.begin(); it != collisions.end(); it++)
			{
				// Wasn't hitting us at the end or were touching
				if (it->end_time != 0x4000 || it->touching)  
					continue;

				Item *item = world->getItem(it->item);
				if (!item) continue;
				ShapeInfo *info = item->getShapeInfo();

				// Uh oh, we hit something, can't move
				if (info->is_solid()) return 0;

			}
		}

		// Trigger all the required events
		bool we_were_released = false;
		for (it = collisions.begin(); it != collisions.end(); it++)
		{
			Item *item = world->getItem(it->item);

			// Hitting us at the start and end, don't do anything
			if (!parent && it->hit_time == 0x0000 && 
					it->end_time == 0x4000)
			{
				continue;
			}
			// Hitting us at the end (call hit on us, got hit on them)
			else if (it->end_time == 0x4000)
			{
				if (objid == 1 && hitting()) item->setExtFlag(Item::EXT_HIGHLIGHT);
				item->callUsecodeEvent_gotHit(objid,0);
				callUsecodeEvent_hit(item->getObjId(),0);
			}
			// Hitting us at the start (call release on us and them)
			else if (!parent && it->hit_time == 0x0000)
			{
				if (objid == 1) item->clearExtFlag(Item::EXT_HIGHLIGHT);
				we_were_released = true;
				item->callUsecodeEvent_release();
			}
		}

		// Call release on us
		if (we_were_released) callUsecodeEvent_release();

		// Remove us from our parent
		if (parent) parent->RemoveItem(this);

		// Move US!
		move(end[0], end[1], end[2]);

		// We reached the end
		return 0x4000;
	}
	else
	{
		sint32 hit = 0x4000;

		// If not force, work out if we can reach the end
		// if not, need to do 'stuff'
		// We don't care about items hitting us at the start
		if (!force) 
		{
			for (it = collisions.begin(); it != collisions.end(); it++)
			{
				// Was only touching
				if (it->touching) continue;

				Item *item = world->getItem(it->item);
				if (!item) continue;
				ShapeInfo *info = item->getShapeInfo();

				// Uh oh, we hit something
				if (info->is_solid())
				{
					hit = it->hit_time;
					break;
				}
			}

			if (hit != 0x4000)
			{
				// Resweep
				//pout << " Hit time: " << hit << std::endl;
				//pout << "    Start: " << start[0] << ", "<< start[1] << ", " << start[2] << std::endl;
				//pout << "      End: " << end[0] << ", "<< end[1] << ", " << end[2] << std::endl;
				it->GetInterpolatedCoords(end,start,end);
				//pout << "Collision: " << end[0] << ", "<< end[1] << ", " << end[2] << std::endl;
				collisions.clear();
				map->sweepTest(start, end, dims, objid, false, &collisions);
			}
		}


		// Trigger all the required events
		bool we_were_released = false;
		for (it = collisions.begin(); it != collisions.end(); it++)
		{
			Item *item = world->getItem(it->item);
			if (!item) continue;

			uint16 proc_gothit = 0, proc_rel = 0;

			// If hitting at start, we should have already 
			// called gotHit and hit
			if (it->hit_time > 0) 
			{
				if (objid == 1 && hitting()) item->setExtFlag(Item::EXT_HIGHLIGHT);
				proc_gothit = item->callUsecodeEvent_gotHit(objid,0);
				callUsecodeEvent_hit(item->getObjId(), 0);
			}

			// If not hitting at end, we will need to call release
			if (it->end_time < 0x4000)
			{
				if (objid == 1) item->clearExtFlag(Item::EXT_HIGHLIGHT);
				we_were_released = true;
				proc_rel = item->callUsecodeEvent_release();
			}

			// We want to make sure that release is called AFTER gotHit.
			if (proc_rel && proc_gothit)
			{
				Process *p = Kernel::get_instance()->getProcess(proc_rel);
				p->waitFor(proc_gothit);
			}
		}

		// Call release on us
		if (we_were_released) callUsecodeEvent_release();

		// Move US!
		move(end[0], end[1], end[2]);

		return hit;
	}

	return 0;
}

uint32 Item::callUsecodeEvent(uint32 event, const uint8* args, int argsize)
{
	uint32	class_id = shape;

	// Non monster NPCs use objid/npcnum + 1024
	if (objid < 256 && !(flags & FLG_MONSTER_NPC)) class_id = objid + 1024;

	// UnkEggs have quality+0x47F
	if (getFamily() == ShapeInfo::SF_UNKEGG) class_id = quality + 0x47F;

	Usecode* u = GameData::get_instance()->getMainUsecode();
	uint32 offset = u->get_class_event(class_id, event);
	if (!offset) return 0;

	//pout << "Item: " << objid << " calling usecode event " << event << " @ " << class_id << ":" << offset << std::endl;

	return callUsecode(static_cast<uint16>(class_id), 
						static_cast<uint16>(offset),
						u, args, argsize);
}

uint32 Item::callUsecodeEvent_look()							// event 0
{
	return callUsecodeEvent(0);	// CONSTANT
}

uint32 Item::callUsecodeEvent_use()								// event 1
{
	return callUsecodeEvent(1);	// CONSTANT
}

uint32 Item::callUsecodeEvent_anim()							// event 2
{
	return callUsecodeEvent(2);	// CONSTANT
}

uint32 Item::callUsecodeEvent_cachein()							// event 4
{
	return callUsecodeEvent(4);	// CONSTANT
}

uint32 Item::callUsecodeEvent_hit(uint16 hitter, sint16 unk)	// event 5
{
	UCStack	arg_stack(4);
	arg_stack.push2(unk);
	arg_stack.push2(hitter);
	return callUsecodeEvent(5, arg_stack.access(), 4);	// CONSTANT 5
}

uint32 Item::callUsecodeEvent_gotHit(uint16 hitter, sint16 unk)	// event 6
{
	UCStack	arg_stack(4);
	arg_stack.push2(unk);
	arg_stack.push2(hitter);
	return callUsecodeEvent(6, arg_stack.access(), 4);	// CONSTANT 6
}

uint32 Item::callUsecodeEvent_hatch()							// event 7
{
	return callUsecodeEvent(7);		// CONSTANT
}

uint32 Item::callUsecodeEvent_schedule()						// event 8
{
	return callUsecodeEvent(8);		// CONSTANT
}

uint32 Item::callUsecodeEvent_release()							// event 9
{
	return callUsecodeEvent(9);		// CONSTANT
}

uint32 Item::callUsecodeEvent_combine()							// event C
{
	return callUsecodeEvent(0xC);	// CONSTANT
}

uint32 Item::callUsecodeEvent_enterFastArea()					// event F
{
	return callUsecodeEvent(0xF);	// CONSTANT
}

uint32 Item::callUsecodeEvent_leaveFastArea()					// event 10
{
	return callUsecodeEvent(0x10);	// CONSTANT
}

uint32 Item::callUsecodeEvent_cast(uint16 unk)					// event 11
{
	UCStack	arg_stack(2);
	arg_stack.push2(unk);
	return callUsecodeEvent(0x11, arg_stack.access(), 2); // CONSTANT 0x11
}

uint32 Item::callUsecodeEvent_justMoved()						// event 12
{
	return callUsecodeEvent(0x12);	// CONSTANT
}

uint32 Item::callUsecodeEvent_AvatarStoleSomething(uint16 unk)	// event 14
{
	UCStack	arg_stack(2);
	arg_stack.push2(unk);
	return callUsecodeEvent(0x14, arg_stack.access(), 2); // CONSTANT 0x14
}

uint32 Item::callUsecodeEvent_guardianBark(sint16 unk)			// event 15
{
	UCStack	arg_stack(2);
	arg_stack.push2(unk);
	return callUsecodeEvent(0x15, arg_stack.access(), 2); // CONSTANT 0x15
}

void Item::destroy()
{
	if (extendedflags & EXT_INGLOB) return; // don't touch glob contents

	if (parent) {		
		// we're in a container, so remove self from parent
		//!! need to make sure this works for equipped items too...
		parent->RemoveItem(this);
	} else if (!(flags & FLG_ETHEREAL)) {
		// remove self from CurrentMap
		World::get_instance()->getCurrentMap()->removeItemFromList(this,x,y);
	}

	clearObjId();
	delete this; // delete self.
}

//
// Item::setupLerp()
//
// Desc: Setup the lerped info for this frame
//
void Item::setupLerp(bool post)
{
	// Setup prev values, but only if fast
	if (!post) l_prev = l_next;

	l_next.x = ix = x;
	l_next.y = iy = y;
	l_next.z = iz = z;
	l_next.shape = shape;
	l_next.frame = frame;

	// Setup prev values, if not fast
	if (post) l_prev = l_next;
}

// Animate the item
void Item::animateItem()
{
	ShapeInfo *info = getShapeInfo();
	Shape *shp = getShapeObject();

	if (!info->animtype) return;

	int anim_data = info->animdata; 
	bool dirty = false;

	if (((int)last_setup%6) != (objid%6) && info->animtype != 1)
		return;

	switch(info->animtype) {
	case 2:
		// 50 % chance
		if (std::rand() & 1) break;

	case 1:
	case 3:
		// 50 % chance
		if (anim_data == 1 && (std::rand() & 1) ) break;
		frame ++;
		if (anim_data < 2) {
			if (shp && frame == shp->frameCount()) frame = 0;
		}
		else {
			unsigned int num = (frame-1) / anim_data;
			if (frame == ((num+1)*anim_data)) frame = num*anim_data;
		}
		dirty = true;
		break;

	case 4:
		if (!(std::rand() % anim_data)) break;
		frame ++;
		if (shp && frame == shp->frameCount()) frame = 0;
		dirty = true;
		break;


	case 5:
		callUsecodeEvent_anim();
		dirty = true;
		break;

	case 6:
		if (anim_data < 2) {
			if (frame == 0) break;
			frame ++;
			if (shp && frame == shp->frameCount()) frame = 1;
		}
		else {
			if (!(frame % anim_data)) break;
			frame ++;
			unsigned int num = (frame-1) / anim_data;
			if (frame == ((num+1)*anim_data)) frame = num*anim_data+1;
		}
		dirty = true;
		break;

	default:
		pout <<"type " << info->animtype << " data " << anim_data  << std::endl;
		break;
	}
	//return dirty;
}


// Called when an item has entered the fast area
void Item::inFastArea(int even_odd, int framenum)
{
	extendedflags &= ~(EXT_FAST0|EXT_FAST1);
	extendedflags |= EXT_FAST0<<even_odd;

	if (!last_setup || framenum != (int)last_setup || !(flags & FLG_FASTAREA))
	{
		setupLerp(!last_setup || (framenum-last_setup)>1 || !(flags & FLG_FASTAREA));
		last_setup = framenum;
		if ((framenum%3) == (objid%3)) animateItem();
	}

	// Only do it if not already fast
	if (flags & FLG_FASTAREA) return;

	flags |= FLG_FASTAREA;


	//!! HACK to get rid of endless SFX loops
	if (shape == 0x2c8) return;

	// Call usecode here
	callUsecodeEvent_enterFastArea();
}

// Called when an item is leaving the fast area
void Item::leavingFastArea()
{
	// Clear the fast area flags
	extendedflags &= ~(EXT_FAST0|EXT_FAST1);

	// Only do it if fast
	if (!(flags & FLG_FASTAREA)) return;

	flags &= ~FLG_FASTAREA;

	// Call usecode here
	callUsecodeEvent_leaveFastArea();
}

void Item::clearGump()
{
	gump = 0;
	flags &= ~FLG_GUMP_OPEN;
}

void Item::fall()
{
	if (flags & FLG_HANGING || getShapeInfo()->is_fixed()) {
		// can't fall
		return;
	}

	GravityProcess* p = 0;
	if (gravitypid) {
		p = p_dynamic_cast<GravityProcess*>(
			Kernel::get_instance()->getProcess(gravitypid));
		assert(p);
	} else {
		p = new GravityProcess(this, 0);
		Kernel::get_instance()->addProcess(p);
		p->init();
	}

	p->setGravity(4); //!! constant
}

void Item::grab()
{
	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	World* world= World::get_instance();
	world->getCurrentMap()->surfaceSearch(&uclist, script, sizeof(script),
										  this, true, false, true);

	for (uint32 i = 0; i < uclist.getSize(); i++)
	{
		Item *item = world->getItem(uclist.getuint16(i));
		if (!item) continue;
		item->fall();
	}
}

void Item::saveData(ODataSource* ods)
{
	ods->write2(1); // version
	Object::saveData(ods);
	ods->write4(shape);
	ods->write4(frame);
	ods->write2(x);
	ods->write2(y);
	ods->write2(z);
	ods->write2(flags);
	ods->write2(quality);
	ods->write2(npcnum);
	ods->write2(mapnum);
	ods->write4(extendedflags);
	ods->write2(gump);
	ods->write2(gravitypid);
	ods->write2(glob_next);
}

bool Item::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Object::loadData(ids)) return false;

	shape = ids->read4();
	frame = ids->read4();
	x = ids->read2();
	y = ids->read2();
	z = ids->read2();
	flags = ids->read2();
	quality = ids->read2();
	npcnum = ids->read2();
	mapnum = ids->read2();
	extendedflags = ids->read4();
	gump = ids->read2();
	gravitypid = ids->read2();
	glob_next = ids->read2();

	//!! hackish...
	if (extendedflags & EXT_INCURMAP) {
		World::get_instance()->getCurrentMap()->addItem(this);
	}

	return true;
}


uint32 Item::I_touch(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item);

	// Guess: this is used to make sure an item is painted in the original.
	// Our renderer is different, making this intrinsic unnecessary.

	return 0;
}

uint32 Item::I_getX(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocationAbsolute(x,y,z);
	return x;
}

uint32 Item::I_getY(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocationAbsolute(x,y,z);
	return y;
}

uint32 Item::I_getZ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocationAbsolute(x,y,z);
	return z;
}

uint32 Item::I_getCX(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocationAbsolute(x,y,z);

	if (item->flags & FLG_FLIPPED)
		return x - item->getShapeInfo()->y * 16;
	else
		return x - item->getShapeInfo()->x * 16;
}

uint32 Item::I_getCY(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocationAbsolute(x,y,z);

	if (item->flags & FLG_FLIPPED)
		return y - item->getShapeInfo()->x * 16;
	else
		return y - item->getShapeInfo()->y * 16;
}

uint32 Item::I_getCZ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocationAbsolute(x,y,z);

	return z + item->getShapeInfo()->z * 4;
}

uint32 Item::I_getPoint(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UC_PTR(ptr);
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocationAbsolute(x,y,z);

	WorldPoint point;
	point.setX(x);
	point.setY(y);
	point.setZ(z);

	UCMachine::get_instance()->assignPointer(ptr, point.buf, 5);

	return 0;
}

uint32 Item::I_getShape(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getShape();
}

uint32 Item::I_setShape(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(shape);
	if (!item) return 0;

	item->setShape(shape);
	return 0;
}

uint32 Item::I_getFrame(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getFrame();
}

uint32 Item::I_setFrame(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(frame);
	if (!item) return 0;

	item->setFrame(frame);
	return 0;
}

uint32 Item::I_getQuality(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUALITY)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getUnkEggType(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_UNKEGG)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getQuantity(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUANTITY ||
		item->getFamily() == ShapeInfo::SF_REAGENT)
		return item->getQuality();
	else
		return 0;
}

uint32 Item::I_getContainer(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	Container *parent = item->getParent();

	//! What do we do if item has no parent?
	//! What do we do with equipped items?

	if (parent)
		return parent->getObjId();
	else
		return 0;
}

uint32 Item::I_getRootContainer(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	Container *parent = item->getParent();

	//! What do we do if item has no parent?
	//! What do we do with equipped items?

	if (!parent) return 0;

	while (parent->getParent()) {
		parent = parent->getParent();
	}

	return parent->getObjId();
}

uint32 Item::I_getQ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getQuality();
}

uint32 Item::I_setQ(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	item->setQuality(q);
	return 0;
}

uint32 Item::I_setQuality(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	if (item->getFamily() != ShapeInfo::SF_GENERIC)
		item->setQuality(q);

	return 0;
}

uint32 Item::I_setQuantity(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(q);
	if (!item) return 0;

	if (item->getFamily() == ShapeInfo::SF_QUANTITY ||
		item->getFamily() == ShapeInfo::SF_REAGENT)
		item->setQuality(q);

	return 0;
}

uint32 Item::I_getFamily(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getFamily();
}

uint32 Item::I_getTypeFlag(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(typeflag);
	if (!item) return 0;

	ShapeInfo *info = item->getShapeInfo();

	if (typeflag >= 64)
		perr << "Invalid TypeFlag greater than 63 requested (" << typeflag << ") by Usecode" << std::endl;

	if (info->getTypeFlag(typeflag))
		return 1;
	else
		return 0;
}

uint32 Item::I_getStatus(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getFlags();
}

uint32 Item::I_orStatus(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->flags |= mask;
	return 0;
}

uint32 Item::I_andStatus(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mask);
	if (!item) return 0;

	item->flags &= mask;
	return 0;
}


uint32 Item::I_getWeight(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getShapeInfo()->weight;
}

uint32 Item::I_getWeightIncludingContents(const uint8* args,
										  unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getTotalWeight();
}

uint32 Item::I_bark(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(str);
	if (id_item == 666) item = World::get_instance()->getItem(1);
	if (!item) return 0;	// Hack!

	GUIApp *app = p_dynamic_cast<GUIApp*>(GUIApp::get_instance());

	if (!app)
	{
		pout << std::endl << std::endl << str  << std::endl << std::endl;
		
		// wait a while
		return Kernel::get_instance()->addProcess(new DelayProcess(50));
	}
	else
	{
		Gump *desktop = app->getDesktopGump();
		Gump *gump = new BarkGump(item->getObjId(), str);
		gump->InitGump();
		desktop->AddChild(gump);

		return gump->GetNotifyProcess()->getPid();
	}
	// TODO:
	// * If multiple things are barked for a single item, they
	//   shouldn't be placed on top of eachother.
	// * We shouldn't output more than 5 lines at once.
}

uint32 Item::I_look(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->callUsecodeEvent_look();
}

uint32 Item::I_use(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->callUsecodeEvent_use();
}

uint32 Item::I_enterFastArea(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->callUsecodeEvent_enterFastArea();
}

uint32 Item::I_ask(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // currently unused.
	ARG_LIST(answers);

	if (!answers) return 0;

	// Use AskGump
	GUIApp *app = p_dynamic_cast<GUIApp*>(GUIApp::get_instance());
	Gump *desktop = app->getDesktopGump();
	Gump *gump = new AskGump(1, answers);
	gump->InitGump();
	desktop->AddChild(gump);
	return gump->GetNotifyProcess()->getPid();
}

uint32 Item::I_legalCreateAtPoint(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_WORLDPOINT(point);

	//! haven't checked if this does what it should do.
	// It just creates an item at a worldpoint currently and returns the id.
	// This may have to check for room at the give spot

	Item* newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0);
	if (!newitem) {
		perr << "I_legalCreateAtPoint failed to create item (" << shape
			 <<	"," << frame << ")." << std::endl;
		return 0;
	}
	newitem->setLocation(point.getX(), point.getY(), point.getZ());
	uint16 objID = newitem->assignObjId();
	World::get_instance()->getCurrentMap()->addItem(newitem);

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

	return objID;
}

uint32 Item::I_legalCreateAtCoords(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);

	//! haven't checked if this does what it should do.
	// It just creates an item at given coords currently and returns the id
	// This may have to check for room at the give spot

	Item* newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0);
	if (!newitem) {
		perr << "I_legalCreateAtCoords failed to create item (" << shape
			 <<	"," << frame << ")." << std::endl;
		return 0;
	}
	newitem->setLocation(x, y, z);
	uint16 objID = newitem->assignObjId();
	World::get_instance()->getCurrentMap()->addItem(newitem);

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

	return objID;
}

uint32 Item::I_legalCreateInCont(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr
	ARG_UINT16(shape);
	ARG_UINT16(frame);
	ARG_UINT16(container_id);
	ARG_CONTAINER_FROM_ID(container);
	ARG_UINT16(unknown); // ?

	//! haven't checked if this does what it should do.
	// It just creates an item, tries to add it to the given container.
	// If it first, return id; otherwise return 0.

	Item* newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0);
	if (!newitem) {
		perr << "I_legalCreateInCont failed to create item (" << shape
			 <<	"," << frame << ")." << std::endl;
		return 0;
	}

	// also need to check weight, volume maybe??
	if (container->AddItem(newitem)) {
		uint16 objID = newitem->assignObjId();

		uint8 buf[2];
		buf[0] = static_cast<uint8>(objID);
		buf[1] = static_cast<uint8>(objID >> 8);
		UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

		return objID;
	} else {
		// failed to add; clean up
		delete newitem;

		return 0;
	}
}

uint32 Item::I_destroy(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	item->destroy();

	return 0;
}

uint32 Item::I_getFootpad(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UC_PTR(xptr);
	ARG_UC_PTR(yptr);
	ARG_UC_PTR(zptr);
	if (!item) return 0;

	uint8 buf[2];
	sint32 x,y,z;
	item->getFootpad(x,y,z);

	buf[0] = static_cast<uint8>(x);
	buf[1] = static_cast<uint8>(x >> 8);
	UCMachine::get_instance()->assignPointer(xptr, buf, 2);

	buf[0] = static_cast<uint8>(y);
	buf[1] = static_cast<uint8>(y >> 8);
	UCMachine::get_instance()->assignPointer(yptr, buf, 2);

	buf[0] = static_cast<uint8>(z);
	buf[1] = static_cast<uint8>(z >> 8);
	UCMachine::get_instance()->assignPointer(zptr, buf, 2);

	return 0;
}

uint32 Item::I_overlaps(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->overlaps(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_overlapsXY(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->overlapsxy(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_isOn(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	if (item->isOn(*item2))
		return 1;
	else
		return 0;
}

uint32 Item::I_getFamilyOfType(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UINT16(shape);

	return GameData::get_instance()->getMainShapes()->
		getShapeInfo(shape)->family;
}

uint32 Item::I_push(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	//! do we need to check if item is already ethereal?
	//! can equipped or contained items become ethereal?
	if (item->getFlags() & (FLG_CONTAINED | FLG_EQUIPPED | FLG_ETHEREAL))
		return 0;

	World::get_instance()->etherealPush(item->getObjId());
	World::get_instance()->getCurrentMap()->
		removeItemFromList(item, item->x, item->y);
	item->setFlag(FLG_ETHEREAL);

	return 0;
}

uint32 Item::I_create(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UC_PTR(itemptr); // need to store the item id at *itemptr (????)
	ARG_UINT16(shape);
	ARG_UINT16(frame);

	Item* newitem = ItemFactory::createItem(shape, frame, 0, 0, 0, 0, 0);
	if (!newitem) {
		perr << "I_create failed to create item (" << shape
			 <<	"," << frame << ")." << std::endl;
		return 0;
	}
	uint16 objID = newitem->assignObjId();

	newitem->setFlag(FLG_ETHEREAL);
	World::get_instance()->etherealPush(objID);

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(itemptr, buf, 2);

	return objID;
}

uint32 Item::I_pop(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused

	World* w = World::get_instance();

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPop();
	Item* item = w->getItem(objid);
	if (!item) return 0; // top item was invalid

	item->clearFlag(FLG_ETHEREAL);
	w->getCurrentMap()->addItem(item);

#if 0
	sint32 x,y,z;
	item->getLocation(x,y,z);
	perr << "Popping item into map: " << item->getShape() << "," << item->getFrame() << " at (" << x << "," << y << "," << z << ")" << std::endl;
#endif

	//! Anything else?

	return objid;
}

uint32 Item::I_popToCoords(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);

	World* w = World::get_instance();

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPop();
	Item* item = w->getItem(objid);
	if (!item) return 0; // top item was invalid

	item->setLocation(x, y, z);
	item->clearFlag(FLG_ETHEREAL);
	w->getCurrentMap()->addItem(item);
#if 0
	perr << "Popping item into map: " << item->getShape() << "," << item->getFrame() << " at (" << x << "," << y << "," << z << ")" << std::endl;
#endif

	//! Anything else?

	return objid;
}

uint32 Item::I_popToContainer(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_CONTAINER_FROM_ID(container);

	if (!container) {
		perr << "Trying to pop item to invalid container (" << id_container << ")." << std::endl;
		return 0;
	}

	World* w = World::get_instance();

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPop();
	Item* item = w->getItem(objid);
	if (!item) return 0; // top item was invalid

	item->clearFlag(FLG_ETHEREAL);
	container->AddItem(item);

	//! Anything else?

	return objid;
}

uint32 Item::I_popToEnd(const uint8* args, unsigned int /*argsize*/)
{
	ARG_NULL32(); // ARG_ITEM_FROM_PTR(item); // unused
	ARG_CONTAINER_FROM_ID(container);

	if (!container) {
		perr << "Trying to pop item to invalid container (" << id_container << ")." << std::endl;
		return 0;
	}

	World* w = World::get_instance();

	if (w->etherealEmpty()) return 0; // no items left on stack

	uint16 objid = w->etherealPop();
	Item* item = w->getItem(objid);
	if (!item) return 0; // top item was invalid

	item->clearFlag(FLG_ETHEREAL);
	container->AddItem(item);

	//! Anything else?

	//! This should probably be different from I_popToContainer, but
	//! how exactly?

	return objid;
}

uint32 Item::I_move(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	ARG_UINT16(z);
	if (!item) return 0;

	item->move(x,y,z);
	//item->collideMove(x, y, z, true, true);
	return 0;
}

uint32 Item::I_legalMoveToPoint(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_WORLDPOINT(point);
	ARG_UINT16(unknown1); // 0/1
	ARG_UINT16(unknown2); // always 0

	//! haven't checked if this does what it should do.
	// Currently: check if item can exist at point. If so, move it there
	// and return true. If not, return false.

//	if (item->canExistAt(point.getX(), point.getY(), point.getZ())) {
//		item->move(point.getX(), point.getY(), point.getZ());
//		return 1;
//	} else {
	return item->collideMove(point.getX(), point.getY(), point.getZ(), true, false) == 0x4000;
//	}
}



uint32 Item::I_getEtherealTop(const uint8* args, unsigned int /*argsize*/)
{
	World* w = World::get_instance();
	if (w->etherealEmpty()) return 0; // no items left on stack
	return w->etherealPeek();
}


//!!! is this correct?
uint32 Item::I_getMapArray(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	return item->getMapNum();
}

//!!! is this correct?
uint32 Item::I_setMapArray(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(mapnum);
	if (!item) return 0;

	item->setMapNum(mapnum);
	return 0;
}

uint32 Item::I_getDirToCoords(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	if (!item) return 0;

	sint32 ix,iy,iz;
	item->getLocationAbsolute(ix,iy,iz);

	return Get_WorldDirection(y - iy, x - ix);
}

uint32 Item::I_getDirFromCoords(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(x);
	ARG_UINT16(y);
	if (!item) return 0;

	sint32 ix,iy,iz;
	item->getLocationAbsolute(ix,iy,iz);

	return Get_WorldDirection(iy - y, ix - x);
}

uint32 Item::I_getDirToItem(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	sint32 ix,iy,iz;
	item->getLocationAbsolute(ix,iy,iz);

	sint32 i2x,i2y,i2z;
	item2->getLocationAbsolute(i2x,i2y,i2z);

	return Get_WorldDirection(i2y - iy, i2x - ix);
}

uint32 Item::I_getDirFromItem(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_ITEM_FROM_ID(item2);
	if (!item) return 0;
	if (!item2) return 0;

	sint32 ix,iy,iz;
	item->getLocationAbsolute(ix,iy,iz);

	sint32 i2x,i2y,i2z;
	item2->getLocationAbsolute(i2x,i2y,i2z);

	return Get_WorldDirection(iy - i2y, ix - i2x);
}

uint32 Item::I_hurl(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_SINT16(xs);
	ARG_SINT16(ys);
	ARG_SINT16(zs);
	ARG_SINT16(grav);
	if (!item) return 0;

	GravityProcess* p = 0;
	if (item->gravitypid) {
		p = p_dynamic_cast<GravityProcess*>(
			Kernel::get_instance()->getProcess(item->gravitypid));
		assert(p);
	} else {
		p = new GravityProcess(item, grav);
		Kernel::get_instance()->addProcess(p);
		p->init();
	}
	p->move(xs,ys,zs);

	return item->gravitypid;
}

uint32 Item::I_shoot(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_WORLDPOINT(point);
	ARG_UINT16(unk1);
	ARG_UINT16(unk2);
	if (!item) return 0;

	return Kernel::get_instance()->addProcess(new MissileProcess(item,
							point.getX(),point.getY(),point.getZ(),unk1,true));
}

uint32 Item::I_fall(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	item->fall();

	return 0;
}

uint32 Item::I_grab(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	item->grab();

	return 0;
}

uint32 Item::I_openGump(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(gumpshape);
	if (!item) return 0;

	if (item->flags & FLG_GUMP_OPEN) return 0;
	assert(item->gump == 0);

	Shape* shape = GameData::get_instance()->getGumps()->getShape(gumpshape);

	ContainerGump* cgump = new ContainerGump(shape, 0, item->getObjId(),
											 Gump::FLAG_ITEM_DEPENDANT);
	//!!TODO: clean up the way this is set
	//!! having the itemarea associated with the shape through the 
	//!! GumpShapeFlex maybe
	cgump->setItemArea(GameData::get_instance()->
					   getGumps()->getGumpItemArea(gumpshape));
	cgump->InitGump();
	GUIApp *app = GUIApp::get_instance();
	app->getDesktopGump()->AddChild(cgump);
	item->flags |= FLG_GUMP_OPEN;
	item->gump = cgump->getObjId();

	return 0;
}

uint32 Item::I_closeGump(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	if (!(item->flags & FLG_GUMP_OPEN)) return 0;

	Gump* g = GUIApp::get_instance()->getGump(item->getGump());
	assert(g);
	g->Close();

	// can we already clear gump here, or do we need to wait for the gump
	// to really close??
	item->clearGump();

	return 0;
}

uint32 Item::I_guardianBark(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(num);
	if (!item) return 0;

	return item->callUsecodeEvent_guardianBark(num);
}

uint32 Item::I_getSurfaceWeight(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;

	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE);
	World* world= World::get_instance();
	world->getCurrentMap()->surfaceSearch(&uclist, script, sizeof(script),
										  item, true, false, true);

	uint32 weight = 0; 
	for (uint32 i = 0; i < uclist.getSize(); i++)
	{
		Item *other = world->getItem(uclist.getuint16(i));
		if (!other) continue;
		weight += other->getTotalWeight();
	}

	return weight;
}

uint32 Item::I_isExplosive(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	if (!item) return 0;
	return item->getShapeInfo()->is_explode()?1:0;
}

