/*
 *  Copyright (C) 2004  The Pentagram Team
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

#ifndef CONTROLSGUMP_H
#define CONTROLSGUMP_H

#include "ModalGump.h"

class ControlsGump : public ModalGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	ControlsGump();
	virtual ~ControlsGump(void);

	// Init the gump, call after construction
	virtual void InitGump();

	// Paint the Gump
	virtual void PaintThis(RenderSurface*, sint32 lerp_factor);

	virtual void ChildNotify(Gump *child, uint32 message);
	virtual bool OnKeyDown(int key, int mod);

	static void showMenu();

	bool loadData(IDataSource* ids);
protected:
	void addEntry(char * binding, char * name, int & x, int & y);
	virtual void saveData(ODataSource* ods);
	typedef std::pair<ObjId, Pentagram::istring> entryPair;
	std::vector<entryPair> entries;
};


#endif
