/*
 *  Copyright (C) 2004-2006  The Pentagram Team
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
#include "BookGump.h"
#include "TextWidget.h"
#include "GameData.h"
#include "Shape.h"
#include "GumpShapeArchive.h"
#include "ShapeFrame.h"
#include "UCMachine.h"
#include "GumpNotifyProcess.h"
#include "Item.h"
#include "getObject.h"


#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(BookGump,ModalGump);

// TODO: Remove all the hacks

BookGump::BookGump()
	: ModalGump()
{

}

BookGump::BookGump(ObjId owner, std::string msg) :
	ModalGump(0, 0, 100, 100, owner), text(msg)
{
}

BookGump::~BookGump(void)
{
}

void BookGump::InitGump(Gump* newparent, bool take_focus)
{
	ModalGump::InitGump(newparent, take_focus);

	// Create the TextWidgets (NOTE: they _must_ have exactly the same dims)
	TextWidget *widget = new TextWidget(9,5,text,true,9,123,129); //!! constants
	widget->InitGump(this);
	textwidgetL = widget->getObjId();
	
	widget = new TextWidget(150,5,text,true,9,123,129); //!! constants
	widget->InitGump(this);
	textwidgetR = widget->getObjId();
	widget->setupNextText();

	text.clear(); // no longer need this

	//!! constant
	Shape* shape = GameData::get_instance()->getGumps()->getShape(6);

	SetShape(shape, 0);

	ShapeFrame* sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;
}

void BookGump::NextText()
{
	TextWidget *widgetL = p_dynamic_cast<TextWidget*>(getGump(textwidgetL));
	TextWidget *widgetR = p_dynamic_cast<TextWidget*>(getGump(textwidgetR));
	assert(widgetL);
	assert(widgetR);
	if (!widgetR->setupNextText()) {
		Close();
	}
	widgetL->setupNextText();
	widgetL->setupNextText();
	widgetR->setupNextText();
}

void BookGump::OnMouseClick(int button, int mx, int my)
{
	// Scroll to next text, if possible
	NextText();
}

void BookGump::OnMouseDouble(int button, int mx, int my)
{
	Close();
}

uint32 BookGump::I_readBook(const uint8* args, unsigned int /*argsize*/)
{
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(str);
	assert(item);

	Gump *gump = new BookGump(item->getObjId(), str);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
	
	return gump->GetNotifyProcess()->getPid();
}

void BookGump::saveData(ODataSource* ods)
{
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool BookGump::loadData(IDataSource* ids, uint32 version)
{
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}
