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

// Added for -xs option
#include "Shape.h"
#include "ConvertShape.h"
#include "u8/ConvertShapeU8.h"
#include "crusader/ConvertShapeCrusader.h"

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
	std::vector<std::string> file_names;
	IDataSource *ids = 0;
	ODataSource *ods = 0;
	FlexWriter * fw;
	const char * fxo_ext = "fxo";
	int i = 0;
	uint32 index = 0;
	program = argv[0];

	if (argc < 2) {
		usage();
		return 1;
	}

	con.DisableWordWrap();
	std::string opt_str = argv[1];

	FileSystem filesys(true);

	if (opt_str.length() == 2 && opt_str[0] == '-') {
		if (argc < 3 && opt_str != "-l" && opt_str != "-x") { // -l and -x might take only flexfile
			perr << "Error: Missing arguments for " << opt_str << " option." << std::endl << std::endl;
			usage();
			return 1;
		}
		if (argc > 2) { // Most options will use argv[2] as flexfile or manifest
			flexfile = argv[2];
		}


		switch(opt_str[1]) {
			case 'a':
			{
				set_mode(mode, ADD);
				if (argc < 4) { perr << "Error: Missing <file>... for -a option." << std::endl << std::endl; usage(); return 1; }
				for (i = 3; i < argc; ++i)
				{
					file_names.push_back(argv[i]);
				}
			}
			break;
			case 'c':
			{
				set_mode(mode, CREATE);
				if (argc < 4) { perr << "Error: Missing <file>... for -c option." << std::endl << std::endl; usage(); return 1; }
				for (i = 3; i < argc; ++i)
				{
					file_names.push_back(argv[i]);
				}
			}
			break;
			case 'i': // Manifest mode
			{
				set_mode(mode, RESPONSE);
				std::string manifest_filename = argv[2]; // argv[2] is the manifest file
				ids = filesys.ReadFile(manifest_filename);
				if (ids) {
					flexfile.clear(); // Ensure string is empty before readline
					ids->readline(flexfile); // readline is void
					if (flexfile.empty() && ids->eof()) { // Check if readline failed due to EOF or line was truly empty
						perr << "Error: Could not read target flexfile name from manifest '" << manifest_filename << "' or manifest is empty." << std::endl;
						FORGET_OBJECT(ids);
						return 1;
					}
					// Trim whitespace from flexfile name just in case
					flexfile.erase(0, flexfile.find_first_not_of(" \t\n\r"));
					flexfile.erase(flexfile.find_last_not_of(" \t\n\r") + 1);
					if (flexfile.empty()){
						perr << "Error: Target flexfile name read from manifest '" << manifest_filename << "' is empty after trim." << std::endl;
						FORGET_OBJECT(ids);
						return 1;
					}

					while(!ids->eof()) { // Loop while not at End Of File
						std::string file_to_add_from_manifest;
						ids->readline(file_to_add_from_manifest);
						if (ids->eof() && file_to_add_from_manifest.empty()) { // Break if EOF and last readline yielded no data
							break;
						}
						// Trim whitespace
						file_to_add_from_manifest.erase(0, file_to_add_from_manifest.find_first_not_of(" \t\n\r"));
						file_to_add_from_manifest.erase(file_to_add_from_manifest.find_last_not_of(" \t\n\r") + 1);
						if (!file_to_add_from_manifest.empty()) {
							file_names.push_back(file_to_add_from_manifest);
						}
						if (ids->eof() && !file_to_add_from_manifest.empty()){ // If last line had data and then EOF
							break;
						}
					}
					FORGET_OBJECT(ids);
				} else {
					perr << "Manifest file not found: " << manifest_filename << std::endl;
					return 1;
				}
			}
			break;
			case 'l':
			{
				set_mode(mode, LIST);
				if (argc < 3) { perr << "Error: Missing <flexfile> for -l option." << std::endl << std::endl; usage(); return 1; }
			}
			break;
			case 'x':
			{
				set_mode(mode, EXTRACT);
				if (argc < 3) { perr << "Error: Missing <flexfile> for -x option." << std::endl << std::endl; usage(); return 1; }
			}
			break;
			case 'r':
			{
				set_mode(mode, REPLACE);
				if (argc < 5)
				{
					perr << "Missing parameters for -r! Need <flexfile> <object number> <file>" << std::endl << std::endl;
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
	} else {
		mode = NONE;
	}

	if (mode == NONE && opt_str.length() > 0) { // If mode wasn't set by any valid option but an option was given
	perr << "Invalid option: " << opt_str << std::endl << std::endl;
	usage();
	return 1;
} else if (mode == NONE) { // No option given, argc < 2 already handled this. This is defensive.
	usage();
	return 1;
}


switch (mode)
{
	case EXTRACT:
	{
		ids = filesys.ReadFile(flexfile);
		if (!ids)
		{
			perr << "Flex file not found: " << flexfile << std::endl;
			return 1;
		}
		else if (! FlexFile::isFlexFile(ids))
		{
			perr << "File is not a Flex: " << flexfile << std::endl;
			delete ids;
			return 1;
		}
		FlexFile * flex = new FlexFile(ids); // ids ownership transferred
		pout << "Extracting objects from: " << flexfile << std::endl;

		for(index=0; index < flex->getIndexCount(); index++)
		{
			uint32 size;
			uint8* data = flex->getObject(index, &size);
			if (!data || size == 0) {
				if(data) delete[] data;
				continue;
			}

			// Shape detection logic integrated here
			IBufferDataSource* obj_ids = new IBufferDataSource(data, size, false);
			const ConvertShapeFormat* detected_format = Shape::DetectShapeFormat(obj_ids, size);
			delete obj_ids; // Clean up the temporary IBufferDataSource

			bool is_shape = false;
			const char* current_ext = fxo_ext; // Default to .fxo

			if (detected_format) {
				if (detected_format == &U8ShapeFormat ||
					detected_format == &U82DShapeFormat ||
						detected_format == &CrusaderShapeFormat ||
							detected_format == &Crusader2DShapeFormat ||
				detected_format == &PentagramShapeFormat) {
					is_shape = true;
					current_ext = "shp"; // Change extension to .shp if it's a shape
				}
			}

			char outfile[32];
			snprintf(outfile,32,"%04u.%s", index, current_ext); // Use current_ext
			
			if (is_shape) {
				//pout << "  Object " << index << " is a SHP file (" << detected_format->name << "). Extracting as " << outfile << std::endl;
			} else {
				//pout << "  Object " << index << " is not a recognized SHP file. Extracting as " << outfile << std::endl;
			}

			ods = filesys.WriteFile(outfile);
			if (!ods)
			{
				perr << "Could not create file: " << outfile << std::endl;
			} else {
				ods->write(data, size);
				FORGET_OBJECT(ods);
			}
			delete[] data;
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
	perr << "Usage: " << program << " -r <flexfile> <object_number> <file>" << std::endl;
	perr << "or" << std::endl;
	perr << "Usage: " << program << " -i <manifestfile>" << std::endl << std::endl;
	perr << "Modes:" << std::endl;
	perr << "  -a : Add <file>(s) to <flexfile>" << std::endl;
	perr << "  -c : Create <flexfile> with <file>(s)" << std::endl;
	perr << "  -x : Extract all objects from <flexfile>" << std::endl;
	perr << "  -l : List contents of <flexfile>" << std::endl;
	perr << "  -r : Replace <object_number> in <flexfile> with <file>" << std::endl;
	perr << "  -i : Process commands from <manifestfile>. Manifest format:" << std::endl;
	perr << "       Line 1: target <flexfile> name" << std::endl;
	perr << "       Subsequent lines: names of files to add/create" << std::endl;
}
