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
#include "FileSystem.h"

#include "UCMachine.h"
#include "UCProcess.h"
#include "UsecodeFlex.h"
#include "IDataSource.h"

int classid, offset; // only temporary, don't worry :-)

Application::Application(int argc, char *argv[])
{
	// Create the kernel
	kernel = new Kernel;
	ucmachine = new UCMachine;
	filesystem = new FileSystem;

	classid = offset = -1;
	if (argc == 3) {
		classid = std::strtol(argv[1], 0, 16);
		offset = std::strtol(argv[2], 0, 16);
	}

}

Application::~Application()
{
	delete kernel;
	delete ucmachine;
	delete filesystem;
}

void Application::run()
{

	IDataSource* ds = filesystem->ReadFile("eusecode.flx");
	Usecode* u = new UsecodeFlex(ds);
	UCProcess* p;
	if (classid != -1) {
		p = new UCProcess(u, classid, offset);
	} else {
		p = new UCProcess(u, 0xD0, 0x80);
// p = new UCProcess(u, 0x581, 0x28F9);
	}

    ucmachine->addProcess(p);
	uint32 framenum = 0;
	while (1) {
		kernel->runProcesses(framenum++);
	}
}