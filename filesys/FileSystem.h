/*
 *	FileSystem.h - The Pentagram File System
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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <fstream>
#include <string>
#include <map>
#include <list>

#include "filesys/IDataSource.h"
#include "filesys/ODataSource.h"

class FileSystem
{
 public:
	//! \param noforcedvpaths if false, all file operations must use vpaths
	FileSystem(bool noforcedvpaths = false);
	~FileSystem();

	//! Initialize builtin data files.
	void initBuiltinData(bool allowoverride);

	static FileSystem* get_instance() { return filesystem; }
	
	//! Get the current users pentagram home path
	//! \returns ~/.pentagram or equivilant
	static const std::string& getHomePath();

	//! Open a file as readable. Streamed.
	//! \param vfn the (virtual) filename
	//! \param is_text open in text mode?
	//! \return 0 on failure
	IDataSource *ReadFile(const std::string &vfn, bool is_text=false);

	//! Open a file as writable. Streamed.
	//! \param vfn the (virtual) filename
	//! \param is_text open in text mode?
	//! \return 0 on failure
	ODataSource *WriteFile(const std::string &vfn, bool is_text=false);

	//! Mount a virtual path
	//! \param vpath the name of the vpath (should start with '@')
	//! \param realpath the name of the path to mount (note that this can
	//!                 be a virtual path itself)
	//! \param create create realpath directory if it doesn't exist?
	//! \return true if succesful
	bool AddVirtualPath(const std::string &vpath, const std::string &realpath,
						bool create=false);

	//! Unmount a virtual path
	bool RemoveVirtualPath(const std::string &vpath);

	//! Create a directory
	//! \param path the directory to create. (Can be virtual)
	//! \return true if successful; otherwise, false.
	bool MkDir(const std::string& path); // can handle both paths and vpaths

	typedef std::list<std::string> FileList;

	//! List files matching a mask
	//! \param mask the mask to match
	//! \param files the FileList to which the found files are appended
	//! \param return OS-specific (FIXME!)
	int ListFiles(const std::string mask, FileList& files);

 private:
	static void switch_slashes(std::string &name);
	static bool base_to_uppercase(std::string& str, int count);

	bool rawopen
	(
	std::ifstream& in,			// Input stream to open.
	const std::string &fname,	// May be converted to upper-case.
	bool is_text = false		// Should the file be opened in text mode
	);
		
	bool rawopen
	(
	std::ofstream& out,			// Output stream to open.
	const std::string &fname,	// May be converted to upper-case.
	bool is_text = false		// Should the file be opened in text mode
	);
	
	static bool IsDir(const std::string& path);

	static FileSystem* filesystem;

	// This will disable the usage of forced virtual paths.
	// It's useful for 'tools'
	bool	noforcedvpaths;

	// This enables/disables overriding builtin data files with external ones
	bool	allowdataoverride;

	// rewrite virtual path in-place (i.e., fvn is replaced)
	// returns false if no rewriting was done
	bool rewrite_virtual_path(std::string& vfn);

	std::map<std::string, std::string> virtualpaths;

	//! Check if the given file is a builtin data file.
	//! If so, return an IDataSource for it. If not, return 0.
	IDataSource* checkBuiltinData(const std::string& vfn, bool is_text=false);

	struct MemoryFile
	{
		MemoryFile(const uint8* _data, const uint32 _len)
			: data(_data), len(_len) { } 
		const uint8		*data;
		const uint32	len;
	};
	std::map<std::string, MemoryFile*> memoryfiles;	// Files mounted in memory

};



#endif
