/*
Copyright (C) 2003-2004 The Pentagram team

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
#include "UCList.h"
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
#include "Animation.h"
#include "DelayProcess.h"
#include "ResurrectionProcess.h"
#include "DeleteActorProcess.h"
#include "Shape.h"

#include "ItemFactory.h"
#include "LoopScript.h"
#include "IDataSource.h"
#include "ODataSource.h"

#include "MissileProcess.h" // hack
#include "PathfinderProcess.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(Actor,Container);

Actor::Actor()
	: strength(0), dexterity(0), intelligence(0),
	  hitpoints(0), mana(0), alignment(0), enemyalignment(0),
	  lastanim(Animation::walk), animframe(0), direction(0), actorflags(0)
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

sint16 Actor::getMaxMana() const
{
	return static_cast<sint16>(2 * getInt());
}

uint16 Actor::getMaxHP() const
{
	return static_cast<uint16>(2 * getStr());
}

bool Actor::loadMonsterStats()
{
	ShapeInfo* shapeinfo = getShapeInfo();
	MonsterInfo* mi = 0;
	if (shapeinfo) mi = shapeinfo->monsterinfo;
	if (!mi)
		return false;

	uint16 hp;
	if (mi->max_hp <= mi->min_hp)
		hp = mi->min_hp;
	else
		hp = mi->min_hp + std::rand() % (mi->max_hp - mi->min_hp);
	setHP(hp);
	
	uint16 dex;
	if (mi->max_dex <= mi->min_dex)
		dex = mi->min_dex;
	else
		dex = mi->min_dex + std::rand() % (mi->max_dex - mi->min_dex);
	setDex(dex);
	
	uint8 alignment = mi->alignment;
	setAlignment(alignment & 0x0F);
	setEnemyAlignment((alignment & 0xF0) >> 4); // !! CHECKME

	return true;
}

bool Actor::removeItem(Item* item)
{
	if (!Container::removeItem(item)) return false;

	item->clearFlag(FLG_EQUIPPED); // unequip if necessary

	return true;
}

bool Actor::setEquip(Item* item, bool checkwghtvol)
{
	const unsigned int backpack_shape = 529; //!! *cough* constant
	uint32 equiptype = item->getShapeInfo()->equiptype;
	bool backpack = (item->getShape() == backpack_shape);

	// valid item type?
	if (equiptype == ShapeInfo::SE_NONE && !backpack) return false;

	// now check 'equipment slots'
	// we can have one item of each equipment type, plus one backpack
	std::list<Item*>::iterator iter;
	for (iter = contents.begin(); iter != contents.end(); ++iter)
	{
		if ((*iter)->getObjId() == item->getObjId()) continue;

		uint32 cet = (*iter)->getShapeInfo()->equiptype;
		bool cbackpack = ((*iter)->getShape() == backpack_shape);

		// already have an item with the same equiptype
		if (cet == equiptype || (cbackpack && backpack)) return false;
	}

	if (!Container::addItem(item, checkwghtvol)) return false;

	item->setFlag(FLG_EQUIPPED);
	item->setZ(equiptype);

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

		if (((*iter)->getFlags() & FLG_EQUIPPED) &&
			(cet == type || (cbackpack && type == 7))) // !! constant
		{
			return (*iter)->getObjId();
		}
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
		perr << "Actor::teleport: " << getObjId() << " to " << newmap << ","
			 << newx << "," << newy << "," << newz << std::endl;
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

uint16 Actor::doAnim(Animation::Sequence anim, int dir)
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
		//!! what does dir == 8 mean? Guessing 'last direction'
		// (should it be direction 'now' or direction when starting animation?)
		dir = direction;
	}

	Process *p = new ActorAnimProcess(this, anim, dir);

	return Kernel::get_instance()->addProcess(p);
}

Animation::Result Actor::tryAnim(Animation::Sequence anim, int dir, PathfindingState* state)
{
	//!NOTE: this is broken, as it does not take height differences
	// into account. tryAnim and ActorAnimProcess::run() should be 
	// unified somehow

	CurrentMap* currentmap = World::get_instance()->getCurrentMap();

	AnimAction* animaction = GameData::get_instance()->getMainShapes()->
		getAnim(getShape(), anim);

	if (!animaction) return Animation::FAILURE;
	if (dir < 0 || dir > 7) return Animation::FAILURE;

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
				return Animation::FAILURE;
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
	
	//check if the animation completes on solid ground
	UCList uclist(2);
	LOOPSCRIPT(script, LS_TOKEN_TRUE); // we want all items
	currentmap->surfaceSearch(&uclist, script, sizeof(script),
										  getObjId(), end, dims,
										  false, true, false);

	for (uint32 i = 0; i < uclist.getSize(); i++)
	{
		Item *item = World::get_instance()->getItem(uclist.getuint16(i));
		if (item->getShapeInfo()->is_land())
			return Animation::SUCCESS;
	}


	return Animation::END_OFF_LAND;
}

uint32 Actor::getArmourClass()
{
	ShapeInfo* si = getShapeInfo();
	if (si->monsterinfo)
		return si->monsterinfo->armour_class;
	else
		return 0;
}

uint16 Actor::getDefenseType()
{
	ShapeInfo* si = getShapeInfo();
	if (si->monsterinfo)
		return si->monsterinfo->defense_type;
	else
		return 0;
}

sint16 Actor::getDefendingDex()
{
	return getDex();
}

sint16 Actor::getAttackingDex()
{
	return getDex();
}

uint16 Actor::getDamageType()
{
	ShapeInfo* si = getShapeInfo();
	if (si->monsterinfo)
		return si->monsterinfo->damage_type;
	else 
		return WeaponInfo::DMG_NORMAL;
}


int Actor::getDamageAmount()
{
	ShapeInfo* si = getShapeInfo();
	if (si->monsterinfo) {

		int min = static_cast<int>(si->monsterinfo->min_dmg);
		int max = static_cast<int>(si->monsterinfo->max_dmg);
		
		int damage = (std::rand() % (max - min + 1)) + min;
		
		return damage;
	} else {
		return 1;
	}
}


void Actor::receiveHit(uint16 other, int dir, int damage, uint16 damage_type)
{
	if (getActorFlags() & ACT_DEAD)
		return; // already dead, so don't bother

	pout << "Actor " << getObjId() << " received hit from " << other << ". ";

	damage = calculateAttackDamage(other, damage, damage_type);

	if (!damage) {
		pout << "No damage." << std::endl;
		return; // attack missed
	} else {
		pout << "Damage: " << damage << std::endl;
	}

	// TODO: accumulate strength for avatar kicks
	// TODO: accumulate dexterity for avatar hits
	// TODO: make us hostile to whoever attacked?

	if (getActorFlags() & (ACT_IMMORTAL | ACT_INVINCIBLE))
		return; // invincible
 
	if (damage >= hitpoints) {
		// we're dead

		if (getActorFlags() & ACT_WITHSTANDDEATH) {
			// or maybe not...

			setHP(getMaxHP());
			// TODO: SFX
			clearActorFlag(ACT_WITHSTANDDEATH);
		} else {
			die();
		}
	} else {
		setHP(static_cast<uint16>(hitpoints - damage));
	}
}

void Actor::die()
{
	setHP(0);
	setActorFlag(ACT_DEAD);

	Kernel::get_instance()->killProcesses(getObjId(), 6); // CONSTANT!

	ProcId animprocid = doAnim(Animation::die, getDir());

	// TODO: Lots, including, but not limited to:
	// * fill with treasure if appropriate
	// * some U8 monsters need special actions: skeletons, eyebeasts, etc...

	Item* potion = ItemFactory::createItem(766, 1, 0, 0, 0, 0, 0);
	potion->assignObjId();
	potion->moveToContainer(this);
	potion->setGumpLocation(20, 20);

	ShapeInfo* shapeinfo = getShapeInfo();
	MonsterInfo* mi = 0;
	if (shapeinfo) mi = shapeinfo->monsterinfo;

	if (mi && mi->resurrection) {
		// this monster will be resurrected after a while

		pout << "Actor::die: scheduling resurrection" << std::endl;

		int timeout = ((std::rand() % 25) + 5) * 30; // 5-30 seconds

		Process* resproc = new ResurrectionProcess(this);
		Kernel::get_instance()->addProcess(resproc);

		Process* delayproc = new DelayProcess(timeout);
		Kernel::get_instance()->addProcess(delayproc);

		ProcId animpid = doAnim(Animation::standUp, 8);
		Process* animproc = Kernel::get_instance()->getProcess(animpid);
		assert(animproc);

		resproc->waitFor(delayproc);
		animproc->waitFor(resproc);
	}

	if (mi && mi->explode) {
		// this monster explodes when it dies

		pout << "Actor::die: exploding" << std::endl;

		int count = 5;
		Shape* explosionshape = GameData::get_instance()->getMainShapes()
			->getShape(mi->explode);
		assert(explosionshape);
		unsigned int framecount = explosionshape->frameCount();

		for (int i = 0; i < count; ++i) {
			Item* piece = ItemFactory::createItem(mi->explode,
												  std::rand()%framecount,
												  0, // qual
												  Item::FLG_FAST_ONLY, //flags,
												  0, // npcnum
												  0, // mapnum
												  0 // extended. flags
				);
			piece->assignObjId();
			piece->move(x - 128 + 32*(std::rand()%6),
						y - 128 + 32*(std::rand()%6),
						z + std::rand()%8 ); // move to near actor's position
			piece->hurl(-25 + (std::rand()%50),
						-25 + (std::rand()%50),
						10 + (std::rand()%10),
						4); // (wrong?) CONSTANTS!
		}
	}

	if (mi && mi->vanish) {
		// body disappears after the death animation

		pout << "Actor::die: scheduling vanishing" << std::endl;

		Process* vanishproc = new DeleteActorProcess(this);
		Kernel::get_instance()->addProcess(vanishproc);

		vanishproc->waitFor(animprocid);
	}
}

int Actor::calculateAttackDamage(uint16 other, int damage, uint16 damage_type)
{
	Item* hitter = World::get_instance()->getItem(other);
	Actor* attacker = World::get_instance()->getNPC(other);

	if (damage == 0 && attacker) {
		damage = attacker->getDamageAmount();
	}

	if (damage_type == 0 && hitter) {
		damage_type = hitter->getDamageType();
	}

	uint16 defense_type = getDefenseType();

	// most damage types are blocked straight away by defense types
	damage_type &= ~(defense_type & ~(WeaponInfo::DMG_MAGIC  |
									  WeaponInfo::DMG_UNDEAD |
									  WeaponInfo::DMG_PIERCE));

	// immunity to non-magical weapons
	if ((defense_type & WeaponInfo::DMG_MAGIC) &&
		!(damage_type & WeaponInfo::DMG_MAGIC))
	{
		damage = 0;
	}

	bool slayer = false;

	// special attacks
	if (damage && damage_type)
	{
		if (damage_type & WeaponInfo::DMG_SLAYER) {
			if (std::rand() % 10 == 0)
				damage = 255; // instant kill
		}

		if ((damage_type & WeaponInfo::DMG_UNDEAD) &&
			(defense_type & WeaponInfo::DMG_UNDEAD))
		{
			damage *= 2; // double damage against undead
			slayer = true;
		}

		if ((defense_type & WeaponInfo::DMG_PIERCE) &&
			!(damage_type & (WeaponInfo::DMG_BLADE |
							 WeaponInfo::DMG_FIRE  |
							 WeaponInfo::DMG_PIERCE)))
		{
			damage /= 2; // resistance to blunt damage
		}
	} else {
		damage = 0;
	}

	// armour
	if (damage && !(damage_type & WeaponInfo::DMG_PIERCE) && !slayer)
	{
		// blocking?
		if ((getLastAnim() == Animation::startblock ||
			 getLastAnim() == Animation::stopblock) &&
			!(getActorFlags() & ACT_STUNNED))
		{
			damage -= getStr() / 5;
		}

		int ACmod = 3 * getArmourClass();
		if (damage_type & WeaponInfo::DMG_FIRE)
			ACmod /= 2; // armour doesn't protect from fire as well

		if (getActorFlags() & ACT_STUNNED)
			ACmod /= 2; // stunned?

		if (ACmod > 100) ACmod = 100;

		// TODO: replace rounding bias by something random
		damage = ((100 - ACmod) * damage) / 100;

		if (damage < 0) damage = 0;
	}

	// to-hit
	if (damage && !(damage_type & WeaponInfo::DMG_PIERCE) && attacker)
	{
		bool hit = false;
		sint16 attackdex = attacker->getAttackingDex();
		sint16 defenddex = getDefendingDex();
		if (attackdex < 0) attackdex = 0;
		if (defenddex <= 0) defenddex = 1;

		if ((getActorFlags() & ACT_STUNNED) ||
			(rand() % (attackdex + 3) > rand() % defenddex))
		{
			hit = true;
		}

		// TODO: give avatar an extra chance to hit monsters
		//       with defense_type DMG_PIERCE

		if (!hit) {
			damage = 0;
		}
	}

	return damage;
}

void Actor::dumpInfo()
{
	Container::dumpInfo();

	pout << "hp: " << hitpoints << ", mp: " << mana << ", str: " << strength
		 << ", dex: " << dexterity << ", int: " << intelligence
		 << ", ac: " << getArmourClass() << ", defense: " << std::hex
		 << getDefenseType() << std::dec << std::endl;
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
	ods->write2(animframe);
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
	lastanim = (Animation::Sequence) ids->read2();
	animframe = ids->read2();
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

	return actor->doAnim((Animation::Sequence) anim, dir);
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

uint32 Actor::I_isEnemy(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_ACTOR_FROM_ID(other);
	if (!actor) return 0;
	if (!other) return 0;

	if (actor->getEnemyAlignment() & other->getAlignment())
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

uint32 Actor::I_setDead(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->setActorFlag(ACT_DEAD);

	return 0;
}

uint32 Actor::I_clrDead(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_DEAD);

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

uint32 Actor::I_setImmortal(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->setActorFlag(ACT_IMMORTAL);
	actor->clearActorFlag(ACT_INVINCIBLE);

	return 0;
}

uint32 Actor::I_clrImmortal(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_IMMORTAL);

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

uint32 Actor::I_setWithstandDeath(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->setActorFlag(ACT_WITHSTANDDEATH);

	return 0;
}

uint32 Actor::I_clrWithstandDeath(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	if (!actor) return 0;

	actor->clearActorFlag(ACT_WITHSTANDDEATH);

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

	return Kernel::get_instance()->addProcess(
		new PathfinderProcess(actor,id2));
//		new MissileProcess(actor,x,y,z,100));
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

	//!! do we need to flag actor as temporary?

	Actor* newactor = ItemFactory::createActor(shape, 0, 0, Item::FLG_IN_NPC_LIST, 0, 0, 0);
	if (!newactor) {
		perr << "I_createActor failed to create actor (" << shape
			 <<	")." << std::endl;
		return 0;
	}
	uint16 objID = newactor->assignObjId();

	// set stats
	if (!newactor->loadMonsterStats()) {
		perr << "I_createActor failed to set stats for actor (" << shape
			 << ")." << std::endl;
	}

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

uint32 Actor::I_setEquip(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ACTOR_FROM_PTR(actor);
	ARG_UINT16(type);
	ARG_ITEM_FROM_ID(item);
	if (!actor) return 0;
	if (!item) return 0;

	if (!actor->setEquip(item, false))
		return 0;

	// check it was added to the right slot
	assert(item->getZ() == type+1);

	return 1;
}
