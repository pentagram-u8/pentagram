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
#include "ScrollGump.h"
#include "TextWidget.h"
#include "GUIApp.h"
#include "GameData.h"
#include "Shape.h"
#include "GumpShapeArchive.h"
#include "ShapeFrame.h"
#include "UCMachine.h"
#include "GumpNotifyProcess.h"
#include "Item.h"
#include "ObjectManager.h"


#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ScrollGump,ModalGump);

// TODO: Remove all the hacks

ScrollGump::ScrollGump()
	: ModalGump()
{

}

ScrollGump::ScrollGump(ObjId owner, std::string msg) :
	ModalGump(0, 0, 100, 100, owner), text(msg)
{
}

ScrollGump::~ScrollGump(void)
{
}

void ScrollGump::InitGump()
{
	ModalGump::InitGump();

	// Create the TextWidget
	Gump *widget = new TextWidget(25,25,text,9,196,125); //!! constants
	widget->InitGump();

	textwidget = widget->getObjId();

	// Add it to us
	AddChild(widget);

	text.clear(); // no longer need this

	Shape* shape = GameData::get_instance()->getGumps()->getShape(19);

	SetShape(shape, 0);

	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;
}

void ScrollGump::NextText()
{
	TextWidget *widget = p_dynamic_cast<TextWidget*>
		(GUIApp::get_instance()->getGump(textwidget));
	assert(widget);
	if (!widget->setupNextText()) {
		Close();
	}
}

bool ScrollGump::Run(const uint32 framenum)
{
	ModalGump::Run(framenum);

	return true;	// Always repaint, even though we really could just try to detect it
}

void ScrollGump::OnMouseClick(int button, int mx, int my)
{
	// Scroll to next text, if possible
	NextText();
}

void ScrollGump::OnMouseDouble(int button, int mx, int my)
{
	Close();
}

uint32 ScrollGump::I_readScroll(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(str);
	assert(item);

	GUIApp *app = p_dynamic_cast<GUIApp*>(GUIApp::get_instance());
	assert(app);

	Gump *desktop = app->getDesktopGump();
	Gump *gump = new ScrollGump(item->getObjId(), str);
	gump->InitGump();
	desktop->AddChild(gump);
	gump->setRelativePosition(CENTER);
	
	return gump->GetNotifyProcess()->getPid();
}

void ScrollGump::saveData(ODataSource* ods)
{
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool ScrollGump::loadData(IDataSource* ids, uint32 version)
{
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}

