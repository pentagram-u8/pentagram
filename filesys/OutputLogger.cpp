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

#include "pent_include.h"
#include "OutputLogger.h"

#include <cstdio>
#include <unistd.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#else
#define _pipe(pipefd,bufsize,flags) pipe(pipefd)
#define _fdopen fdopen
#define _dup dup
#define _dup2 dup2
#define _fileno fileno
#define _read read
#define _close close
#define _O_BINARY 0
#endif

OutputLogger::OutputLogger(FILE *file, const std::string &filename) :
	fd(-1), fileOld(NULL), filenameLog(filename), fileLog(NULL), 	
	fdPipeRead(-1), pThread(NULL)
{
	// Get the underlying file descriptor for the FILE stream
	fd = _fileno(file);

	// Probably on windows and there is no stdout/stderr in Subsystem Windows
	if (fd < 0) {
		fileLog = freopen(filenameLog.c_str(),"w",file);
#ifdef WIN32
		DWORD stdhandle = 0;
		if (fileLog == stdout) 
			stdhandle = STD_OUTPUT_HANDLE;
		else if (fileLog == stderr) 
			stdhandle = STD_ERROR_HANDLE;
		if (stdhandle) 
			SetStdHandle(stdhandle,(HANDLE)_get_osfhandle(_fileno(fileLog)));
#endif
		return;
	}

	// Create the pipe
	int fdsPipe[2];
	if (_pipe(fdsPipe,256,_O_BINARY) != 0) {
		return;
	}
	fdPipeRead = fdsPipe[0];
	int fdPipeWrite = fdsPipe[1];

	// Make sure that buffering is turned off on the incoming file
	std::setvbuf(file,NULL,_IONBF,0);

	// Duplicate the fd and create new filestream for it
	fileOld = _fdopen(_dup(fd),"w");
	// If the file was stderr, make sure the new file doesn't have buffering
	if (fileOld && file == stderr) std::setvbuf(fileOld,NULL,_IONBF,0);

	// Create the output log file
	fileLog =  std::fopen(filenameLog.c_str(),"w");

	// Duplicate pipe write file descriptor and replace original
	_dup2(fdPipeWrite,fd);
	
	// Close the fd for the write end of the pipe
	_close(fdPipeWrite);
	fdPipeWrite = -1;

	// Create the thread
	pThread = SDL_CreateThread( (int (SDLCALL*)(void*)) &OutputLogger::sThreadMain,this);
}

OutputLogger::~OutputLogger(void)
{
	// Replace fd with it's original details
	if (fileOld && fd >= 0) {
		_dup2(_fileno(fileOld),fd);
		fd = -1;
	}

	if (pThread) {
		// Wait for the thread to exit (the write handle is now invalid so it will have broken the pipe)
		SDL_WaitThread(pThread,NULL);
		pThread = NULL;
	}

	// Close the read end of the pipe, completely closing the pipe
	if (fdPipeRead >= 0) {
		_close(fdPipeRead);
		fdPipeRead = -1;
	}

	// Close the duplicated original file
	if (fileOld) {
		std::fclose(fileOld);
		fileOld = NULL;
	}

	// Close the log file
	if (fileLog) {

		// In the event that we did a freopen on the handle remove the windows
		// output std handle
#ifdef WIN32
		DWORD stdhandle = 0;
		if (fileLog == stdout) 
			stdhandle = STD_OUTPUT_HANDLE;
		else if (fileLog == stderr) 
			stdhandle = STD_ERROR_HANDLE;
		if (stdhandle) 
			SetStdHandle(stdhandle,NULL);
#endif

		// Get the size written to the log file
		int sizeLog = std::ftell(fileLog);

		std::fclose(fileLog);
		fileLog = NULL;

		// Delete the log file if it was empty
		if (!sizeLog) std::remove(filenameLog.c_str());
		filenameLog = std::string();
	}

}

int OutputLogger::ThreadMain()
{
	char buffer[4096];
	int numread;

	// Read from the pipe (till its broken)
	while ((numread = _read( fdPipeRead, buffer, 256)) > 0) { 
		// Write to the duplicated original file stream
		if (fileOld) {
			fwrite(buffer, 1, numread, fileOld);
			if (fd == 1) {
				for (int i = 0; i < numread; i++) {
					if (buffer[i] == '\n') {
						fflush(fileOld);
						break;
					}
				}
			}
		}
		// And write to the log file
		if (fileLog) {
			fwrite(buffer, 1, numread, fileLog);
		}
	} 
	return 1;
}

