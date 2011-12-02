/*
 *	FileSystem.cpp - The Pentagram File System
 *
 *  Copyright (C) 2002-2005  The Pentagram Team
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

#include "filesys/FileSystem.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string>
using	std::string;

#include "filesys/ListFiles.h"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#endif

FileSystem* FileSystem::filesystem = 0;

FileSystem::FileSystem(bool noforced)
	: noforcedvpaths(noforced), allowdataoverride(true)
{
	con.Print(MM_INFO, "Creating FileSystem...\n");

	assert(filesystem == 0);
	filesystem = this;

#ifdef UNDER_CE
	TCHAR module_filename[256];
	TCHAR *c = module_filename;
	TCHAR *last = NULL;
	GetModuleFileName(NULL, module_filename, 256);

	while (*c)
	{
		if (*c == '/' || *c == '\\')
			last = c;

		c++;
	}

	if (last)
	{
		*last = 0;
	}
	else
	{
		module_filename[0] = '\\';
		module_filename[1] = 0;
	}

	size_t len = _tcslen (module_filename) + 1;
	char *str = (char*) _alloca(len);
	WideCharToMultiByte(CP_ACP, 0, module_filename, -1, str, len, NULL, NULL);

	AddVirtualPath(".", str);

#endif

}

FileSystem::~FileSystem()
{
	con.Print(MM_INFO, "Destroying FileSystem...\n");

	filesystem = 0;
}


// Open a streaming file as readable. Streamed (0 on failure)
IDataSource* FileSystem::ReadFile(const string &vfn, bool is_text)
{
	string filename = vfn;

	IDataSource* data = checkBuiltinData(vfn, is_text);

	// allow data-override?
	if (!allowdataoverride && data) return data;

	std::ifstream *f = new std::ifstream();
	if(!rawopen(*f, filename, is_text)) {
		delete f;
		return data;
	}

	return new IFileDataSource(f);
}

// Open a streaming file as readable. Streamed (0 on failure)
ODataSource* FileSystem::WriteFile(const string &vfn, bool is_text)
{
	string filename = vfn;
	std::ofstream *f = new std::ofstream();
	if(!rawopen(*f, filename, is_text)) {
		delete f;
		return 0;
	}
	return new OFileDataSource(f);
}

/*
 *	Open a file for input,
 *	trying the original name (lower case), and the upper case version
 *	of the name.
 *
 *	Output: 0 if couldn't open.
 */

bool FileSystem::rawopen
	(
	std::ifstream& in,			// Input stream to open.
	const string &fname,		// May be converted to upper-case.
	bool is_text				// Should the file be opened in text mode
	)
{
	string name = fname;
	if (!rewrite_virtual_path(name)) {
//		perr << "Illegal file access" << std::endl;
		return false;
	}

#if defined(MACOS) || (__GNUG__ > 2)
	std::ios_base::openmode mode = std::ios::in;
	if (!is_text) mode |= std::ios::binary;
#elif defined(UNIX)
	int mode = std::ios::in;
#else
	int mode = std::ios::in;
	if (!is_text) mode |= std::ios::binary;
#endif
	switch_slashes(name);

	int uppercasecount = 0;
	do {
		// We first "clear" the stream object. This is done to prevent
		// problems when re-using stream objects
		in.clear();
		try {
			in.open(name.c_str(), mode);			// Try to open
		} catch (std::exception &)
		{}
		if (in.good() && !in.fail()) return true;	// found it!
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
	return false;
}

/*
 *	Open a file for output,
 *	trying the original name (lower case), and the upper case version
 *	of the name.
 *
 *	Output: 0 if couldn't open.
 */

bool FileSystem::rawopen
	(
	std::ofstream& out,			// Output stream to open.
	const string &fname,			// May be converted to upper-case.
	bool is_text				// Should the file be opened in text mode
	)
{
	string name = fname;
	if (!rewrite_virtual_path(name)) {
		con.Print_err(MM_MAJOR_WARN, "Illegal file access\n");
		return false;
	}

#if defined(MACOS) || (__GNUG__ > 2)
	std::ios_base::openmode mode = std::ios::out | std::ios::trunc;
	if (!is_text) mode |= std::ios::binary;
#elif defined(UNIX)
	int mode = std::ios::out | std::ios::trunc;
#else
	int mode = std::ios::out | std::ios::trunc;
	if (!is_text) mode |= std::ios::binary;
#endif
	switch_slashes(name);

	// We first "clear" the stream object. This is done to prevent
	// problems when re-using stream objects
	out.clear();

	int uppercasecount = 0;
	do {
		out.open(name.c_str(), mode);		// Try to open
		if (out.good()) return true;		// found it!
		out.clear();						// Forget ye not
	} while (base_to_uppercase(name, ++uppercasecount));

	// file not found.
	return false;
}

void FileSystem::switch_slashes(string &name)
{
#ifdef WIN32
	for(string::iterator X = name.begin(); X != name.end(); ++X)
	{
		if(*X == '/' )
			*X =  '\\';
	}
#elif defined(MACOS)
	// We use a component-wise algorithm (suggested by Yorick)
	// Basically, split the path along the "/" seperators
	// If a component is empty or '.', remove it. If it's '..', replace it
	// with the empty string. convert all / to :
	string::size_type	begIdx, endIdx;;
	string	component;
	string	new_name;

	if( name.at(0) != '/' )
		new_name = ":";

	begIdx = name.find_first_not_of('/');
	while( begIdx != string::npos )
	{
		endIdx = name.find_first_of('/', begIdx);
		if( endIdx == std::string::npos )
			component = name.substr(begIdx);
		else
			component = name.substr(begIdx, endIdx-begIdx);
		if( component == ".." )
			new_name += ":";
		else if( !component.empty() && component != "." )
		{
			new_name += component;
			if( endIdx != std::string::npos )
				new_name += ":";
		}
		begIdx = name.find_first_not_of('/', endIdx);
	}

	name = new_name;
#else
	// do nothing
#endif
}

/*
 *	Convert just the last 'count' parts of a filename to uppercase.
 *  returns false if there are less than 'count' parts
 */

bool FileSystem::base_to_uppercase(string& str, int count)
{
	if (count <= 0) return true;

	int todo = count;
					// Go backwards.
	string::reverse_iterator X;
	for(X = str.rbegin(); X != str.rend(); ++X)
	{
					// Stop at separator.
		if (*X == '/' || *X == '\\' || *X == ':')
			todo--;
		if (todo <= 0)
			break;

#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
		if ((*X >= 'a') && (*X <= 'z')) *X -= 32;
#else
		*X = static_cast<char>(std::toupper(*X));
#endif
	}
	if (X == str.rend())
		todo--; // start of pathname counts as separator too

	// false if it didn't reach 'count' parts
	return (todo <= 0);
}

bool FileSystem::AddVirtualPath(const string &vpath, const string &realpath, const bool create)
{
	string vp = vpath, rp = realpath;

	// remove trailing slash
	if (vp.rfind('/') == vp.size() - 1)
		vp.erase(vp.rfind('/'));

	if (rp.rfind('/') == rp.size() - 1)
		rp.erase(rp.rfind('/'));

	if (rp.find("..") != string::npos) {
		con.Printf_err(MM_MINOR_ERR,
			"Error mounting virtual path \"%s\": \"..\" not allowed.\n",
			vp.c_str());
		return false;
	}

	// Finding Reserved Virtual Path Names
	// memory path is reserved
	if (vp == "@memory" || vp.substr(0, 8) == "@memory/")
	{
		con.Printf_err(MM_MINOR_ERR,
			"Error mounting virtual path \"%s\": %s\"@memory\" is a reserved virtual path name.\n",
			vp.c_str());
		return false;
	}

	string fullpath = rp;
	rewrite_virtual_path(fullpath);
	// When mounting a memory file, it wont exist, so don't attempt to create the dir
#ifdef DEBUG
	con.Printf(MM_INFO, "virtual path \"%s\": %s\n", vp.c_str(), fullpath.c_str());
#endif
	if (!(fullpath.substr(0, 8) == "@memory/"))
	{
		if (!IsDir(fullpath)) {
			if(!create) {
#ifdef DEBUG
				con.Printf_err(MM_MINOR_WARN,
					"Problem mounting virtual path \"%s\": directory not found: %s\n",
					vp.c_str(), fullpath.c_str());
#endif
				return false;
			}
			else {
				MkDir(fullpath);
			}
		}
	}

	virtualpaths[vp] = rp;
	return true;
}

bool FileSystem::RemoveVirtualPath(const string &vpath)
{
	string vp = vpath;

	// remove trailing slash
	if (vp.rfind('/') == vp.size() - 1)
		vp.erase(vp.rfind('/'));

	std::map<string, string>::iterator i = virtualpaths.find(vp);

	if (i == virtualpaths.end()) {
		return false;
	} else {
		virtualpaths.erase(vp);
		return true;
	}
}

IDataSource* FileSystem::checkBuiltinData(const std::string& vfn, bool is_text)
{
	// Is it a Memory file?
	std::map<string, MemoryFile*>::iterator mf = memoryfiles.find(vfn);

	if (mf != memoryfiles.end())
		return new IBufferDataSource(mf->second->data,
									 mf->second->len, is_text);

	return 0;
}

bool FileSystem::rewrite_virtual_path(string &vfn)
{
	bool ret = false;
	string::size_type pos = vfn.size();

	while ((pos = vfn.rfind('/', pos)) != std::string::npos) {
//		perr << vfn << ", " << vfn.substr(0, pos) << ", " << pos << std::endl;
		std::map<string, string>::iterator p = virtualpaths.find(
			vfn.substr(0, pos));

		if (p != virtualpaths.end()) {
			ret = true;
			// rewrite first part of path
			vfn = p->second + vfn.substr(pos);
			pos = string::npos;
		} else {
			if (pos == 0)
				break;
			--pos;
		}
	}

	// We will allow all paths to work
	if (noforcedvpaths) ret = true;

	return ret;
}


bool FileSystem::IsDir(const string &path)
{
#ifndef UNDER_CE
	bool exists;
	struct stat sbuf;

	string name = path;

	int uppercasecount = 0;
	do {
		exists = (stat(name.c_str(), &sbuf) == 0);
		if (exists) {
			if (S_ISDIR(sbuf.st_mode))
				return true;  // exists, and is a directory
			else
				return false; // exists, but not a directory
		}
	} while (base_to_uppercase(name, ++uppercasecount));
#else

	// A little hack
	if (path == "." || path == "/" || path == "\\" ) return true;

	const TCHAR		*lpszT;
	WIN32_FIND_DATA	fileinfo;
	HANDLE			handle;

#ifdef UNICODE
	const char *name = path.c_str();
	std::size_t nLen = strlen(name)+1;
	LPTSTR lpszT2 = (LPTSTR) _alloca(nLen*2);
	lpszT = lpszT2;
	MultiByteToWideChar(CP_ACP, 0, name, -1, lpszT2, nLen);
#else
	lpszT = path.c_str();
#endif

	handle = FindFirstFile (lpszT, &fileinfo);

	if (handle != INVALID_HANDLE_VALUE)
	{
		FindClose (handle);

		if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return true;
	}
#endif

	return false; // not found
}

/*
 *	Create a directory
 */

bool FileSystem::MkDir(const string &path)
{
	string name = path;
	if(name[0]=='@')
		rewrite_virtual_path(name);

#if (defined(MACOSX) || defined(BEOS))
	// remove any trailing slashes
	string::size_type pos = name.find_last_not_of('/');
	if (pos != string::npos)
	  name.resize(pos+1);
#endif

#if defined(WIN32)
	return CreateDirectoryA(name.c_str(), NULL) == TRUE;
#else
	return mkdir(name.c_str(), 0750) == 0;
#endif
}

/*
 *	Get the current users pentagram home path
 */

const std::string& FileSystem::getHomePath()
{
	static std::string home;
	if (!home.empty()) return home;

#ifdef HAVE_HOME
	home = getenv("HOME");
	home += "/.pentagram";
#elif defined(WIN32)
	// Use the Pentagram sub directory of Application Data, under Windows NT4 and later
	char configFilePath[MAX_PATH] = "";

	HRESULT (WINAPI * SHGetFolderPath)(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPSTR pszPath) = 0;

	HMODULE shell32 = LoadLibrary("shell32.dll");
	if (shell32) *(FARPROC*)&SHGetFolderPath = GetProcAddress(shell32,"SHGetFolderPathA");

	if (!SHGetFolderPath) {
		HMODULE shfolder = LoadLibrary("SHFolder.dll");
		if (shfolder) *(FARPROC*)&SHGetFolderPath = GetProcAddress(shell32,"SHGetFolderPathA");		
	}

	// SHGetFolderPath
	if (SHGetFolderPath) {
		HRESULT hr = SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,0,configFilePath);
		if (hr == S_FALSE || hr == E_FAIL) {
			if (FAILED(SHGetFolderPath(NULL,CSIDL_APPDATA|CSIDL_FLAG_CREATE,NULL,0,configFilePath))) {
				configFilePath[0] = 0;
			}
		}

	}

	// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders\AppData
	if (!*configFilePath) {
		HKEY regkey;

		if (RegOpenKey(HKEY_CURRENT_USER,TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),&regkey) == ERROR_SUCCESS) {

			DWORD type;
			DWORD size = MAX_PATH;
			if (RegQueryValueEx(regkey,TEXT("AppData"),NULL,&type,(LPBYTE)configFilePath,&size) == ERROR_SUCCESS)
			{
				if (type == REG_SZ || type == REG_EXPAND_SZ)
					configFilePath[MAX_PATH-1] = 0;				
				else
					configFilePath[0] = 0;				
			}
			RegCloseKey(regkey);
		}

	}

	// %APPDATA%
	if (!*configFilePath) {
		if (!GetEnvironmentVariable("APPDATA", configFilePath, sizeof(configFilePath))) {
			configFilePath[0] = 0;
		}
	}
	// %USERPROFILE%\Application Data
	if (!*configFilePath) {

		if (!GetEnvironmentVariable("USERPROFILE", configFilePath, sizeof(configFilePath))) {
			configFilePath[0] = 0;
		}
		else {
			Pentagram::strcat_s(configFilePath, "\\Application Data");

			if (!CreateDirectory(configFilePath, NULL)) {
				if (GetLastError() != ERROR_ALREADY_EXISTS) {
					configFilePath[0] = 0;
					perr << "Cannot create application data folder" << std::endl;
				}
			}
		}
	}

	if (*configFilePath) {

		home = configFilePath;
		if (*home.rbegin() != '\\' && *home.rbegin() != '/')
			home += "\\";
		
		home += "Pentagram";
	} else {
		home = ".";
	}
#elif defined(MACOSX)
	home = getenv("HOME");
	home += "/Library/Application Support/Pentagram";
#else
	// TODO: what to do on systems without $HOME?
	home = ".";
#endif

	// If its not a dir, make it
	if (!IsDir(home)) {
		bool succeeded = 
	#if defined(WIN32)
			CreateDirectoryA(home.c_str(), NULL) == TRUE;
	#else
			mkdir(home.c_str(), 0750) == 0;
	#endif
		// If home doesn't exist and we can't create it, chances are there
		// will be some major issues. So we return . (current dir) as the 
		// home path. We first clear the home variable because this 
		// function is called more than once. The first time is before
		// output logging has been enabled, the second time after. We want
		// the error message to be logged so we need to make sure that the
		// function can't use the cached value the second time its called.

		if (!succeeded) {
			perr << "Error creating home directory \""<< home << "\"" << std::endl;

			home = std::string();
			static std::string dot = ".";
			return dot;
		}
	}

	return home;
}

