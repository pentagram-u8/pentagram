/*
 *	FileSystem.h - The Pentagram File System
 *
 *  Copyright (C) 2002, 2003  The Pentagram Team
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

/*

Currently this is just a very cut-down and stub (and also wrong, but that's
a problem for another time), class to handle the FileSys. How the 'real'
one should look like is below. *grin*

*****************************************************************************

using std::string;      // Looks too ugly otherwise

class PentagramFileSystem {
public:
// Add (or replace) a virtual path (false on failure)
bool AddVirtualPath (const string &vpath, const string &real_path);

// Remove a virtual path (false on failure)
bool RemoveVirtualPath (const string &vpath)

// Clear all virtual paths (false on failure)
bool ClearVirtualPathTable();

// Get the disk filename from a virtual filename ("" on failure)
string GetRealName (const string &vfn);

// List all files in a directory (0 sized vector on failure)
std::vector<string> ListFiles (const string &vpath, const string &wildcard)

// Open a streaming file as readable. Streamed (0 on failure)
DataSource *ReadFile(const string &vfn, bool text=false);

// Open a buffered file as readable (0 on failure)
DataSource *ReadBuffered(const string &vfn, bool text=false);

// Open a file for writing (0 on failure)
DataSource *WriteFile(const string &vfn, bool text=false);
};

The DataSource's used for reading and writing would be a tad different to
the normal ones that are currently used. The Reading and Writing ones will
need to close the file once they are deleted. The Buffered should probably
use a shared buffer so multiple ReadBuffered calls for the same file will
use the same buffer rather than create a new one. It would use reference
counting and each time a buffered data source is deleted the ref is
decreased and once the ref becomes 0, the buffer itself is deleted. It
would probably also be possible to cache the buffers for a certain length
of time with the FileSystem object automatically flushing the cache of
unused buffers after a certain amount of time.

I know, make the FileSystem a process... just had to say it, but it would
be one way to automatically flush the cache, should it be done. :-)

A further possible idea would be mounting zip files and accessing their
contents by a virtual filename. It's not really anything that we'd use
right away but it's something to think about for the future. Regardless, it
would still be possible to add in such stuff at a later date if things are
kept clean enough and I really can't see how they wouldn't be.

You might want to have an alternative here too, something like:

// List all files in a directory, into filelist (returns false on failure)
bool ListFiles (std::vector<string> &filelist, const string &vpath, const
string &wildcard)

For those who are either 1) excessivly paranoid, or 2) are expecting to grab a
lot of files from that directory where the vector/string copying may be a
significant overhead.
*/

#include <fstream>
#include <string>
#include <map>
#include <list>

#include "IDataSource.h"
#include "ODataSource.h"

class FileSystem
{
 public:
	//! \param noforcedvpaths if false, all file operations must use vpaths
	FileSystem(bool noforcedvpaths = false);
	~FileSystem();

	static FileSystem* get_instance() { return filesystem; }

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

	//! Mount a buffer as a file in memory
	//! \param vpath the vpath under which to mount the file.
	//!              (Preferably starting with '@memory', but not required.)
	//! \param data the data
	//! \param len size (in bytes) of data
	//! \return true if succesful
	bool MountFileInMemory(const std::string &vpath, const uint8 *data,
						   const uint32 len);

	//! Create a directory
	//! \param path the directory to create. (Can be virtual)
	//! \return OS-specific (FIXME!)
	int MkDir(const std::string& path); // can handle both paths and vpaths

	typedef std::list<std::string> FileList;

	//! List files matching a mask
	//! \param mask the mask to match
	//! \param files the FileList to which the found files are appended
	//! \param return OS-specific (FIXME!)
	int ListFiles(const std::string mask, FileList& files);

 private:
	void switch_slashes(std::string &name);
	bool base_to_uppercase(std::string& str, int count);

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
	
	bool IsDir(const std::string& path);

	static FileSystem* filesystem;

	// This will disable the usage of forced virtual paths.
	// It's useful for 'tools'
	bool	noforcedvpaths;

	// rewrite virtual path in-place (i.e., fvn is replaced)
	// returns false if no rewriting was done
	bool rewrite_virtual_path(std::string& vfn);

	std::map<std::string, std::string> virtualpaths;

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
