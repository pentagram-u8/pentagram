/*
 *  Copyright (C) 2003-2005  The Pentagram Team
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

#ifndef SLIDINGWIDGET_H
#define SLIDINGWIDGET_H

#include "Gump.h"

class SlidingWidget : public Gump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	SlidingWidget();
	SlidingWidget(int x, int y, Shape* shape, uint32 framenum);
	virtual ~SlidingWidget();

	virtual void InitGump(Gump* newparent, bool take_focus=true);
	virtual uint16 TraceObjId(int mx, int my);

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);
};

#endif
