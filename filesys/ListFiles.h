/*
Copyright (C) 2001 The Exult Team
Copyright (C) 2003 The Pentagram Team

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

#ifndef LISTFILES_H
#define LISTFILES_H

#include <vector>
#include <string>
#include <iostream>


// System Specific Code for Windows
#if defined(WIN32)

// Need this for _findfirst, _findnext, _findclose
#include <windows.h>
#include <malloc.h>
#include <tchar.h>

int FileSystem::ListFiles(const std::string mask, FileList& files)
{
	std::string		path(mask);
	const TCHAR		*lpszT;
	WIN32_FIND_DATA	fileinfo;
	HANDLE			handle;
	char			*stripped_path;
	int				i, nLen, nLen2;

	if(path[0]=='@')
		rewrite_virtual_path(path);

#ifdef UNICODE
	const char *name = path.c_str();
	nLen = strlen(name)+1;
	LPTSTR lpszT2 = (LPTSTR) _alloca(nLen*2);
	lpszT = lpszT2;
	MultiByteToWideChar(CP_ACP, 0, name, -1, lpszT2, nLen);
#else
	lpszT = path.c_str();
#endif

	handle = FindFirstFile (lpszT, &fileinfo);

	stripped_path = new char [path.length()+1];
	std::strcpy (stripped_path, path.c_str());

	for (i = std::strlen (stripped_path)-1; i; i--)
		if (stripped_path[i] == '\\' || stripped_path[i] == '/')
			break;

	if (stripped_path[i] == '\\' || stripped_path[i] == '/')
		stripped_path[i+1] = 0;


#ifdef DEBUG
	perr << "FileSystem::ListFiles(): " << mask << " = " << path << std::endl;
#endif

	// Now search the files
	if (handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			nLen = std::strlen(stripped_path);
			nLen2 = _tcslen (fileinfo.cFileName)+1;
			char *filename = new char [nLen+nLen2];
			strcpy (filename, stripped_path);
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, fileinfo.cFileName, -1, filename+nLen, nLen2, NULL, NULL);
#else
			std::strcat (filename, fileinfo.cFileName);
#endif

			files.push_back(filename);
#ifdef DEBUG
			perr << filename << std::endl;
#endif
			delete [] filename;
		} while (FindNextFile( handle, &fileinfo ));
	}

	if (GetLastError() != ERROR_NO_MORE_FILES) {
		LPTSTR lpMsgBuf;
		char* str;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
#ifdef UNICODE
		nLen2 = _tcslen (lpMsgBuf) + 1;
		str = (char*) _alloca(nLen);
		WideCharToMultiByte(CP_ACP, 0, lpMsgBuf, -1, str, nLen2, NULL, NULL);
#else
		str = lpMsgBuf;
#endif
		perr << "FileSystem::ListFiles(): Error while listing files: " << str << std::endl;
		LocalFree( lpMsgBuf );
	}

#ifdef DEBUG
	perr << files.size() << " filenames" << std::endl;
#endif

	delete [] stripped_path;
	FindClose (handle);
	return 0;
}

#else	// This system has glob.h

#include <glob.h>

int FileSystem::ListFiles(const std::string mask, FileList& files)

{
	glob_t globres;
	std::string name(mask);
	
	// get the 'root' (@u8 or whatever) stub
	const std::string rootpath(name.substr(0, name.find('/')));

	// munge the path to a 'real' one.
	if (!rewrite_virtual_path(name)) {
		perr << "Warning: FileSystem sandbox violation when accessing:"
			 << std::endl << "\t" << mask << std::endl;
		return -1;
	}
	
#if 0
	pout << "Root: " << rootpath << std::endl;
	pout << name << "\t" << name.size() << std::endl;
	pout << mask << '\t' << mask.size() << std::endl;
#endif
	
	// calculate the difference in length between the real path, the 
	// original path, and the root @whatever
	uint32 newplen = name.size() - mask.size() + rootpath.size();
	
	int err = glob(name.c_str(), GLOB_NOSORT, 0, &globres);

	switch (err) {
		case 0:  //OK
			for(unsigned int i=0; i<globres.gl_pathc; i++)
			{
				std::string newfname(globres.gl_pathv[i]);
#if 0
				pout << newfname << std::endl;
#endif
				newfname = rootpath + newfname.substr(newplen);
				// If the OS uses anything other then / as a path seperator, 
				// they probably need to swap it back here...
#if 0
				pout << newfname << std::endl;
#endif
				files.push_back(newfname);
			}
			globfree(&globres);
			return 0;               
		case 3:  //no matches
			return 0;
		default: //error
			perr << "Glob error " << err << std::endl;
			return err;
	}
}

#endif

#endif
