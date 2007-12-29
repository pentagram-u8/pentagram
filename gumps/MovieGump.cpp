/*
 *  Copyright (C) 2004-2005  The Pentagram Team
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
#include "MovieGump.h"

#include "RawArchive.h"
#include "SKFPlayer.h"
#include "GUIApp.h"
#include "DesktopGump.h"
#include "GumpNotifyProcess.h"

#include "FileSystem.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MovieGump,ModalGump);

MovieGump::MovieGump()
	: ModalGump(), player(0)
{

}

MovieGump::MovieGump(int width, int height, RawArchive* movie,
					 bool introMusicHack, uint32 _Flags, sint32 layer)
	: ModalGump(50, 50, width, height, 0, _Flags, layer)
{
	player = new SKFPlayer(movie, width, height, introMusicHack);
}

MovieGump::~MovieGump()
{
	delete player;
}

void MovieGump::InitGump(Gump* newparent, bool take_focus)
{
	ModalGump::InitGump(newparent, take_focus);
	player->start();

	GUIApp::get_instance()->pushMouseCursor();
	GUIApp::get_instance()->setMouseCursor(GUIApp::MOUSE_NONE);
}

void MovieGump::Close(bool no_del)
{
	GUIApp::get_instance()->popMouseCursor();

	ModalGump::Close(no_del);
}

void MovieGump::run()
{
	ModalGump::run();

	player->run();
	if (!player->isPlaying()) {
		Close();
	}
}

void MovieGump::PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled)
{
	player->paint(surf, lerp_factor);
}

bool MovieGump::OnKeyDown(int key, int mod)
{
	switch(key)
	{
	case SDLK_ESCAPE:
	{
		Close();
	} break;
	default:
		break;
	}

	return true;
}

//static
ProcId MovieGump::U8MovieViewer(RawArchive* movie, bool introMusicHack)
{
	Gump* gump = new MovieGump(320, 200, movie, introMusicHack);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
	gump->CreateNotifier();
	return gump->GetNotifyProcess()->getPid();
}

//static
void MovieGump::ConCmd_play(const Console::ArgvType &argv)
{
	if (argv.size() != 2) {
		pout << "play usage: play <moviename>" << std::endl;
		return;
	}

	std::string filename = "@game/static/" + argv[1] + ".skf";
	FileSystem* filesys = FileSystem::get_instance();
	IDataSource* skf = filesys->ReadFile(filename);
	if (!skf) {
		pout << "movie not found." << std::endl;
		return;
	}

	RawArchive* flex = new RawArchive(skf);
	U8MovieViewer(flex);
}


bool MovieGump::loadData(IDataSource* ids)
{
	return false;
}

void MovieGump::saveData(ODataSource* ods)
{

}
