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

#include "pent_include.h"
#include "ButtonWidget.h"
#include "SimpleTextWidget.h"
#include "GUIApp.h"
#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_class stuff 
DEFINE_RUNTIME_CLASSTYPE_CODE(ButtonWidget,SimpleTextWidget);

ButtonWidget::ButtonWidget()
	: SimpleTextWidget()
{
}

ButtonWidget::ButtonWidget(int X, int Y, std::string txt, int font, int w, int h) :
	SimpleTextWidget(X,Y,txt,font,w,h)
{
}

ButtonWidget::~ButtonWidget(void)
{
}

void ButtonWidget::InitGump()
{
	SimpleTextWidget::InitGump();
}

Gump *ButtonWidget::OnMouseDown(int button, int mx, int my)
{
	Gump *ret = SimpleTextWidget::OnMouseDown(button,mx,my);
	if (ret) return ret;
	if (button == GUIApp::BUTTON_LEFT) return this;
	return 0;
}

void ButtonWidget::OnMouseUp(int button, int mx, int my)
{
	parent->ChildNotify(this,BUTTON_UP);
}

void ButtonWidget::OnMouseClick(int button, int mx, int my)
{
	parent->ChildNotify(this,BUTTON_DOWN);
}

void ButtonWidget::OnMouseDouble(int button, int mx, int my)
{
	parent->ChildNotify(this,BUTTON_DOUBLE);
}

void ButtonWidget::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	SimpleTextWidget::saveData(ods);
}

bool ButtonWidget::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!SimpleTextWidget::loadData(ids)) return false;

	return true;
}
