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
#include "GumpNotifyProcess.h"
#include "Gump.h"
#include "GUIApp.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(GumpNotifyProcess,Process);

GumpNotifyProcess::GumpNotifyProcess()
	: Process()
{

}

GumpNotifyProcess::GumpNotifyProcess(uint16 it) : Process(it), gump(0)
{
	result = 0;
}

GumpNotifyProcess::~GumpNotifyProcess(void)
{
}

void GumpNotifyProcess::setGump(Gump *g)
{
	gump = g->getObjId();
}


void GumpNotifyProcess::notifyClosing(int res)
{
	gump = 0;
	result = res;
	if (!terminated) terminate();
}

void GumpNotifyProcess::terminate()
{
	Process::terminate();

	if (gump) {
		Gump* g = GUIApp::get_instance()->getGump(gump);
		g->Close();
	}
}

bool GumpNotifyProcess::run(const uint32)
{
	return false;
}

void GumpNotifyProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	ods->write2(gump);
}

bool GumpNotifyProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	gump = ids->read2();

	return true;
}

// Colourless Protection
