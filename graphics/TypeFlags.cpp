/*
 *  Copyright (C) 2003-2004 The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include "TypeFlags.h"
#include "IDataSource.h"
#include "ConfigFileManager.h"
#include "CoreApp.h"
#include "GameData.h"
#include "MainShapeFlex.h"
#include "Shape.h"
#include "TreasureLoader.h"

TypeFlags::TypeFlags()
{

}


TypeFlags::~TypeFlags()
{

}

ShapeInfo* TypeFlags::getShapeInfo(uint32 shapenum)
{
	if (shapenum < shapeInfo.size())
		return &(shapeInfo[shapenum]);
	else
		return 0;
}


void TypeFlags::load(IDataSource *ds)
{
	// TODO: detect U8/crusader format somehow?!
	// (Or probably pass it as parameter)
	// The 'parsing' below is only for U8

	uint32 size = ds->getSize();
	uint32 count = size / 8;

	shapeInfo.clear();
	shapeInfo.resize(count);

	for (uint32 i = 0; i < count; ++i)
	{
		uint8 data[8];
		ds->read(data, 8);

		ShapeInfo si;

		si.flags = 0;
		if (data[0] & 0x01) si.flags |= ShapeInfo::SI_FIXED;
		if (data[0] & 0x02) si.flags |= ShapeInfo::SI_SOLID;
		if (data[0] & 0x04) si.flags |= ShapeInfo::SI_SEA;
		if (data[0] & 0x08) si.flags |= ShapeInfo::SI_LAND;
		if (data[0] & 0x10) si.flags |= ShapeInfo::SI_OCCL;
		if (data[0] & 0x20) si.flags |= ShapeInfo::SI_BAG;
		if (data[0] & 0x40) si.flags |= ShapeInfo::SI_DAMAGING;
		if (data[0] & 0x80) si.flags |= ShapeInfo::SI_NOISY;

		if (data[1] & 0x01) si.flags |= ShapeInfo::SI_DRAW;
		if (data[1] & 0x02) si.flags |= ShapeInfo::SI_IGNORE;
		if (data[1] & 0x04) si.flags |= ShapeInfo::SI_ROOF;
		if (data[1] & 0x08) si.flags |= ShapeInfo::SI_TRANSL;
		si.family = data[1] >> 4;

		si.equiptype = data[2] & 0x0F;
		si.x = data[2] >> 4;

		si.y = data[3] & 0x0F;
		si.z = data[3] >> 4;

		si.animtype = data[4] & 0x0F;
		si.animdata = data[4] >> 4;

		si.unknown = data[5] & 0x0F;
		if (data[5] & 0x10) si.flags |= ShapeInfo::SI_EDITOR;
		if (data[5] & 0x20) si.flags |= ShapeInfo::SI_EXPLODE;
		if (data[5] & 0x40) si.flags |= ShapeInfo::SI_UNKNOWN46;
		if (data[5] & 0x80) si.flags |= ShapeInfo::SI_UNKNOWN47;

		si.weight = data[6];

		si.volume = data[7];

		si.weaponinfo = 0;
		si.armourinfo = 0;
		
		shapeInfo[i] = si;
	}

	loadWeaponInfo();
	loadArmourInfo();
	loadMonsterInfo();
}


// load weapon info from the 'weapons' config root
void TypeFlags::loadWeaponInfo()
{
	ConfigFileManager* config = ConfigFileManager::get_instance();

	// load weapons
	std::vector<Pentagram::istring> weaponkeys;
	weaponkeys = config->listSections("weapons", true);	
	for (std::vector<Pentagram::istring>::iterator iter = weaponkeys.begin();
		 iter != weaponkeys.end(); ++iter)
	{
		Pentagram::istring k = *iter;
		WeaponInfo* wi = new WeaponInfo;

		int val;

		config->get(k + "/shape", val);
		wi->shape = static_cast<uint32>(val);

		config->get(k + "/overlay", val);
		wi->overlay_type = static_cast<uint8>(val);

		config->get(k + "/overlay_shape", val);
		wi->overlay_shape = static_cast<uint32>(val);

		config->get(k + "/damage_mod", val);
		wi->damage_modifier = static_cast<uint8>(val);

		config->get(k + "/base_damage", val);
		wi->base_damage = static_cast<uint8>(val);

		config->get(k + "/attack_dex", val);
		wi->dex_attack_bonus = static_cast<uint8>(val);

		config->get(k + "/defend_dex", val);
		wi->dex_defend_bonus = static_cast<uint8>(val);

		config->get(k + "/armour", val);
		wi->armour_bonus = static_cast<uint8>(val);

		config->get(k + "/damage_type", val);
		wi->damage_type = static_cast<uint16>(val);

		assert(wi->shape < shapeInfo.size());
		shapeInfo[wi->shape].weaponinfo = wi;
	}
}


void TypeFlags::loadArmourInfo()
{
	ConfigFileManager* config = ConfigFileManager::get_instance();
	MainShapeFlex* msf = GameData::get_instance()->getMainShapes();

	// load armour
	std::vector<Pentagram::istring> armourkeys;
	armourkeys = config->listSections("armour", true);	
	for (std::vector<Pentagram::istring>::iterator iter = armourkeys.begin();
		 iter != armourkeys.end(); ++iter)
	{
		Pentagram::istring k = *iter;
		ArmourInfo ai;

		int val;

		config->get(k + "/shape", val);
		ai.shape = static_cast<uint32>(val);

		assert(ai.shape < shapeInfo.size());
		assert(msf->getShape(ai.shape));
		unsigned int framecount = msf->getShape(ai.shape)->frameCount();
		ArmourInfo* aia = shapeInfo[ai.shape].armourinfo;
		if (!aia) {
			aia = new ArmourInfo[framecount];
			shapeInfo[ai.shape].armourinfo = aia;
			for (unsigned int i = 0; i < framecount; ++i) {
				aia[i].shape = 0;
				aia[i].frame = 0;
				aia[i].armour_class = 0;
				aia[i].defense_type = 0;
				aia[i].kick_attack_bonus = 0;
			}
		}

		config->get(k + "/frame", val);
		ai.frame = static_cast<uint32>(val);

		assert(ai.frame < framecount);

		config->get(k + "/armour", val);
		ai.armour_class = static_cast<uint16>(val);

		config->get(k + "/type", val);
		ai.defense_type = static_cast<uint16>(val);

		if (!config->get(k + "/kick_bonus", val)) val = 0;
		ai.kick_attack_bonus = static_cast<uint16>(val);

		aia[ai.frame] = ai;
	}
}

void TypeFlags::loadMonsterInfo()
{
	ConfigFileManager* config = ConfigFileManager::get_instance();

	TreasureLoader treasureLoader;
	treasureLoader.loadDefaults();

	// load monsters
	std::vector<Pentagram::istring> monsterkeys;
	monsterkeys = config->listSections("monsters", true);	
	for (std::vector<Pentagram::istring>::iterator iter = monsterkeys.begin();
		 iter != monsterkeys.end(); ++iter)
	{
		Pentagram::istring k = *iter;
		MonsterInfo* mi = new MonsterInfo;

		int val;

		config->get(k + "/shape", val);
		mi->shape = static_cast<uint32>(val);

		config->get(k + "/hp_min", val);
		mi->min_hp = static_cast<uint16>(val);

		config->get(k + "/hp_max", val);
		mi->max_hp = static_cast<uint16>(val);

		config->get(k + "/dex_min", val);
		mi->min_dex = static_cast<uint16>(val);

		config->get(k + "/dex_max", val);
		mi->max_dex = static_cast<uint16>(val);

		config->get(k + "/damage_min", val);
		mi->min_dmg = static_cast<uint16>(val);

		config->get(k + "/damage_max", val);
		mi->max_dmg = static_cast<uint16>(val);

		config->get(k + "/armour", val);
		mi->armour_class = static_cast<uint16>(val);

		config->get(k + "/alignment", val);
		mi->alignment = static_cast<uint8>(val);

		config->get(k + "/unk", val);
		mi->unk = (val != 0);

		config->get(k + "/damage_type", val);
		mi->damage_type = static_cast<uint16>(val);

		config->get(k + "/defense_type", val);
		mi->defense_type = static_cast<uint16>(val);

		if (config->get(k + "/resurrection", val))
			mi->resurrection = (val != 0);
		else
			mi->resurrection = false;

		if (config->get(k + "/vanish", val))
			mi->vanish = (val != 0);
		else
			mi->vanish = false;

		if (config->get(k + "/explode", val))
			mi->explode = val;
		else
			mi->explode = 0;

		std::string treasure;
		if (config->get(k + "/treasure", treasure)) {
			bool ok = treasureLoader.parse(treasure, mi->treasure);
			if (!ok) {
				perr << "failed to parse treasure info for monster '" << k
					 << "'"  << std::endl;
				mi->treasure.clear();
			}
		} else {			
			mi->treasure.clear();
		}

		assert(mi->shape < shapeInfo.size());
		shapeInfo[mi->shape].monsterinfo = mi;
	}
}
