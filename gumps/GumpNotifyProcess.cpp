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

#include "pent_include.h"
#include "GumpNotifyProcess.h"
#include "Gump.h"
#include "getObject.h"

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
	type = 0x200; // CONSTANT!
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
	if (!(flags & PROC_TERMINATED)) terminate();
}

void GumpNotifyProcess::terminate()
{
	Process::terminate();

	if (gump) {
		Gump* g = ::getGump(gump);
		assert(g);
		g->Close();
	}
}

void GumpNotifyProcess::run()
{
}

void GumpNotifyProcess::dumpInfo()
{
	Process::dumpInfo();
	pout << " gump: " << gump << std::endl;
}

void GumpNotifyProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);

	ods->write2(gump);
}

bool GumpNotifyProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;

	gump = ids->read2();

	return true;
}

// Colourless Protection
