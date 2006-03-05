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

#include "pent_include.h"
#include "MiniStatsGump.h"

#include "GameData.h"
#include "GumpShapeArchive.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "MainActor.h"
#include "RenderSurface.h"
#include "Mouse.h"
#include "PaperdollGump.h"
#include "getObject.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(MiniStatsGump,Gump);

static const int gumpshape = 33;
static const int hpx = 6;
static const int manax = 13;
static const int bary = 19;
static const int barheight = 14;

static const uint32 hpcolour[] = { 0x980404, 0xBC0C0C, 0xD43030 };
static const uint32 manacolour[] = { 0x4050FC, 0x1C28FC, 0x0C0CCC };


MiniStatsGump::MiniStatsGump() : Gump()
{

}

MiniStatsGump::MiniStatsGump(int x, int y, uint32 _Flags, sint32 layer)
	: Gump(x, y, 5, 5, 0, _Flags, layer)
{

}

MiniStatsGump::~MiniStatsGump()
{

}

void MiniStatsGump::InitGump(Gump* newparent, bool take_focus)
{
	Gump::InitGump(newparent, take_focus);

	shape = GameData::get_instance()->getGumps()->getShape(gumpshape);
	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;
}

void MiniStatsGump::PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled)
{
	Gump::PaintThis(surf, lerp_factor, scaled);

	Actor *a = getMainActor();
	assert(a);

	sint16 maxmana = a->getMaxMana();
	sint16 mana = a->getMana();

	uint16 maxhp = a->getMaxHP();
	uint16 hp = a->getHP();

	int manaheight, hpheight;


	if (maxmana == 0)
		manaheight = 0;
	else
		manaheight = (mana * barheight) / maxmana;

	if (maxhp == 0)
		hpheight = 0;
	else
		hpheight = (hp * barheight) / maxhp;

	for (int i = 0; i < 3; ++i) {
		surf->Fill32(hpcolour[i], hpx+i, bary-hpheight+1, 1, hpheight);
		surf->Fill32(manacolour[i], manax+i, bary-manaheight+1, 1, manaheight);
	}
}

uint16 MiniStatsGump::TraceObjId(int mx, int my)
{
	uint16 objid = Gump::TraceObjId(mx, my);
	if (objid && objid != 65535) return objid;

	if (PointOnGump(mx, my)) return getObjId();

	return 0;
}

Gump* MiniStatsGump::OnMouseDown(int button, int mx, int my)
{
	if (button == BUTTON_LEFT)
		return this;

	return 0;
}

void MiniStatsGump::OnMouseDouble(int button, int mx, int my)
{
	// check if there already is an open PaperdollGump
	MainActor* av = getMainActor();
	if (!av->getGump()) {
		av->callUsecodeEvent_use();
	}

	Close();
}

void MiniStatsGump::saveData(ODataSource* ods)
{
	Gump::saveData(ods);
}

bool MiniStatsGump::loadData(IDataSource* ids, uint32 version)
{
	if (!Gump::loadData(ids, version)) return false;

	return true;
}
