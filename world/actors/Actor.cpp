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

#include "ObjectManager.h"
#include "Kernel.h"
#include "UCMachine.h"
#include "World.h"
#include "ActorAnimProcess.h"
#include "CurrentMap.h"
#include "Direction.h"
#include "GameData.h"
#include "MainShapeFlex.h"
#include "AnimAction.h"
#include "CurrentMap.h"
#include "ShapeInfo.h"
#include "Pathfinder.h"

#include "ItemFactory.h"
#include "IDataSource.h"
#include "ODataSource.h"

#include "MissileProcess.h" // hack
#include "PathfinderProcess.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Actor,Container);

Actor::Actor()
	: strength(0), dexterity(0), intelligence(0),
	  hitpoints(0), mana(0), alignment(0), enemyalignment(0),
	  lastanim(0), direction(0), actorflags(0)
{

}

Actor::~Actor()
{

}

uint16 Actor::assignObjId()
{
	if (objid == 0xFFFF)
		objid = ObjectManager::get_instance()->assignActorObjId(this);

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter) {
		(*iter)->assignObjId();
		(*iter)->setParent(objid);
	}

	return objid;
}

bool Actor::CanAddItem(Item* item, bool checkwghtvol)
{
	const unsigned int backpack_shape = 529; //!! *cough* constant

	if (!Container::CanAddItem(item, checkwghtvol)) return false;
	if (item->getParent() == objid) return true; // already in here

	// now check 'equipment slots'
	// we can have one item of each equipment type, plus one backpack

	uint32 equiptype = item->getShapeInfo()->equiptype;
	bool backpack = (item->getShape() == backpack_shape);

	// valid item type?
	if (equiptype == ShapeInfo::SE_NONE && !backpack) return false;

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter)
	{
		uint32 cet = (*iter)->getShapeInfo()->equiptype;
		bool cbackpack = ((*iter)->getShape() == backpack_shape);

		// already have an item with the same equiptype
		if (cet == equiptype || (cbackpack && backpack)) return false;
	}

	return true;
}

bool Actor::addItem(Item* item, bool checkwghtvol)
{
	if (!Container::addItem(item, checkwghtvol)) return false;

	item->setFlag(FLG_EQUIPPED);

	uint32 equiptype = item->getShapeInfo()->equiptype;
	item->setZ(equiptype);

	return true;
}

bool Actor::removeItem(Item* item)
{
	if (!Container::removeItem(item)) return false;

	item->clearFlag(FLG_EQUIPPED);

	return true;
}

uint16 Actor::getEquip(uint32 type)
{
	const unsigned int backpack_shape = 529; //!! *cough* constant

	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter)
	{
		uint32 cet = (*iter)->getShapeInfo()->equiptype;
		bool cbackpack = ((*iter)->getShape() == backpack_shape);

		if (cet == type || (cbackpack && type == 7)) // !! constant
			return (*iter)->getObjId();
	}

	return 0;
}

void Actor::teleport(int newmap, sint32 newx, sint32 newy, sint32 newz)
{
	// Set the mapnum
	setMapNum(newmap);

	// Put it in the void
	moveToEtherealVoid();

	// Move it to this map
	if (newmap == World::get_instance()->getCurrentMap()->getNum())
	{
		move(newx, newy, newz);
	}
	// Move it to another map
	else
	{
		World::get_instance()->etherealRemove(objid);
		x = newx;
		y = newy;
		z = newz;
	}
} 

uint16 Actor::doAnim(int anim, int dir)
{
	if (dir < 0 || dir > 8) {
		perr << "Actor::doAnim: Invalid direction (" << dir << ")" <<std::endl;
		return 0;
	}

#if 0
	if (tryAnim(anim, dir)) {
		perr << "Actor::doAnim: tryAnim = Ok!" << std::endl;
	} else {
		perr << "Actor::doAnim: tryAnim = bad!" << std::endl;
	}
#endif

	if (dir == 8) {
		//!!! CHECKME
		//!! what does dir == 8 mean?
		dir = 0;
	}

	Process *p = new ActorAnimProcess(this, anim, dir);

	return Kernel::get_instance()->addProcess(p);
}

bool Actor::tryAnim(int anim, int dir, PathfindingState* state)
{
	//!NOTE: this is broken, as it does not take height differences
	// into account. tryAnim and ActorAnimProcess::run() should be 
	// unified somehow

	CurrentMap* currentmap = World::get_instance()->getCurrentMap();

	AnimAction* animaction = GameData::get_instance()->getMainShapes()->
		getAnim(getShape(), anim);

	if (!animaction) return false;
	if (dir < 0 || dir > 7) return false;

	sint32 start[3];
	sint32 end[3];
	sint32 dims[3];
	std::list<CurrentMap::SweepItem> hit;
	bool flipped, firststep;

	if (state == 0) {
		getLocation(end[0], end[1], end[2]);
		flipped = (getFlags() & Item::FLG_FLIPPED) != 0;
		firststep = (getActorFlags() & Actor::ACT_FIRSTSTEP) != 0;
	} else {
		end[0] = state->x;
		end[1] = state->y;
		end[2] = state->z;
		flipped = state->flipped;
		firststep = state->firststep;
	}

	getFootpadWorld(dims[0], dims[1], dims[2]);

	// getFootpad gets the footpad of the _current_ item, so curflipped
	// is also set to the flipped-state of the current item.
	bool curflipped = (getFlags() & Item::FLG_FLIPPED) != 0;
 
	unsigned int startframe, endframe;
	animaction->getAnimRange(this, dir, startframe, endframe);

	// run through the animation stages
	for (unsigned int f = startframe; f != endframe; ) {
		AnimFrame& frame = animaction->frames[dir][f];

		start[0] = end[0]; start[1] = end[1]; start[2] = end[2];

		end[0] = start[0] + 4 * x_fact[dir] * frame.deltadir;
		end[1] = start[1] + 4 * y_fact[dir] * frame.deltadir;
		end[2] = start[2] + frame.deltaz;

		if (frame.is_flipped() != curflipped) {
			sint32 t = dims[0]; dims[0] = dims[1]; dims[1] = t;
			curflipped = !curflipped;
		}

		hit.clear();
		currentmap->sweepTest(start, end, dims, getObjId(), true, &hit);

		std::list<CurrentMap::SweepItem>::iterator iter;
		for (iter = hit.begin(); iter != hit.end(); ++iter) {
			if (!iter->touching && iter->hit_time != 0x4000 &&
				iter->end_time != 0)
			{
#if 0
				perr << "tryAnim: (" << start[0] << "," << start[1]
					 << "," << start[2] << ")-(" << end[0] << "," << end[1]
					 << "," << end[2] << ") dims (" << dims[0] << ","
					 << dims[1] << "," << dims[2] << ")" << std::endl;
				perr << "tryAnim: blocked by item " << iter->item << ", ";
				Item *item = World::get_instance()->getItem(iter->item);
				if (item) {
					sint32 ix, iy, iz;
					item->getLocation(ix, iy, iz);
					perr << item->getShape() << " (" << ix << "," << iy
						 << "," << iz << ")" << std::endl;
				} else {
					perr << "(invalid)" << std::endl;
				}
#endif
				return false;
			}
		}

		++f;
		if (f != endframe && f >= animaction->size) {
			if (animaction->flags & AnimAction::AAF_LOOPING)
				f = 1;
			else
				f = 0;
		}
			
	}

	// animation ok. Update state
	if (state) {
		// toggle ACT_FIRSTSTEP flag if necessary
		if (animaction->flags & AnimAction::AAF_TWOSTEP) {
			state->firststep = !state->firststep;
		} else {
			state->firststep = true;
		}

		state->lastanim = anim;
		state->direction = dir;
		state->x = end[0];
		state->y = end[1];
		state->z = end[2];
	}
	return true;
}

void Actor::receiveHit(uint16 other, int dir, int damage, uint16 type)
{
	// TODO: implement this
}

void Actor::dumpInfo()
{
	Container::dumpInfo();

	pout << "hp: " << hitpoints << ", mp: " << mana << ", str: " << strength
		 << ", dex: " << dexterity << ", int: " << intelligence << std::endl;
}

void Actor::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Container::saveData(ods);
	ods->write2(strength);
	ods->write2(dexterity);
	ods->write2(intelligence);
	ods->write2(hitpoints);
	ods->write2(mana);
	ods->write2(alignment);
	ods->write2(enemyalignment);
	ods->write2(lastanim);
	ods->write2(direction);
	ods->write4(actorflags);
}

bool Actor::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Container::loadData(ids)) return false;

	strength = static_cast<sint16>(ids->read2());
	dexterity = static_cast<sint16>(ids->read2());
	intelligence = static_cast<sint16>(ids->read2());
	hitpoints = ids->read2();
	mana = static_cast<sint16>(ids->read2());
	alignment = ids->read2();
	enemyalignment = ids->read2();
	lastanim = ids->read2();
	direction = ids->read2();
	actorflags = ids->read4();

	return true;
}


uint32 Actor::I_isNPC(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;
	return 1;
}

uint32 Actor::I_getMap(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getMapNum();
}

uint32 Actor::I_teleport(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(newx);
	ARG_UINT16(newy);
	ARG_UINT16(newz);
	ARG_UINT16(newmap);
	if (!actor) return 0;

	actor->teleport(newmap,newx,newy,newz);
	return 0;
}

uint32 Actor::I_doAnim(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(anim);
	ARG_UINT16(dir); // seems to be 0-8
	ARG_UINT16(unk1); // this is almost always 10000 in U8.Maybe speed-related?
	ARG_UINT16(unk2); // appears to be 0 or 1. Some flag?

	if (!actor) return 0;

	return actor->doAnim(anim, dir);
}

uint32 Actor::I_getDir(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getDir();
}

uint32 Actor::I_getLastAnimSet(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getLastAnim();
}

uint32 Actor::I_getStr(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getStr();
}

uint32 Actor::I_getDex(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getDex();
}

uint32 Actor::I_getInt(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getInt();
}

uint32 Actor::I_getHp(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getHP();
}

uint32 Actor::I_getMana(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getMana();
}

uint32 Actor::I_getAlignment(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getAlignment();
}

uint32 Actor::I_getEnemyAlignment(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	return actor->getEnemyAlignment();
}

uint32 Actor::I_setStr(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(str);
	if (!actor) return 0;

	actor->setStr(str);
	return 0;
}

uint32 Actor::I_setDex(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(dex);
	if (!actor) return 0;

	actor->setDex(dex);
	return 0;
}

uint32 Actor::I_setInt(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(int_);
	if (!actor) return 0;

	actor->setStr(int_);
	return 0;
}

uint32 Actor::I_setHp(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(hp);
	if (!actor) return 0;

	actor->setHP(hp);
	return 0;
}

uint32 Actor::I_setMana(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_SINT16(mp);
	if (!actor) return 0;

	actor->setMana(mp);
	return 0;
}

uint32 Actor::I_setAlignment(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(a);
	if (!actor) return 0;

	actor->setAlignment(a);
	return 0;
}

uint32 Actor::I_setEnemyAlignment(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(a);
	if (!actor) return 0;

	actor->setEnemyAlignment(a);
	return 0;
}

uint32 Actor::I_isInCombat(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->isInCombat())
		return 1;
	else
		return 0;
}

uint32 Actor::I_isDead(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->getActorFlags() & ACT_DEAD)
		return 1;
	else
		return 0;
}

uint32 Actor::I_isImmortal(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->getActorFlags() & ACT_IMMORTAL)
		return 1;
	else
		return 0;
}

uint32 Actor::I_isWithstandDeath(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->getActorFlags() & ACT_WITHSTANDDEATH)
		return 1;
	else
		return 0;
}

uint32 Actor::I_isFeignDeath(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	if (actor->getActorFlags() & ACT_FEIGNDEATH)
		return 1;
	else
		return 0;
}

uint32 Actor::I_pathfindToItem(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_OBJID(id2);
	Item* item = World::get_instance()->getItem(id2);
	if (!actor) return 0;
	if (!item) return 0;

	sint32 x,y,z;
	item->getLocation(x,y,z);

	return Kernel::get_instance()->addProcess(
//		new PathfinderProcess(actor,x,y,z));
		new MissileProcess(actor,x,y,z,100));
}

uint32 Actor::I_isBusy(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UC_PTR(ptr);
	uint16 id = UCMachine::ptrToObject(ptr);

	uint32 count = Kernel::get_instance()->getNumProcesses(id, 0x00F0);
	if (count > 0)
		return 1;
	else
		return 0;
}

uint32 Actor::I_createActor(const uint8* args, unsigned int /*argsize*/)
{
	ARG_UC_PTR(ptr);
	ARG_UINT16(shape);
	ARG_UINT16(unknown); // !!! what's this?

	//!! do we need to flag actor as temporary somehow?

	Actor* newactor = ItemFactory::createActor(shape, 0, 0, Item::FLG_IN_NPC_LIST, 0, 0, 0);
	if (!newactor) {
		perr << "I_createActor failed to create actor (" << shape
			 <<	")." << std::endl;
		return 0;
	}
	uint16 objID = newactor->assignObjId();

	Actor* av = World::get_instance()->getNPC(1);
	newactor->setMapNum(av->getMapNum());
	newactor->setNpcNum(objID);
	newactor->setFlag(FLG_ETHEREAL);
	World::get_instance()->etherealPush(objID);

	uint8 buf[2];
	buf[0] = static_cast<uint8>(objID);
	buf[1] = static_cast<uint8>(objID >> 8);
	UCMachine::get_instance()->assignPointer(ptr, buf, 2);

#if 0
	perr << "I_createActor: created actor #" << objID << " with shape " << shape << std::endl;
#endif

	return objID;
}

uint32 Actor::I_getEquip(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(type);
	if (!actor) return 0;

	return actor->getEquip(type+1);
}
