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

#include "GameDetector.h"
#include "FileSystem.h"
#include "GameInfo.h"
#include "Flex.h"

bool GameDetector::detect(std::string path, GameInfo *info)
{
	FileSystem* fs = FileSystem::get_instance();
	if (!fs->AddVirtualPath("@detect", path))
		return false;

	IDataSource* ids;

	//!! this should be properly thought through sometime...
	//!! for now:

	// game type
	if (info->type == GameInfo::GAME_UNKNOWN) {

		ids = fs->ReadFile("@detect/static/u8gumps.flx"); // random U8 file
		if (ids) {
			info->type = GameInfo::GAME_U8;
			delete ids; ids = 0;
		}

	}
	// for Crusader: No Regret, we can use @detect/static/help1.bmp
	// for Crusader: No remorse, @detect/static/help1.dat

	//TODO: game version


	// game language

	// detect using eusecode/fusecode/gusecode
	if (info->language == GameInfo::GAMELANG_UNKNOWN) {
		ids = fs->ReadFile("@detect/usecode/eusecode.flx");
		if (ids) {
			// distinguish between english and spanish
			Flex* f = new Flex(ids);
			const char* buf = (const char*)(f->get_object_nodel(183));
			unsigned int size = f->get_size(183);
			for (unsigned int i = 0; i < size - 9; ++i) {
				if (strncmp(buf+i, "tableware", 9) == 0) {
					info->language = GameInfo::GAMELANG_ENGLISH;
					break;
				}
				if (strncmp(buf+i, "vajilla", 7) == 0) {
					info->language = GameInfo::GAMELANG_SPANISH;
					break;
				}
			}

			// if still unsure, English
			if (info->language == GameInfo::GAMELANG_UNKNOWN)
				info->language = GameInfo::GAMELANG_ENGLISH;

			delete f;
			delete ids; ids = 0;
		}
	}
	if (info->language == GameInfo::GAMELANG_UNKNOWN) {
		ids = fs->ReadFile("@detect/usecode/fusecode.flx");
		if (ids) {
			info->language = GameInfo::GAMELANG_FRENCH;
			delete ids; ids = 0;
		}
	}
	if (info->language == GameInfo::GAMELANG_UNKNOWN) {
		ids = fs->ReadFile("@detect/usecode/gusecode.flx");
		if (ids) {
			info->language = GameInfo::GAMELANG_GERMAN;
			delete ids; ids = 0;
		}
	}

	fs->RemoveVirtualPath("@detect");

	return (info->type != GameInfo::GAME_UNKNOWN &&
			/* info->version != 0 && */
			info->language != GameInfo::GAMELANG_UNKNOWN);
}
