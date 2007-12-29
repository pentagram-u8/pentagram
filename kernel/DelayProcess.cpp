/*
 *  Copyright (C) 2004-2007  The Pentagram Team
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
#include "DelayProcess.h"

#include "IDataSource.h"
#include "ODataSource.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(DelayProcess,Process);



DelayProcess::DelayProcess(int count_)
	: Process(), count(count_)
{

}

DelayProcess::~DelayProcess()
{

}

void DelayProcess::run()
{
	if (--count == 0)
		terminate();
}

void DelayProcess::dumpInfo()
{
	Process::dumpInfo();
	pout << "Frames left: " << count << std::endl;
}


bool DelayProcess::loadData(IDataSource* ids, uint32 version)
{
	if (!Process::loadData(ids, version)) return false;
	
	count = static_cast<int>(ids->read4());
	
	return true;
}

void DelayProcess::saveData(ODataSource* ods)
{
	Process::saveData(ods);
	ods->write4(static_cast<uint32>(count));
}
