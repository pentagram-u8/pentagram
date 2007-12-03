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

#ifndef MESSAGEBOXGUMP_H
#define MESSAGEBOXGUMP_H

#include "ModalGump.h"
#include "intrinsics.h"

#include <vector>

class MessageBoxGump : public ModalGump
{
	std::string	title;
	std::string	message;
	std::vector<std::string> buttons; 
	int title_colour;
public:
	ENABLE_RUNTIME_CLASSTYPE();

	MessageBoxGump();
	MessageBoxGump(const std::string &title, const std::string &message, uint32 title_colour, std::vector<std::string> *buttons);
	virtual ~MessageBoxGump();

	// Init the gump, call after construction
	virtual void InitGump(Gump* newparent, bool take_focus=true);
	virtual void Close(bool no_del = false);

	bool loadData(IDataSource* ids, uint32 version);

	virtual void PaintThis(RenderSurface*, sint32 lerp_factor, bool scaled);

	//! Create a Message Box
	//! \param title Title of the message box 
	//! \param message Message to be displayed in box
	//! \param title_colour The colour to be displayed behind the title bar
	//! \param buttons Array of button names to be displayed. Default is "Ok"
	//! \return Pid of process that will have the result when finished
	static ProcId Show(std::string title, std::string message, uint32 title_colour=0xFF30308F, std::vector<std::string> *buttons=0);
	static ProcId Show(std::string title, std::string message, std::vector<std::string> *buttons) { return Show(title,message,0xFF30308F,buttons); }

	virtual void ChildNotify(Gump *child, uint32 message);

protected:
	virtual void saveData(ODataSource* ods);
};

#endif
