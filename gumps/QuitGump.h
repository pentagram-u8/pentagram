/*
 *  Copyright (C) 2004-2005  The Pentagram Team
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

#ifndef QUITGUMP_H
#define QUITGUMP_H

#include "ModalGump.h"

class QuitGump : public ModalGump
{
public:
	ENABLE_RUNTIME_CLASSTYPE();

	QuitGump();
	virtual ~QuitGump(void);

	virtual void InitGump(Gump* newparent, bool take_focus=true);

	virtual void PaintThis(RenderSurface*, sint32 lerp_factor, bool scaled);

	virtual bool OnKeyDown(int key, int mod);
	virtual bool OnTextInput(int unicode);
	virtual void ChildNotify(Gump *child, uint32 message);

	static void verifyQuit();
	//! "QuitGump::verifyQuit" console command
	static void ConCmd_verifyQuit(const Console::ArgvType &argv);

	bool loadData(IDataSource* ids);
protected:
	virtual void saveData(ODataSource* ods);
	ObjId yesWidget, noWidget;
};

#endif
