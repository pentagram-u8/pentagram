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

#include "GameData.h"
#include "FileSystem.h"
#include "IDataSource.h"
#include "UsecodeFlex.h"
#include "MainShapeFlex.h"
#include "FontShapeFlex.h"
#include "GumpShapeFlex.h"
#include "Flex.h"
#include "Glob.h"
#include "PaletteManager.h"
#include "Shape.h"
#include "MusicFlex.h"
#include "WpnOvlayDat.h"
#include "CoreApp.h"
#include "ConfigFileManager.h"

GameData* GameData::gamedata = 0;


GameData::GameData()
	: fixed(0), mainshapes(0), mainusecode(0), globs(0), fonts(0), gumps(0),
	  mouse(0), music(0), weaponoverlay(0)
{
	assert(gamedata == 0);
	gamedata = this;
}

GameData::~GameData()
{
	delete fixed;
	fixed = 0;

	delete mainshapes;
	mainshapes = 0;

	delete mainusecode;
	mainusecode = 0;

	for (unsigned int i = 0; i < globs.size(); ++i)
		delete globs[i];
	globs.clear();

	delete fonts;
	fonts = 0;

	delete gumps;
	gumps = 0;

	delete mouse;
	mouse = 0;

	delete music;
	music = 0;

	delete weaponoverlay;
	weaponoverlay = 0;

	gamedata = 0;
}

Glob* GameData::getGlob(uint32 glob) const
{
	if (glob < globs.size())
		return globs[glob];
	else
		return 0;
}

ShapeFlex* GameData::getShapeFlex(uint16 flexId) const
{
	switch (flexId) {
		case MAINSHAPES:
			return mainshapes;
		case GUMPS:
			return gumps;
		default:
			break;
	};
	return 0;
}

Shape* GameData::getShape(FrameID f) const
{
	ShapeFlex* sf = getShapeFlex(f.flexid);
	if (!sf) return 0;
	Shape* shape = sf->getShape(f.shapenum);
	return shape;
}

ShapeFrame* GameData::getFrame(FrameID f) const
{
	Shape* shape = getShape(f);
	if (!shape) return 0;
	ShapeFrame* frame = shape->getFrame(f.framenum);
	return frame;
}

void GameData::loadTranslation()
{
	ConfigFileManager* config = ConfigFileManager::get_instance();
	std::string translationfile;

	if (gameinfo.type == GameInfo::GAME_U8) {
		switch (gameinfo.language) {
		case GameInfo::GAMELANG_ENGLISH:
			// default. Don't need to do anything
			break;
		case GameInfo::GAMELANG_FRENCH:
			translationfile = "u8french.ini";
			break;
		case GameInfo::GAMELANG_GERMAN:
			translationfile = "u8german.ini";
			break;
		case GameInfo::GAMELANG_SPANISH:
			translationfile = "u8spanish.ini";
			break;
		default:
			perr << "Unknown language." << std::endl;
			break;
		}

		if (!translationfile.empty()) {
			translationfile = "@data/" + translationfile;

			pout << "Loading translation: " << translationfile << std::endl;

			config->readConfigFile(translationfile, "language", true);
		}
	}
}

std::string GameData::translate(std::string text)
{
	// TODO: maybe cache these lookups? config calls may be expensive

	ConfigFileManager* config = ConfigFileManager::get_instance();
	Pentagram::istring key = "language/text/" + text;
	if (!config->exists(key))
		return text;

	std::string trans;
	config->get(key, trans);
	return trans;
}

FrameID GameData::translate(FrameID f)
{
	// TODO: maybe cache these lookups? config calls may be expensive
	// TODO: add any non-gump shapes when applicable
	// TODO: allow translations to be in another shapeflex

	ConfigFileManager* config = ConfigFileManager::get_instance();
	Pentagram::istring key = "language/";
	switch (f.flexid) {
	case GUMPS:
		key += "gumps/";
		break;
	default:
		return f;
	}

	char buf[100];
	sprintf(buf, "%d,%d", f.shapenum, f.framenum);

	key += buf;
	if (!config->exists(key))
		return f;

	std::string trans;
	config->get(key, trans);

	FrameID t;
	t.flexid = f.flexid;
	int n = sscanf(trans.c_str(), "%d,%d", &t.shapenum, &t.framenum);
	if (n != 2) {
		perr << "Invalid shape translation: " << trans << std::endl;
		return f;
	}

	return t;
}


void GameData::loadU8Data()
{
	FileSystem* filesystem = FileSystem::get_instance();

	IDataSource *fd = filesystem->ReadFile("@u8/static/fixed.dat");
	if (!fd) {
		perr << "Unable to load static/fixed.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	fixed = new Flex(fd);

	IDataSource* uds = filesystem->ReadFile("@u8/usecode/eusecode.flx");

	//!! hack alert
	if (!uds)
		uds = filesystem->ReadFile("@u8/usecode/gusecode.flx");
	if (!uds)
		uds = filesystem->ReadFile("@u8/usecode/fusecode.flx");

	if (!uds) {
		perr << "Unable to load usecode/eusecode.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	mainusecode = new UsecodeFlex(uds);

	// Load main shapes
	pout << "Load Shapes" << std::endl;
	IDataSource *sf = filesystem->ReadFile("@u8/static/u8shapes.flx");
	if (!sf) {
		perr << "Unable to load static/u8shapes.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes = new MainShapeFlex(sf, MAINSHAPES,
		PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	// Load weapon, armour info
	ConfigFileManager* config = ConfigFileManager::get_instance();
	config->readConfigFile("@data/u8weapons.ini", "weapons", true);
	config->readConfigFile("@data/u8armour.ini", "armour", true);
	config->readConfigFile("@data/u8monsters.ini", "monsters", true);
	config->readConfigFile("@data/u8.ini", "game", true);

	// Load typeflags
	IDataSource *tfs = filesystem->ReadFile("@u8/static/typeflag.dat");
	if (!tfs) {
		perr << "Unable to load static/typeflag.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadTypeFlags(tfs);
	delete tfs;

	// Load animdat
	IDataSource *af = filesystem->ReadFile("@u8/static/anim.dat");
	if (!af) {
		perr << "Unable to load static/anim.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadAnimDat(af);
	delete af;

	// Load weapon overlay data
	IDataSource* wod = filesystem->ReadFile("@u8/static/wpnovlay.dat");
	if (!wod) {
		perr << "Unable to load static/wpnovlay.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	Flex* overlayflex = new Flex(wod);
	weaponoverlay = new WpnOvlayDat();
	weaponoverlay->load(overlayflex);
	delete overlayflex;

	// Load globs
	IDataSource *gds = filesystem->ReadFile("@u8/static/glob.flx");
	if (!gds) {
		perr << "Unable to load static/glob.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	Flex* globflex = new Flex(gds);
	globs.clear();
	globs.resize(globflex->get_count());
	for (unsigned int i = 0; i < globflex->get_count(); ++i) {
		Glob* glob = 0;
		IDataSource* globds = globflex->get_datasource(i);

		if (globds && globds->getSize()) {
			glob = new Glob;
			glob->read(globds);
		}
		delete globds;

		globs[i] = glob;
	}
	delete globflex;

	// Load fonts
	IDataSource *fds = filesystem->ReadFile("@u8/static/u8fonts.flx");
	if (!fds) {
		perr << "Unable to load static/u8fonts.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	fonts = new FontShapeFlex(fds, OTHER,
		PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	// Load mouse
	IDataSource *msds = filesystem->ReadFile("@u8/static/u8mouse.shp");
	if (!msds) {
		perr << "Unable to load static/u8mouse.shp. Exiting" << std::endl;
		std::exit(-1);
	}
	mouse = new Shape(msds, 0);
	mouse->setPalette(PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));
	delete msds;

	IDataSource *gumpds = filesystem->ReadFile("@u8/static/u8gumps.flx");
	if (!gumpds) {
		perr << "Unable to load static/u8gumps.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	gumps = new GumpShapeFlex(gumpds, GUMPS,
		PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game));

	IDataSource *gumpageds = filesystem->ReadFile("@u8/static/gumpage.dat");
	if (!gumpageds) {
		perr << "Unable to load static/gumpage.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	gumps->loadGumpage(gumpageds);
	delete gumpageds;


	IDataSource *mf = filesystem->ReadFile("@u8/sound/music.flx");
	if (!mf) {
		perr << "Unable to load sound/music.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	music = new MusicFlex(mf);

	loadTranslation();
}
