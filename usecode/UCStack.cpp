/*
Copyright (C) 2003-2004 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "pent_include.h"

#include "UCStack.h"
#include "IDataSource.h"
#include "ODataSource.h"

void UCStack::save(ODataSource* ods)
{
	ods->write4(size);
	ods->write4(getSP());

	ods->write(buf_ptr, stacksize());
}

bool UCStack::load(IDataSource* ids, uint32 version)
{
	size = ids->read4();
#ifdef USE_DYNAMIC_UCSTACK
	if (buf) delete[] buf;
	buf = new uint8[size];
#else
	if (size > sizeof(buf_array)) 
	{
		perr << "Error: UCStack size mismatch (buf_array too small)" << std::endl;
		return false;
	}
	buf = buf_array;
#endif
	uint32 sp = ids->read4();
	buf_ptr = buf + sp;

	ids->read(buf_ptr, size - sp);

	return true;
}
