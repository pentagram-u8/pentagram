/*
 *  Copyright (C) 2003-2004  The Pentagram Team
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

#ifndef ASKGUMP_H_INCLUDED
#define ASKGUMP_H_INCLUDED

#include "ItemRelativeGump.h"

class UCList;

class AskGump : public ItemRelativeGump
{
	UCList *answers;
public:
	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE();

	AskGump();
	AskGump(uint16 owner, UCList *answers);
	virtual	~AskGump();

	// Init the gump, call after construction
	virtual void		InitGump(Gump* newparent, bool take_focus=true);

	virtual void		ChildNotify(Gump *child, uint32 message);

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif // ASKGUMP_H_INCLUDED
