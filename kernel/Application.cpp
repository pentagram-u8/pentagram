/*
Copyright (C) 2002 The Pentagram team

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

#include "Application.h"
#include "Kernel.h"

#include "UCProcess.h"
#include "Usecode.h"

Application::Application(int argc, char *argv[])
{
	// Create the kernel
	kernel = new Kernel;
}

Application::~Application()
{
	delete kernel;
}

void Application::run()
{
	Usecode* u = new Usecode(0);
	UCProcess* p = new UCProcess(u, 0, 0);
	kernel->addProcess(p);
	uint32 framenum = 0;
	while (1) {
		kernel->runProcesses(framenum++);
	}
}
