/*
 *  Copyright (C) 2003  The Pentagram Team
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

#ifndef BARKGUMP_H_INCLUDED
#define BARKGUMP_H_INCLUDED

#include "ItemRelativeGump.h"

class BarkGump : public ItemRelativeGump
{
	std::string	barked;
	sint32 counter;
	ObjId textwidget;
public:
	ENABLE_RUNTIME_CLASSTYPE();

	BarkGump();
	BarkGump(uint16 owner, std::string msg);
	virtual ~BarkGump(void);

	// Run the gump (decrement the counter)
	virtual bool		Run(const uint32 framenum);

	// Got to the next page on mouse click
	virtual Gump *		OnMouseDown(int button, int mx, int my);

	// Init the gump, call after construction
	virtual void		InitGump();

protected:
	void NextText(); 

public:
	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif //BARKGUMP_H_INCLUDED
