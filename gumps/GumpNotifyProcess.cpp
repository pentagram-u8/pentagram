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

#include "pent_include.h"
#include "GumpNotifyProcess.h"
#include "Gump.h"

GumpNotifyProcess::GumpNotifyProcess(void) : Process(), gump(0)
{
	result = 0;
}

GumpNotifyProcess::~GumpNotifyProcess(void)
{
}

void GumpNotifyProcess::notifyClosing(int res)
{
	gump = 0;
	result = res;
	terminate();
}

void GumpNotifyProcess::terminate()
{
	if (gump) gump->Close();
	Process::terminate();
}

bool GumpNotifyProcess::run(const uint32)
{
	return false;
}

// Colourless Protection
