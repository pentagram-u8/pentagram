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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <vector>
#include <set>

class XMLTree;
class ConfigNode;

class Configuration
{
 public:
	Configuration();
	~Configuration();

	// read config file. Multiple files may be read. Order is important.
	bool readConfigFile(std::string fname, std::string root,
						bool readonly=false);
	// write all (writable) config files
	void write();
	// clear everything
	void clear();

	// get value
	void value(std::string key, std::string &ret, const char *defaultvalue="");
	void value(std::string key, int &ret, int defaultvalue=0);
	void value(std::string key, bool &ret, bool defaultvalue=false);

	// set value
	bool set(std::string key, std::string value);
	bool set(std::string key, const char* value);
	bool set(std::string key, int value);
	bool set(std::string key, bool value);

	// get node ref. (delete it afterwards)
	ConfigNode* getNode(std::string key);

	// list all subkeys of a key. (no guaranteed order in result)
	std::set<std::string> listKeys(std::string key, bool longformat = false);

 private:

	std::vector<XMLTree*> trees;
};

#endif
