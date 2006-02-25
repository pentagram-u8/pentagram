/*
Copyright (C) 2005 The Pentagram team

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
#include "FixedWidthFont.h"

#include "Texture.h"
#include "IDataSource.h"
#include "ConfigFileManager.h"
#include "FileSystem.h"

FixedWidthFont * FixedWidthFont::Create(std::string iniroot)
{
	ConfigFileManager *config = ConfigFileManager::get_instance();
	FileSystem *filesys = FileSystem::get_instance();

	std::string	filename;
	if (!config->get(iniroot + "/font/path",filename)) {
		perr << "Error: 'path' key not found in font ini" << std::endl;
		return 0;
	}

	IDataSource *ds = filesys->ReadFile(filename);

	if (!ds) {
		perr << "Error: Unable to open file " << filename << std::endl;
		return 0;
	}

	Texture *fonttex = Texture::Create(ds, filename.c_str());

	if (!fonttex)
	{
		perr << "Error: Unable to read texture " << filename << std::endl;
		return 0;
	}

	delete ds;

	FixedWidthFont *fwf = new FixedWidthFont;

	fwf->tex = fonttex;

	if (!config->get(iniroot + "/font/width",fwf->width)) {
		fwf->width = fwf->tex->width / 16;
	}
	
	if (!config->get(iniroot + "/font/height",fwf->height)) {
		fwf->height = fwf->tex->height / 16;
	}

	if (!config->get(iniroot + "/font/align_x",fwf->align_x)) {
		for (int i = 0; i < 32; i++) {
			if (fwf->width <= (1 << i)) {
				fwf->align_x = 1 << i;
				break;
			}
		}
	}

	if (!config->get(iniroot + "/font/align_y",fwf->align_y)) {
		for (int i = 0; i < 32; i++) {
			if (fwf->height <= (1 << i)) {
				fwf->align_y = 1 << i;
				break;
			}
		}
	}

	return fwf;
}
