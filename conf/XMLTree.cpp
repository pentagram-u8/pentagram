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

#include "pent_include.h"

#include "XMLTree.h"
#include "XMLNode.h"

#include "IDataSource.h"
#include "FileSystem.h"

#include "util.h"

using std::string;
using Pentagram::istring;

XMLTree::XMLTree()
	: tree(new XMLNode("config")), root("config"), is_file(false),
	  readonly(false)
{

}

XMLTree::XMLTree(string fname, istring root_)
	: tree (new XMLNode(root_)), root(root_), is_file(true),
	  readonly(false)
{
	readConfigFile(fname);
}

XMLTree::~XMLTree()
{
	delete tree;
}

void XMLTree::clear(istring root_)
{
	delete tree;
	tree = new XMLNode(root_);
	root = root_;
	is_file = false;
	readonly = false;
}

bool XMLTree::readConfigFile(string fname)
{
	IDataSource *f = FileSystem::get_instance()->ReadFile(fname, true);
	if (!f) return false;

	string sbuf, line;
	while (!f->eof()) {
		f->readline(line);
		sbuf += line;
	}

	delete f;

	if (!readConfigString(sbuf))
		return false;

	is_file = true; // readConfigString sets is_file = false
	filename = fname;
	return true;
}

bool XMLTree::readConfigString(string s)
{
	is_file = false;

	string sbuf(s);
	std::size_t nn=0;
	while(isspace(s[nn])) ++nn;

	if (s[nn] != '<') {
		PERR("Error reading config file");
		return false;
	}
	++nn;
	
	tree->xmlparse(sbuf,nn);

	return true;
}

string XMLTree::dump()
{
	return tree->dump();
}

void XMLTree::write()
{
	if (!is_file || readonly)
		return;

	ODataSource *f = FileSystem::get_instance()->WriteFile(filename, true);
	if (!f) return;

	std::string s = dump();
	const char *cstr = s.c_str();
	f->write(cstr,strlen(cstr));

	delete f;
}

bool XMLTree::hasNode(istring key) const
{
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		return true;
	else
		return false;
}

bool XMLTree::checkRoot(istring key) const
{
	istring k = key.substr(0, key.find('/'));
	return (k == root);
}

void XMLTree::value(istring key, string &ret, const char *defaultvalue) const
{
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void XMLTree::value(istring key, int &ret, int defaultvalue) const
{
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = strtol(sub->value().c_str(), 0, 0);
	else
		ret = defaultvalue;
}

void XMLTree::value(istring key, bool &ret,	bool defaultvalue) const
{
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		ret = (to_uppercase(sub->value()) == "YES");
	else
		ret = defaultvalue;
}

void XMLTree::set(istring key, string value)
{
	tree->xmlassign(key, value);
}

void XMLTree::set(istring key, const char* value)
{
	tree->xmlassign(key, value);
}

void XMLTree::set(istring key, int value)
{
	char buf[32];
	snprintf(buf, 32, "%d", value);
	set(key, buf);
}

void XMLTree::set(istring key, bool value)
{
	if (value)
		set(key, "yes");
	else
		set(key, "no");
}

std::vector<istring> XMLTree::listKeys(istring key, bool longformat)
{
	std::vector<istring> keys;
	const XMLNode *sub = tree->subtree(key);
	if (sub)
		sub->listkeys(key, keys, longformat);

	return keys;
}

void XMLTree::getSubkeys(KeyTypeList &ktl, istring basekey)
{
	tree->searchpairs(ktl, basekey, istring(), 0);
}
