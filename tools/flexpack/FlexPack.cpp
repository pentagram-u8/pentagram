/*
Copyright (C) 2004 The Pentagram team

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
#include "FlexWriter.h"
#include "FlexFile.h"
#include "ODataSource.h"
#include "IDataSource.h"
#include "FileSystem.h"

void usage();

enum Arch_mode { NONE, LIST, EXTRACT, CREATE, ADD, REPLACE, RESPONSE };

void set_mode(Arch_mode &mode, Arch_mode new_mode)
{
	if(mode!=NONE) {
		perr << "Error: cannot specify multiple modes" << std::endl;
		exit(1);
	} else
		mode = new_mode;
}

static char * program;

int main(int argc, char **argv)
{
	Arch_mode mode = NONE;
	std::string flexfile = c_empty_string;
	std::string file_name;
	std::string option;
	std::vector<std::string> file_names;
	IDataSource *ids = 0;
	ODataSource *ods = 0;
	FlexWriter * fw;
	const char * ext = "fxo";
	int i = 0;
	uint32 index = 0;
	program = argv[0];

	if (argc < 3) {
		usage();
		return 1;
	}

	con.DisableWordWrap();
	option = argv[1];

	FileSystem filesys(true);
	if((argv[1][0]=='-')&&(strlen(argv[1])==2))
	{
		flexfile = argv[2];

		switch(argv[1][1]) {
			case 'a':
			{
				set_mode(mode, ADD);
				for (i = 3; i < argc; ++i)
				{
					file_names.push_back(argv[i]);
				}
			}
			break;
			case 'c':
			{
				set_mode(mode, CREATE);
				for (i = 3; i < argc; ++i)
				{
					file_names.push_back(argv[i]);
				}
			}
			break;
			case 'i':
			{
				set_mode(mode, RESPONSE);
				ids = filesys.ReadFile(flexfile);
				if (ids)
				{
					ids->readline(flexfile);
					while(! ids->eof())
					{
						ids->readline(file_name);
						file_names.push_back(argv[i]);				
					}
					FORGET_OBJECT(ids);
				}
				else
				{
					perr << "Manifest not found." << std::endl;
					return 1;
				}
			}
			break;
			case 'l':
			{
				set_mode(mode, LIST);
			}
			break;
			case 'x':
			{
				set_mode(mode, EXTRACT);
			}
			break;
			case 'r':
			{
				set_mode(mode, REPLACE);
				if (argc < 5)
				{
					perr << "Missing parameters!" << std::endl << std::endl;
					usage();
					return 1;				
				}
				index = strtoul(argv[3],0,10);
				file_name = argv[4];
			}
			break;
			default:
				mode = NONE;
		};
	}

	switch (mode)
	{
		case EXTRACT:
		{
			ids = filesys.ReadFile(flexfile);
			if (!ids)
			{
				perr << "Flex not found" << std::endl;
				return 1;
			}
			else if (! FlexFile::isFlexFile(ids))
			{
				perr << "File is not a Flex" << std::endl;
				delete ids;
				return 1;
			}
			FlexFile * flex = new FlexFile(ids);
			for(index=0; index < flex->getIndexCount(); index++)
			{
				uint32 size;
				uint8* data = flex->getObject(index, &size);
				if (!data || size == 0) continue;
				char outfile[32];
				snprintf(outfile,32,"%04X.%s", index, ext);
				ods = filesys.WriteFile(outfile);
				if (!ods)
				{
					perr << "Could not create file" << std::endl;
					delete flex;
					return 1;
				}
				ods->write(data, size);
				delete[] data;
				FORGET_OBJECT(ods);
			}
			delete flex;
		}
		break;
		case LIST:
		{
			ids = filesys.ReadFile(flexfile);
			if (!ids)
			{
				perr << "Flex not found" << std::endl;
				return 1;
			}
			else if (! FlexFile::isFlexFile(ids))
			{
				perr << "File is not a Flex" << std::endl;
				delete ids;
				return 1;
			}
			FlexFile * flex = new FlexFile(ids);
			pout << "Archive: " << flexfile << std::endl;
			pout << "Size: " << flex->getIndexCount() << std::endl;
			pout << "-------------------------" << std::endl;
			pout << "Object\tLength" << std::endl;
			for(index=0; index < flex->getIndexCount(); index++) {
				uint32 size = flex->getSize(index);
				if (size > 0)
					pout << index << "\t" << flex->getSize(index) << std::endl;
			}
			delete flex;
		}
		break;
		case REPLACE:
		{
			ids = filesys.ReadFile(flexfile);
			if (!ids)
			{
				perr << "Flex not found" << std::endl;
				return 1;
			}
			else if (! FlexFile::isFlexFile(ids))
			{
				perr << "File is not a Flex" << std::endl;
				delete ids;
				return 1;
			}

			FlexFile * flex = new FlexFile(ids);
			fw = new FlexWriter(flex);
			ids = filesys.ReadFile(file_name);
			if (!ids)
			{
				perr << "File Not Found: " << file_name << std::endl;
				delete fw;
				return 1;
			} 

			fw->set_datasource(index, ids);

			ods = filesys.WriteFile(flexfile);
			if (!ods)
			{
				perr << "Could not create flex" << std::endl;
				delete fw;
				return 1;
			}
			fw->write(ods);
			delete fw;
		}
		break;
		case ADD:
		{
			ids = filesys.ReadFile(flexfile);
			if (!ids)
			{
				perr << "Flex not found" << std::endl;
				return 1;
			}
			else if (! FlexFile::isFlexFile(ids))
			{
				perr << "File is not a Flex" << std::endl;
				delete ids;
				return 1;
			}

			FlexFile * flex = new FlexFile(ids);
			fw = new FlexWriter(flex);
			std::vector<std::string>::iterator it;
			for (it = file_names.begin(); it != file_names.end(); ++it)
			{
				ids = filesys.ReadFile(*it);
				if (!ids)
				{
					perr << "File Not Found: " << *it << std::endl;
					delete fw;
					return 1;
				} 
				fw->add_datasource(ids);
			}

			ods = filesys.WriteFile(flexfile);
			if (!ods)
			{
				perr << "Could not create flex" << std::endl;
				delete fw;
				return 1;
			}
			fw->write(ods);
			delete fw;
		}
		break;
		case RESPONSE:
		case CREATE:
		{
			fw = new FlexWriter();
			std::vector<std::string>::iterator it;
			for (it = file_names.begin(); it != file_names.end(); ++it)
			{
				ids = filesys.ReadFile(*it);
				if (!ids)
				{
					perr << "File Not Found: " << *it << std::endl;
					delete fw;
					return 1;
				} 
				fw->add_datasource(ids);
			}
			ods = filesys.WriteFile(flexfile);
			if (!ods)
			{
				perr << "Could not create flex" << std::endl;
				delete fw;
				return 1;
			}
			fw->write(ods);
			delete fw;
		}
		break;
		case NONE:
		default:
		{
			perr << "Invalid switch!" << std::endl << std::endl;
			usage();
			return 1;
		}
	};
	return 0;
}

void usage()
{
	perr << "Usage: " << program << " [-a|-c] <flexfile> <file>..." << std::endl;
	perr << "or" << std::endl;
	perr << "Usage: " << program << " [-x|-l] <flexfile>" << std::endl;
	perr << "or" << std::endl;
	perr << "Usage: " << program << " -r <flexfile> <object number> <file>" << std::endl;
	perr << "or" << std::endl;
	perr << "Usage: " << program << " -i MANIFEST" << std::endl;
}
