/*
 *  Copyright (C) 2005  The Pentagram Team
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
#include "U8SaveGump.h"

#include "RenderSurface.h"
#include "DesktopGump.h"
#include "EditWidget.h"
#include "TextWidget.h"
#include "GUIApp.h"
#include "GameData.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "FileSystem.h"
#include "Savegame.h"
#include "PagedGump.h"

#include "IDataSource.h"
#include "ODataSource.h"

static const int entryfont = 4;

DEFINE_RUNTIME_CLASSTYPE_CODE(U8SaveGump,Gump);

U8SaveGump::U8SaveGump(bool save_, int page_)
	: Gump(0, 0, 5, 5), save(save_), page(page_)
{

}

U8SaveGump::~U8SaveGump()
{

}


// gumps: 36/0-11: number 1-12
//        46/0: "Entry"

void U8SaveGump::InitGump(Gump* newparent, bool take_focus)
{
	Gump::InitGump(newparent, take_focus);

	dims.w = 220;
	dims.h = 170;

	FrameID entry_id(GameData::GUMPS, 46, 0);
	entry_id = _TL_SHP_(entry_id);

	Shape* entryShape;
	entryShape = GameData::get_instance()->getShape(entry_id);
	ShapeFrame* sf = entryShape->getFrame(entry_id.framenum);
	int entrywidth = sf->width;
	int entryheight = sf->height;

	if (save)
		editwidgets.resize(6); // constant!

	loadDescriptions();

	for (int i = 0; i < 6; ++i) {
		int index = page * 6 + i;

		FrameID entrynum_id(GameData::GUMPS, 36, index);
		entrynum_id = _TL_SHP_(entrynum_id);

		int xbase = 3;
		int yi = i;
		if (i >= 3) {
			xbase += dims.w/2 + 9;
			yi -= 3;
		}

		Gump* gump = new Gump(xbase, 3+40*yi, 1, 1);
		gump->SetShape(entry_id, true);
		gump->InitGump(this, false);
			
		gump = new Gump(xbase+2+entrywidth, 3+40*yi, 1, 1);
		gump->SetShape(entrynum_id, true);
		gump->InitGump(this, false);

		if (index == 0) {
			// special case for 'The Beginning...' save
			Gump* widget = new TextWidget(xbase, 12+entryheight,
										  _TL_("The Beginning..."),
										  true, entryfont, 95);
			widget->InitGump(this, false);

		} else {

			if (save) {
				EditWidget* ew = new EditWidget(xbase, entryheight+4+40*yi,
												descriptions[i],
												true, entryfont,
												95, 38-entryheight, 0, true);
				ew->SetIndex(i);
				ew->InitGump(this, false);
				editwidgets[i] = ew;
			} else {
				// load
				Gump* widget = new TextWidget(xbase, entryheight+4+40*yi,
											  descriptions[i], true, entryfont,
											  95);
				widget->InitGump(this, false);
			}
		}

	}

	// remove focus from children (just in case)
	if (focus_child) focus_child->OnFocus(false);
	focus_child = 0;
}

void U8SaveGump::Close(bool no_del)
{
	Gump::Close(no_del);
}

void U8SaveGump::OnFocus(bool gain)
{
	if (gain)
	{
		if (save)
			GUIApp::get_instance()->setMouseCursor(GUIApp::MOUSE_QUILL);
		else
			GUIApp::get_instance()->setMouseCursor(GUIApp::MOUSE_MAGGLASS);
	}
}

Gump* U8SaveGump::OnMouseDown(int button, int mx, int my)
{
	// take all clicks
	return this;
}


void U8SaveGump::OnMouseClick(int button, int mx, int my)
{
	if (button != BUTTON_LEFT) return;

	ParentToGump(mx,my);

	int x;
	if (mx >= 3 && mx <= 100)
		x = 0;
	else if (mx >= dims.w/2 + 10)
		x = 1;
	else
		return;

	int y;
	if (my >= 3 && my <= 40)
		y = 0;
	else if (my >= 43 && my <= 80)
		y = 1;
	else if (my >= 83 && my <= 120)
		y = 2;
	else
		return;

	int i = 3*x + y;
	int index = 6*page + i;

	if (save && !focus_child && editwidgets[i]) {
		editwidgets[i]->MakeFocus();
		PagedGump* p = p_dynamic_cast<PagedGump*>(parent);
		if (p) p->enableButtons(false);
	}

	if (!save) {
		loadgame(index); // 'this' will be deleted here!
	}
}

void U8SaveGump::ChildNotify(Gump *child, uint32 message)
{
	if (child->IsOfType<EditWidget>() && message == EditWidget::EDIT_ENTER)
	{
		// save
		assert(save);

		EditWidget* widget = p_dynamic_cast<EditWidget*>(child);
		assert(widget);

		std::string name = widget->getText();
		if (name.empty()) return;

		if (savegame(widget->GetIndex() + 6*page, name))
			parent->Close(); // close PagedGump (and us)

		return;
	}

	if (child->IsOfType<EditWidget>() && message == EditWidget::EDIT_ESCAPE)
	{
		// cancel edit
		assert(save);

		// remove focus
		if (focus_child) focus_child->OnFocus(false);
		focus_child = 0;

		PagedGump* p = p_dynamic_cast<PagedGump*>(parent);
		if (p) p->enableButtons(true);

		EditWidget* widget = p_dynamic_cast<EditWidget*>(child);
		assert(widget);
		widget->setText(descriptions[widget->GetIndex()]);
		
		return;
	}

}

bool U8SaveGump::OnKeyDown(int key, int mod)
{
	if (Gump::OnKeyDown(key, mod)) return true;

	return false;
}

static std::string getFilename(int index) {
	char buf[32];
	sprintf(buf, "%02d", index);

	std::string filename = "@save/pent";
	filename += buf;
	filename += ".sav";
	return filename;
}

bool U8SaveGump::loadgame(int index)
{
	if (index == 0) {
		GUIApp::get_instance()->newGame();
		return true;
	}

	pout << "Load " << index << std::endl;

	std::string filename = getFilename(index);
	GUIApp::get_instance()->loadGame(filename);

	return true;
}

bool U8SaveGump::savegame(int index, const std::string& name)
{
	pout << "Save " << index << ": \"" << name << "\"" << std::endl;

	if (name.empty()) return false;

	std::string filename = getFilename(index);
	GUIApp::get_instance()->saveGame(filename, name, true);
	return true;
}

void U8SaveGump::loadDescriptions()
{
	descriptions.resize(6);

	for (int i = 0; i < 6; ++i) {
		int index = 6*page + i;

		std::string filename = getFilename(index);
		IDataSource* ids = FileSystem::get_instance()->ReadFile(filename);
		if (!ids) continue;

		Savegame* sg = new Savegame(ids);
		uint32 version = sg->getVersion();
		if (version != 1) continue; // FIXME: move version checks elsewhere!!

		descriptions[i] = sg->getDescription();
		delete sg;
	}
}
