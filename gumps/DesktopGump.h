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

#ifndef DESKTOPGUMP_H_INCLUDED
#define DESKTOPGUMP_H_INCLUDED

#include "Gump.h"

class DesktopGump : public Gump
{
	static bool faded_modal;
public:
	ENABLE_RUNTIME_CLASSTYPE();

	DesktopGump();
	DesktopGump(sint32 x, sint32 y, sint32 width, sint32 height);
	virtual ~DesktopGump(void);

	virtual void PaintThis(RenderSurface *surf, sint32 lerp_factor, bool scaled);
	virtual void PaintChildren(RenderSurface *surf, sint32 lerp_factor, bool scaled);

	virtual bool StartDraggingChild(Gump* gump, int mx, int my);
	virtual void DraggingChild(Gump* gump, int mx, int my);
	virtual void StopDraggingChild(Gump* gump);

	bool loadData(IDataSource* ids, uint32 version);

	void RenderSurfaceChanged(RenderSurface *surf);

	static void SetFadedModal(bool set) { faded_modal = set; }

protected:
	virtual void saveData(ODataSource* ods);
	virtual void RenderSurfaceChanged();
};

#endif
