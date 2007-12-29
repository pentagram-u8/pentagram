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

#ifndef CREATEITEMPROCESS_H
#define CREATEITEMPROCESS_H

#include "Process.h"

class CreateItemProcess : public Process
{
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();

	CreateItemProcess();
	CreateItemProcess(uint32 shape, uint32 frame, uint16 quality,
					  uint16 flags, uint16 npcnum, uint16 mapnum,
					  uint32 extendedflags, sint32 x, sint32 y, sint32 z);
	virtual ~CreateItemProcess(void);

	virtual void run();

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	uint32 shape;
	uint32 frame;
	uint16 quality;
	uint16 flags;
	uint16 npcnum;
	uint16 mapnum;
	uint32 extendedflags;
	sint32 x,y,z;
};

#endif
