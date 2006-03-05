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

#ifndef SLIDERGUMP_H
#define SLIDERGUMP_H

#include "ModalGump.h"

class UCProcess;
class RenderedText;

class SliderGump : public ModalGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	SliderGump();
	SliderGump(int x, int y, sint16 min, sint16 max,
			   sint16 value, sint16 delta=1);
	virtual ~SliderGump(void);

	virtual void InitGump(Gump* newparent, bool take_focus=true);
	virtual void PaintThis(RenderSurface* surf, sint32 lerp_factor, bool scaled);
	virtual void Close(bool no_del = false);	
	virtual void ChildNotify(Gump *child, uint32 message);

	void setUsecodeNotify(UCProcess* ucp);

	// Dragging
	virtual bool StartDraggingChild(Gump* gump, int mx, int my);
	virtual void DraggingChild(Gump* gump, int mx, int my);
	virtual void StopDraggingChild(Gump* gump);

	virtual bool OnKeyDown(int key, int mod);

	bool loadData(IDataSource* ids, uint32 version);
protected:
	virtual void saveData(ODataSource* ods);

	sint16 min;
	sint16 max;
	sint16 delta;
	sint16 value;

	uint16 usecodeNotifyPID;

	sint16 renderedvalue;
	RenderedText* renderedtext;
	
	int getSliderPos();
	void setSliderPos();
	void setValueFromSlider(int sliderx);
	void drawText(RenderSurface* surf);
};

#endif
