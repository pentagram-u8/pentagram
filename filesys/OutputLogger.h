/*
Copyright (C) 2011 The Pentagram team

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

#ifndef OUTPUTLOGGER_INCLUDED
#define OUTPUTLOGGER_INCLUDED

#include <string>
#include <SDL_thread.h>

//! Class that will duplicate output sent to FILE into another file
class OutputLogger
{
	int			fd;				//< Original fd of file being redirected
	FILE*		fileOld;		//< Duplicated stream that points to the original file

	std::string	filenameLog;	//< Name of log file
	FILE*		fileLog;		//< Stream for log file

	int			fdPipeRead;		//< file descriptor to the read end of the pipe

	SDL_Thread*	pThread;		//< Pointer to the thread that reads the pipe

	int ThreadMain();	
	static int SDLCALL sThreadMain(OutputLogger *instance) { return instance->ThreadMain(); }

public:
	OutputLogger(FILE *file, const std::string &filename);	// Should be using unicode in this day and age
	~OutputLogger(void);
};


#endif // OUTPUTLOGGER_INCLUDED
