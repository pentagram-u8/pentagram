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

#include "GameData.h"
#include "FileSystem.h"
#include "IDataSource.h"
#include "UsecodeFlex.h"
#include "MainShapeFlex.h"
#include "Flex.h"
#include "Glob.h"

GameData* GameData::gamedata = 0;


GameData::GameData()
	: mainshapes(0), mainusecode(0), globs(0)
{
	assert(gamedata == 0);
	gamedata = this;
}

GameData::~GameData()
{
	gamedata = 0;

	globs.clear();

	delete mainusecode;
	mainusecode = 0;

	delete mainshapes;
	mainshapes = 0;
}

Glob* GameData::getGlob(uint32 glob) const
{
	if (glob < globs.size())
		return globs[glob];
	else
		return 0;
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
	//! we're leaking fd here

	IDataSource* uds = filesystem->ReadFile("@u8/usecode/eusecode.flx");
	if (!uds) {
		perr << "Unable to load usecode/eusecode.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	mainusecode = new UsecodeFlex(uds);
	//! we're leaking uds here

	// Load main shapes
	pout << "Load Shapes" << std::endl;
	IDataSource *sf = filesystem->ReadFile("@u8/static/u8shapes.flx");
	if (!sf) {
		perr << "Unable to load static/u8shapes.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes = new MainShapeFlex(sf);

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
			glob->read(globflex->get_datasource(i));
		}
		delete globds;

		globs[i] = glob;
	}
	delete globflex;
}
