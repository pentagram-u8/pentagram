/*
Copyright (C) 2004 The Pentagram team

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

#include "U8Game.h"

#include "PaletteManager.h"
#include "IDataSource.h"
#include "FileSystem.h"
#include "GameData.h"
#include "XFormBlend.h"
#include "U8Save.h"
#include "World.h"
#include "MainActor.h"
#include "ItemFactory.h"
#include "Egg.h"
#include "CurrentMap.h"
#include "UCList.h"
#include "LoopScript.h"
#include "ObjectManager.h"
#include "CameraProcess.h"
#include "GUIApp.h"
#include "SettingManager.h"

U8Game::U8Game() : Game()
{

}

U8Game::~U8Game()
{

}

bool U8Game::loadFiles()
{
	// Load palette
	pout << "Load Palette" << std::endl;
	IDataSource *pf = FileSystem::get_instance()->ReadFile("@u8/static/u8pal.pal");
	if (!pf) {
		perr << "Unable to load static/u8pal.pal." << std::endl;
		return false;
	}
	pf->seek(4); // seek past header

	IBufferDataSource xfds(U8XFormPal,1024);
	PaletteManager::get_instance()->load(PaletteManager::Pal_Game, *pf, xfds);
	delete pf;

	pout << "Load GameData" << std::endl;
	GameData::get_instance()->loadU8Data();

	return true;
}

bool U8Game::startGame()
{
	// NOTE: assumes the entire engine has been reset!

	pout << "Starting new Ultima 8 game." << std::endl;

	ObjectManager* objman = ObjectManager::get_instance();

	// reserve a number of objids just in case we'll need them sometime
	for (uint16 i = 384; i < 512; ++i)
		objman->reserveObjId(i);

	// reserve ObjId 666 for the Guardian Bark hack
	objman->reserveObjId(666);

	IDataSource *saveds = FileSystem::get_instance()->ReadFile("@u8/savegame/u8save.000");
	if (!saveds) {
		perr << "Unable to load savegame/u8save.000." << std::endl;
		return false;
	}
	U8Save *u8save = new U8Save(saveds);

	IDataSource *nfd = u8save->get_datasource("NONFIXED.DAT");
	if (!nfd) {
		perr << "Unable to load savegame/u8save.000/NONFIXED.DAT." <<std::endl;
		return false;
	}
	World::get_instance()->loadNonFixed(nfd);

	IDataSource *icd = u8save->get_datasource("ITEMCACH.DAT");
	if (!icd) {
		perr << "Unable to load savegame/u8save.000/ITEMCACH.DAT." <<std::endl;
		return false;
	}
	IDataSource *npcd = u8save->get_datasource("NPCDATA.DAT");
	if (!npcd) {
		perr << "Unable to load savegame/u8save.000/NPCDATA.DAT." << std::endl;
		return false;
	}

	World::get_instance()->loadItemCachNPCData(icd, npcd);
	delete u8save;

	MainActor* av = World::get_instance()->getMainActor();
	assert(av);

	av->setName("Avatar"); // FIXME

	// avatar needs a backpack ... CONSTANTs and all that
	Container* backpack = p_dynamic_cast<Container*>(
		ItemFactory::createItem(529, 0, 0, 0, 0, 0, 0));

	// a little bonus :-)
	Item* money = ItemFactory::createItem(143, 7, 500, 0, 0, 0, 0);
	money->moveToContainer(backpack);
	money->setGumpLocation(40, 20);

	// skull of quakes
	Item *skull = ItemFactory::createItem(814, 0, 0, 0, 0, 0, 0);
	skull->moveToContainer(backpack);  
	skull->setGumpLocation(60, 20);

	// recall item
	Item *recall = ItemFactory::createItem(833, 0, 0, 0, 0, 0, 0);
	recall->moveToContainer(backpack);
	recall->setGumpLocation(20, 20);

	// sword
	Item* sword = ItemFactory::createItem(420, 0, 0, 0, 0, 0, 0);
	sword->moveToContainer(backpack);
	sword->setGumpLocation(20, 30);

	// Deceiver
	Item* deceiver = ItemFactory::createItem(822, 0, 0, 0, 0, 0, 0);
	deceiver->moveToContainer(backpack);
	deceiver->setGumpLocation(20, 30);

	Item* flamesting = ItemFactory::createItem(817, 0, 0, 0, 0, 0, 0);
	flamesting->moveToContainer(backpack);
	flamesting->setGumpLocation(20, 30);

	// armour
	Item* armour = ItemFactory::createItem(64, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(30, 30);

	armour = ItemFactory::createItem(532, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(40, 30);

	armour = ItemFactory::createItem(539, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(50, 30);

	armour = ItemFactory::createItem(530, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(10, 40);

	armour = ItemFactory::createItem(531, 0, 0, 0, 0, 0, 0);
	armour->moveToContainer(backpack);
	armour->setGumpLocation(20, 40);

	// necromancy reagents
	Item* bagitem = ItemFactory::createItem(637, 0, 0, 0, 0, 0, 0);
	bagitem->moveToContainer(backpack);
	bagitem->setGumpLocation(70, 40);

	bagitem = ItemFactory::createItem(637, 0, 0, 0, 0, 0, 0);
	Container* bag = p_dynamic_cast<Container*>(bagitem);
	bagitem->moveToContainer(backpack);
	bagitem->setGumpLocation(70, 20);

	Item* reagents = ItemFactory::createItem(395, 0, 50, 0, 0, 0, 0); 
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(10, 10);
	reagents = ItemFactory::createItem(395, 6, 50, 0, 0, 0, 0); 
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(30, 10);
	reagents = ItemFactory::createItem(395, 8, 50, 0, 0, 0, 0); 
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(50, 10);
	reagents = ItemFactory::createItem(395, 9, 50, 0, 0, 0, 0); 
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(20, 30);
	reagents = ItemFactory::createItem(395, 10, 50, 0, 0, 0, 0); 
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(40, 30);
	reagents = ItemFactory::createItem(395, 14, 50, 0, 0, 0, 0); 
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(60, 30);

	// theurgy foci
	bagitem = ItemFactory::createItem(637, 0, 0, 0, 0, 0, 0);
	bag = p_dynamic_cast<Container*>(bagitem);
	bagitem->moveToContainer(backpack);
	bagitem->setGumpLocation(0, 30);

	Item* focus = ItemFactory::createItem(396, 8, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(10, 10);
	focus = ItemFactory::createItem(396, 9, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(25, 10);
	focus = ItemFactory::createItem(396, 10, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(40, 10);
	focus = ItemFactory::createItem(396, 11, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(55, 10);
	focus = ItemFactory::createItem(396, 12, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(70, 10);
	focus = ItemFactory::createItem(396, 13, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(10, 30);
	focus = ItemFactory::createItem(396, 14, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(30, 30);
	focus = ItemFactory::createItem(396, 15, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(50, 30);	
	focus = ItemFactory::createItem(396, 17, 0, 0, 0, 0, 0);
	focus->moveToContainer(bag);
	focus->setGumpLocation(70, 30);	

	// oil flasks
	Item* flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);
	flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);
	flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);

	backpack->assignObjId();
	backpack->moveToContainer(av);

	World::get_instance()->switchMap(av->getMapNum());

	return true;
}

bool U8Game::startInitialUsecode()
{
	SettingManager* settingman = SettingManager::get_instance();
	settingman->setDefault("skipstart", false);
	bool skipstart;
	settingman->get("skipstart", skipstart);

	if (skipstart) return true;

	GUIApp::get_instance()->setAvatarInStasis(true);

	CurrentMap* currentmap = World::get_instance()->getCurrentMap();
	UCList uclist(2);
	// (shape == 73 && quality == 36)
	//const uint8 script[] = "@%\x49\x00=*%\x24\x00=&$";
	LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL1(73), LS_Q_EQUAL(36)));
	currentmap->areaSearch(&uclist, script, sizeof(script),
						   0, 256, false, 16188, 7500);
	if (uclist.getSize() < 1) {
		perr << "Unable to find FIRST egg!" << std::endl;
		return false;
	}
	
	uint16 objid = uclist.getuint16(0);
	Egg* egg = p_dynamic_cast<Egg*>(
		ObjectManager::get_instance()->getObject(objid));
	sint32 ix, iy, iz;
	egg->getLocation(ix,iy,iz);
	// Center on egg
	CameraProcess::SetCameraProcess(new CameraProcess(ix,iy,iz));
	egg->hatch();

	// Music Egg
	// Item 2145 (class Item, shape 562, 0, (11551,2079,48) q:52, m:0, n:0, f:2000, ef:2)
	uclist.free();
	LOOPSCRIPT(musicscript, LS_SHAPE_EQUAL1(562));
	currentmap->areaSearch(&uclist, musicscript, sizeof(musicscript),
						   0, 256, false, 11551, 2079);

	if (uclist.getSize() < 1) {
		perr << "Unable to find MUSIC egg!" << std::endl;
	}
	else {
		objid = uclist.getuint16(0);
		Item *musicEgg = p_dynamic_cast<Item*>(
			ObjectManager::get_instance()->getObject(objid));

		musicEgg->callUsecodeEvent_cachein();
	}

	return true;
}


void U8Game::writeSaveInfo(ODataSource* ods)
{
	MainActor* av = World::get_instance()->getMainActor();
	sint32 x,y,z;

	std::string avname = av->getName();
	uint8 namelength = static_cast<uint8>(avname.size());
	ods->write1(namelength);
	for (unsigned int i = 0; i < namelength; ++i)
		ods->write1(static_cast<uint8>(avname[i]));

	av->getLocation(x,y,z);
	ods->write2(av->getMapNum());
	ods->write4(static_cast<uint32>(x));
	ods->write4(static_cast<uint32>(y));
	ods->write4(static_cast<uint32>(z));

	ods->write2(av->getStr());
	ods->write2(av->getInt());
	ods->write2(av->getDex());
	ods->write2(av->getHP());
	ods->write2(av->getMaxHP());
	ods->write2(av->getMana());
	ods->write2(av->getMaxMana());
	ods->write2(av->getArmourClass());
	ods->write2(av->getTotalWeight());

	for (unsigned int i = 1; i <= 6; i++)
	{
		uint16 objid = av->getEquip(i);
		Item* item = World::get_instance()->getItem(objid);
		if (item) {
			ods->write4(item->getShape());
			ods->write4(item->getFrame());
		} else {
			ods->write4(0);
			ods->write4(0);
		}
	}
}
