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
}


void GameData::loadU8Data()
{
	FileSystem* filesystem = FileSystem::get_instance();

	IDataSource* uds = filesystem->ReadFile("@u8/usecode/eusecode.flx");
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
	mainshapes = new MainShapeFlex(sf);

	// Load typeflags
	IDataSource *tfs = filesystem->ReadFile("@u8/static/typeflag.dat");
	if (!tfs) {
		perr << "Unable to load static/typeflag.dat. Exiting" << std::endl;
		std::exit(-1);
	}
	mainshapes->loadTypeFlags(tfs);
	delete tfs;

	// Load globs
	IDataSource *gds = filesystem->ReadFile("@u8/static/glob.flx");
	if (!gds) {
		perr << "Unable to load static/glob.flx. Exiting" << std::endl;
		std::exit(-1);
	}
	globs = new Flex(gds);
}
