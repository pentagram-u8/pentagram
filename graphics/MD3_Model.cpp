/*
 *  Copyright (C) 2002  Ryan Nunn and The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "MD3_Model.h"
#include <cstring>
#include "IDataSource.h"

using std::memcmp;

//
// Read Header
//
bool MD3_Header::Read(IDataSource *ds)
{
	//
	// Info
	//


	// "IDP3"
	ds->read(type, 4);
	if (memcmp(type, "IDP3", 4)) {
		perr << "Invalid MD3 type. Expected \"IDP3\" found " << type[0] << type[1] << type[2] << type[3] << std::endl;
		return false;
	}

	// 15
	version = ds->read4();
	if (version != 15) {
		perr << "Invalid MD3 version. Expected 15 found " << version << std::endl;
		return false;
	}

	// Name of model
	ds->read(name, 64);
	

	// Usage???
	unknown0 = ds->read4();


	//
	// Counters
	//

	// Number of frames
	num_frames = ds->read4();
	// Number or tags
	num_tags = ds->read4();
	// Number of surfaces
	num_surfs = ds->read4();


	// Usage???
	unknown1 = ds->read4();


	//
	// Offsets
	//

	// Offset of frame info (from start of header)
	offset_finfo = ds->read4();
	// Offset of tags
	offset_tags = ds->read4();
	// Offset of surfaces
	offset_surfs = ds->read4();
	// Offset to end of file
	offset_end = ds->read4();

	return true;
}

//
// Read Frame Info
//
void MD3_FrameInfo::Read(IDataSource *ds)
{
	// Bounding Box lower extents
	box_min[0] = ds->readf();
	box_min[1] = ds->readf();
	box_min[2] = ds->readf();
	
	// Bounding Box greater extents
	box_max[0] = ds->readf();
	box_max[1] = ds->readf();
	box_max[2] = ds->readf();
	
	// Local origin (center of model)
	origin[0] = ds->readf();
	origin[1] = ds->readf();
	origin[2] = ds->readf();
	
	// Radius from origin
	radius = ds->readf();

	// Name of frame
	ds->read(name, 16);
}

//
// Read Tags
//
void MD3_Tag::Read(IDataSource *ds)
{
	// Tag name
	ds->read(name, 64);

	// Tag origin
	origin[0] = ds->readf();
	origin[1] = ds->readf();
	origin[2] = ds->readf();

	// Tag Matrix (vectors forward=0, right=1 and up=2)

	// NOTE That i'm not reading it entirely correctly...
	// I'm doing it like this so I can easily do a MatrixMultiply3x3

	// Forward=0
	matrix[0][0] = ds->readf();	
	matrix[1][0] = ds->readf();	
	matrix[2][0] = ds->readf();	

	// Right=1
	matrix[0][1] = ds->readf();	
	matrix[1][1] = ds->readf();	
	matrix[2][1] = ds->readf();	

	// Up=2
	matrix[0][2] = ds->readf();	
	matrix[1][2] = ds->readf();	
	matrix[2][2] = ds->readf();
}

//
// Read Triangles
//
void MD3_Triangle::Read(IDataSource *ds)
{
	// Triangle incidces
	indices[0] = ds->read4();	
	indices[1] = ds->read4();	
	indices[2] = ds->read4();	
}

//
// Tex Coords
//
void MD3_TexCoord ::Read(IDataSource *ds)
{
	// ST coords
	st[0] = ds->readf();
	st[1] = ds->readf();
}

//
// Read Surface Shaders
//
void MD3_Shader::Read(IDataSource *ds)
{
	// Filename of shader
	ds->read(filename, 64);

	// Shader index
	index = ds->read4();
}

//
// Read Vertices
//
void MD3_DiskVertex::Read(IDataSource *ds)
{
	// Vertex
	xyz[0] = ds->read2();
	xyz[1] = ds->read2();
	xyz[2] = ds->read2(); 

	// Normal longigude (0 to 255 == 0 to 2*PI)
	longitude = ds->read1();

	// Normal latitiude (0 to 255 == 0 to 2*PI)
	latitude =  ds->read1();
}

//
// Read Surface Header
//
bool MD3_SurfaceHeader::Read(IDataSource *ds)
{
	// "IDP3"
	ds->read(type, 4);
	if (memcmp(type, "IDP3", 4)) {
		perr << "Invalid MD3 Surface type. Expected \"IDP3\" found " << type[0] << type[1] << type[2] << type[3] << std::endl;
		return false;
	}

	// Name of surface
	ds->read(name, 64);

	// Usage???
	unknown0 = ds->read4();
	
	// Number of frames. Must be the same as in header
	num_frames = ds->read4();
	// Number of shaders/textures for this surface
	num_shaders = ds->read4();
	// Number of verts for this surface
	num_verts = ds->read4();
	// Number of triangles for this surface
	num_tris = ds->read4();

	// Offset to tris (from start of surf)
	offset_tris = ds->read4();
	// Offset to shaders
	offset_shaders = ds->read4();
	// Offset to texture coords 
	offset_st = ds->read4();
	// Offset to verts and normals
	offset_verts = ds->read4();

	// Offset from start of this surface to next surface
	offset_next = ds->read4();

	return true;
};

//
// Free Surface Data
//
void MD3_Surface::FreeData()
{
	// Delete verts for all frames
	if (vertices)  {
		for (uint32 i = 0; i < num_frames; i++) delete [] vertices[i];
		delete [] vertices;
	}

	// Delete triangles
	delete [] triangles;

	// Delete Shaders
	delete [] shaders;

	// Delete texture coords
	delete [] texcoords;

	// Just quickly fill with 0
	memset(this, 0, sizeof(*this));
}

//
// Read Surface
//
bool MD3_Surface::Read(IDataSource *ds)
{
	uint32 i;
	int start = ds->getPos();

	// Read the header
	if (!MD3_SurfaceHeader::Read(ds)) {
		perr << "Failed to read MD3_SurfaceHeader" << std::endl;
		return false;
	}

	//
	// The Triangles the belong to this surface
	//

	// Seek to the triangle offset
	ds->seek(start + offset_tris);

	// Create buffer
	triangles = new MD3_Triangle[num_tris];

	// Read Triangles
	for (i = 0; i < num_tris; i++) triangles[i].Read(ds);

	//	
	// The shaders belonging to this surface
	//

	// Might not have any!
	if (num_shaders) {

		// Seek to the shader offset
		ds->seek(start + offset_shaders);

		// Create buffer
		shaders = new MD3_Shader[num_shaders];

		// Read Shaders
		for (i = 0; i < num_shaders; i++) shaders[i].Read(ds);
	}
	else {
		// Set it to 0 if we don't have any shaders
		shaders = 0;
	}

	//
	// Texture coords for this surface
	//

	// Seek to the texture coords offset
	ds->seek(start + offset_st);

	// Create buffer
	texcoords = new MD3_TexCoord[num_verts];

	// Read Texture coords
	for (i = 0; i < num_verts; i++) texcoords[i].Read(ds);

	//
	// Frame Vertices
	//

	// Seek to the vertices offset
	ds->seek(start + offset_verts);

	// Create buffer for frames
	vertices = new MD3_Vertex *[num_frames];	

	// Iterate for each frame
	for (uint32 f = 0; f < num_frames; f++) {

		// Create vertex buffer
		vertices[f] = new MD3_Vertex [num_verts];	

		MD3_DiskVertex	disk_vert;

		// Read all verts 
		for (i = 0; i < num_verts; i++) {

			// Read from disk
			disk_vert.Read(ds);

			// Convert to memory format
			vertices[f][i].ConvertDisk(disk_vert);
		}
	}

	//
	// Finished
	//

	// Seek to the next surface
	ds->seek(start + offset_next);

	return true;
};

// MD3Model Default Constructor
MD3_Model::MD3_Model()
{
	// Just quickly fill with 0
	memset(this, 0, sizeof(*this));
}

// MD3Model Destructor
MD3_Model::~MD3_Model()
{
	FreeData();
}

//
// Free All Model Data
//

void MD3_Model::FreeData()
{
	// Delete all tags
	if (tags) {
		for (uint32 i = 0; i < num_frames; i++) delete [] tags[i];
		delete [] tags;
	}

	// Delete Surfaces [num_surfs]
	if (surfaces) {
		for (uint32 i = 0; i < num_surfs; i++) surfaces[i].FreeData();
		delete [] surfaces;
	}

	// Delete Frame info 
	delete [] frameinfo;	

	// Just quickly fill with 0
	memset(this, 0, sizeof(*this));
}

//
// Read MD3 Model
//
bool MD3_Model::Read(IDataSource *ds)
{
	uint32 i;
	int start = ds->getPos();

	// Read the header
	if (!MD3_Header::Read(ds)) {
		perr << "Failed to read MD3_Header" << std::endl;
		return false;
	}

	//
	// Frame info
	//

	// Seek to the triangle offset
	ds->seek(start + offset_finfo);

	// Create buffer
	frameinfo = new MD3_FrameInfo[num_frames];	

	// Read frame info
	for (i = 0; i < num_frames; i++) frameinfo[i].Read(ds);

	//
	// Tags
	//

	// Seek to the vertices offset
	ds->seek(start + offset_tags);

	// Create buffer for tag frames
	tags = new MD3_Tag *[num_frames];	

	// Iterate for each frame
	for (uint32 f = 0; f < num_frames; f++) {

		// Create the buffer for a single frame of tags
		tags[f] = new MD3_Tag [num_tags];	

		// Read the tags
		for (i = 0; i < num_tags; i++) tags[f][i].Read(ds);
	}


	//
	// Surfaces
	//

	// Seek the surface offset
	ds->seek(start + offset_surfs);

	// Create buffer for surfaces
	surfaces = new MD3_Surface[num_surfs];

	// Clear buffer
	memset (surfaces, 0, num_surfs*sizeof(MD3_Surface));

	// Read each surface
	for (i = 0; i < num_surfs; i++) surfaces[i].Read(ds);


	//
	// Finished
	//

	// Seek to the end
	ds->seek(start + offset_end);

	return true;

}

// Output info to the console
void MD3_Model::OutputInfo()
{
	int i;


	pout << "MD3 name: " << name << std::endl;
	pout << "MD3 frames: " << num_frames << std::endl;
	pout << "MD3 tags: " << num_tags << std::endl;
	for (i = 0; i < num_tags; i++) pout << "MD3 tag " << i << " name: " << tags[0][i].name << std::endl;
	pout << "MD3 surfs: " << num_surfs<< std::endl;
	for (i = 0; i < num_surfs; i++) {
		pout << "MD3 surface " << i << " name: " << surfaces[i].name << std::endl;
		pout << "MD3 surface " << i << " frames: " << surfaces[i].num_frames << std::endl;
		pout << "MD3 surface " << i << " shaders: " << surfaces[i].num_shaders << std::endl;
		pout << "MD3 surface " << i << " tris: " << surfaces[i].num_tris << std::endl;
		pout << "MD3 surface " << i << " verts: " << surfaces[i].num_verts << std::endl;

		for (int j = 0; j < surfaces[i].num_shaders; j++) {
			MD3_Shader *s = surfaces[i].shaders+j;
			pout << "MD3 surface " << i << " shader " << j << " index: " << s->index << std::endl;
			pout << "MD3 surface " << i << " shader " << j << " filename: " << s->filename << std::endl;
		}
	}

	pout << std::endl;
}
