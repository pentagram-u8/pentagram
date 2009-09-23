/*
Copyright (C) 2002-2005 The Pentagram team

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

#include "ConApp.h"

#include "Kernel.h"

#include "DisasmProcess.h"
#include "CompileProcess.h"

ConApp::ConApp(int argc_, const char* const* argv_)
	: CoreApp(argc_, argv_), kernel(0),
	  weAreDisasming(false), weAreCompiling(false)
	// FIXME! Need a 'console' intrinsic set instead of the 0 above!
{
	application = this;
}

void ConApp::startup()
{
	CoreApp::startup();

	kernel = new Kernel();

	if(weAreDisasming==true)
	{
		//QUIET(pout << "We Are Disassembling..." << std::endl);
		con.Print(MM_INFO, "We Are Diassembling...\n");
		kernel->addProcess(new DisasmProcess());
	}
	else if(weAreCompiling==true)
	{
		//QUIET(pout << "We Are Compiling..." << std::endl);
		con.Print(MM_INFO, "We Are Compiling...\n");
		kernel->addProcess(new CompileProcess(filesystem));
	}
	else
	{
		// we'll presume we're testing for the moment, but we really need to have a proper testing target in the future...
		con.Print(MM_INFO, "We Are Compiling...\n");
		kernel->addProcess(new CompileProcess(filesystem));
		//assert(false);
	}
}

void ConApp::DeclareArgs()
{
	CoreApp::DeclareArgs();

	parameters.declare("--disasm",  &weAreDisasming, true);
	parameters.declare("--compile", &weAreCompiling, true);
}

void ConApp::helpMe()
{
	CoreApp::helpMe();

	//TODO: some actual help :-)
	//note: arguments are already parsed at this point, so we could
	//provide more detailed help on e.g., './llc --help --disasm' 

	con.Print("\t--disasm\t- disassemble\n");
	con.Print("\t--compile\t- compile\n");
}

ConApp::~ConApp()
{

}

void ConApp::run()
{
	isRunning = true;

	while (isRunning) {
		// this needs some major changes, including possibly:
		// - handling events in-between processes?
		//   (so have a kernel->runProcess() that runs a single process)
		// - smarter painting
		//    - dirty rectangles?
		//    - more/less often depending on speed
		//    ...
		// ...

		kernel->runProcesses();
	
		if (kernel->getNumProcesses(0, 6) == 0)
			isRunning=false;

	}
}

