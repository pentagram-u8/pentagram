/*
Copyright (C) 2003 The Pentagram team

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

#include "CompileProcess.h"
#include "Application.h"

// p_dynamic_cast stuff
DEFINE_DYNAMIC_CAST_CODE(CompileProcess,Process);

CompileProcess::CompileProcess() : termCounter(120)
{
}


CompileProcess::~CompileProcess()
{
}

bool CompileProcess::run(const uint32 /*framenum*/)
{
	if (suspended)
		return false;

	if(termCounter==0)
		Application::application->ForceQuit();

	//pout << termCounter << std::endl;
	termCounter--;

	// if we need to redraw the screen (aka, we've done something), we need to return true;
	return false;
}
