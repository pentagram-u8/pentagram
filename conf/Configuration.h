/*
Copyright (C) 2002-2003 The Pentagram team

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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <vector>
#include <set>

class XMLTree;
class ConfigNode;


/*
Configuration class.

The configuration values are stored in a tree. (or a forest, technically)
All values are stored as strings, but access functions for ints and bools
 are provided
You should only store values in leaf nodes. (This isn't enforced everywhere,
 but contents of non-leaf nodes can disappear without warning)

You can load multiple config files, which can be read-only.
Each config file contains a single tree.
Values in files loaded last override values in files loaded earlier.
Values are written to the last-loaded writable config file with the right root.
 Because of this it's important to make sure the last-loaded config file with
 a given root is writable. (The idea is that you can load a system-wide config
 file first, and a user's config file after that.)
 
*/

class Configuration
{
 public:
	Configuration();
	~Configuration();

	//! read a config file. Multiple files may be read. Order is important.
	//! \param fname The file to read
	//! \param root The name of the root node in the file
	//! \param readonly If true, don't write to this file's tree (or the file)
	//! \return true if succesful
	bool readConfigFile(std::string fname, std::string root,
						bool readonly=false);

	//! write all (writable) config files
	void write();

	//! clear everything
	void clear();

	//! get value
	void value(std::string key, std::string &ret, const char *defaultvalue="");
	//! get value
	void value(std::string key, int &ret, int defaultvalue=0);
	//! get value
	void value(std::string key, bool &ret, bool defaultvalue=false);


	//! set value
	bool set(std::string key, std::string value);
	//! set value
	bool set(std::string key, const char* value);
	//! set value
	bool set(std::string key, int value);
	//! set value
	bool set(std::string key, bool value);


	//! get node ref. (Delete it afterwards)
	ConfigNode* getNode(std::string key);

	//! list all subkeys of a key
	//! \param key The key to return setkeys of
	//! \param longformat If true, return the full key name, instead of
	//!                   just the last part
	//! \return the subkeys. They have no guaranteed order.
	std::set<std::string> listKeys(std::string key, bool longformat = false);

	typedef std::pair<std::string, std::string> KeyType;
	typedef std::vector<KeyType> KeyTypeList;

	//! list all key-value pairs under the given basekey.
	//! \param ktl The list in which key-value pairs are returned
	//! \param basekey The key to search
	void getSubkeys(KeyTypeList &ktl, std::string basekey);

 private:

	std::vector<XMLTree*> trees;
};

#endif
