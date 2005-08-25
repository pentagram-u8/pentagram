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
#include "pent_include.h"
#include "SlidingWidget.h"
#include "ShapeFrame.h"
#include "Shape.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(SlidingWidget,Gump);

SlidingWidget::SlidingWidget()
	: Gump()
{
}

SlidingWidget::SlidingWidget(int x, int y, Shape* shape_, uint32 framenum_)
	: Gump(x, y, 5, 5, 0, FLAG_DRAGGABLE)
{
	shape = shape_;
	framenum = framenum_;
}

SlidingWidget::~SlidingWidget()
{
}

void SlidingWidget::InitGump(Gump* newparent, bool take_focus)
{
	Gump::InitGump(newparent, take_focus);

	ShapeFrame* sf = shape->getFrame(framenum);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;
}

uint16 SlidingWidget::TraceObjId(int mx, int my)
{
	if (PointOnGump(mx, my))
		return getObjId();
	else
		return 0;
}

void SlidingWidget::saveData(ODataSource* ods)
{
	Gump::saveData(ods);
}

bool SlidingWidget::loadData(IDataSource* ids, uint32 version)
{
	if (!Gump::loadData(ids, version)) return false;

	return true;
}
