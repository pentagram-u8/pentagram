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

#ifndef GLOBEGG_H
#define GLOBEGG_H

#include "Item.h"

class Glob;

class GlobEgg : public Item
{
	friend class ItemFactory;
public:
	GlobEgg();
	virtual ~GlobEgg();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE();

	//! expand the contents of the glob into the main objlist
	void expand();

	//! delete the expanded contents
	//! currently, this doesn't do anything, as the contents will be
	//! deleted simply by not saving them. (in switchMap)
	void unexpand();

	//! check if the contents of this glob have changed
	//! mark any item which hasn't changed with EXT_SAVE_GLOBSKIP
	void checkContents();

	//! restore any contents with EXT_SAVE_GLOBSKIP set to their
	//! original state. Also add those items to the CurrentMap if necessary.
	void restoreContents();

	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);

	uint16 contents;
};


#endif
