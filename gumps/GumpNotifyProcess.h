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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GUMPNOTIFYPROCESS_H_INCLUDED
#define GUMPNOTIFYPROCESS_H_INCLUDED

#include "Process.h"

class Gump;

class GumpNotifyProcess : public Process
{
	Gump	*gump;

public:
	ENABLE_RUNTIME_CLASSTYPE();

	GumpNotifyProcess(uint16 it);
	virtual ~GumpNotifyProcess(void);

	void setGump(Gump *g) { gump = g; }
	Gump *getGump() const { return gump; }

	virtual void notifyClosing(int res);

	virtual void terminate();

	virtual bool run(const uint32 framenum);
};

#endif //GUMPNOTIFYPROCESS_H_INCLUDED
