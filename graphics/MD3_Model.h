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

#ifndef MD3_MODEL_H
#define MD3_MODEL_H

#include <cmath>
#include "q_math.h"
#include "databuf.h"
#include <iostream>

    /////////////////////////
   //                     //
  // MD3 Loading Library //
 //                     //
/////////////////////////

/* Note that all the structures are packed tight on disk */


  //        //
 // Header //
//        //
struct MD3_Header {

	//
	// Info
	//

	// "IDP3"
	char	type[4];
	// 15
	uint32	version;
	// Name of model
	char	name[64];
	

	// Usage???
	uint32	unknown0;


	//
	// Counters
	//

	// Number of frames
	uint32	num_frames;
	// Number or tags
	uint32	num_tags;
	// Number of surfaces
	uint32	num_surfs;


	// Usage???
	uint32	unknown1;


	//
	// Offsets
	//

	// Offset of frame info (from start of header)
	uint32	offset_finfo;
	// Offset of tags
	uint32	offset_tags;
	// Offset of surfaces
	uint32	offset_surfs;
	// Offset to end of file
	uint32	offset_end;

	// Read from a DataSource
	bool	Read(DataSource*);
};

//
// Frame Info
//
struct MD3_FrameInfo {
	float	box_min[3];	// Bounding Box lower extents
	float	box_max[3];	// Bounding Box greater extents
	float	origin[3];	// Local origin (center of model)
	float	radius;		// Radius from origin
	char	name[16];	// Name of frame

	//
	// Methods
	//

	// Read from DataSource
	void	Read(DataSource*);
};

//
// Tags
//
struct MD3_Tag {
	char	name[64];		// Tag name
	float	origin[3];		// Tag origin
	float	matrix[3][3];	// Tag Matrix (vectors forward=0, right=1 and up=2)

	//
	// Methods
	//

	void	Read(DataSource*);
};

//
// Triangles
//
struct MD3_Triangle {
	uint32	indices[3];	// Triangle incidces

	//
	// Methods
	//

	// Read from DataSource
	void	Read(DataSource*);
};

//
// Tex Coords
//
struct MD3_TexCoord {
	float	st[2];		// ST coords

	//
	// Methods
	//

	// Read from DataSource
	void	Read(DataSource*);
};

//
// Surface Shaders
//
struct MD3_Shader {
	char	filename[64];		// Filename of shader
	uint32	index;				// Shader index

	//
	// Methods
	//

	// Read from DataSource
	void	Read(DataSource*);
};

//
// Vertices (Disk)
//
struct MD3_DiskVertex {
	sint16	xyz[3];		// Vertex

	uint8	longitude;	// Normal longigude (0 to 255 == 0 to 2*PI)
	uint8	latitude;	// Normal latitiude (0 to 255 == 0 to 2*PI)

	//
	// Methods
	//

	// Read from DataSource
	void	Read(DataSource*);

	// Get the vertex
	inline void GetVertex(float vertex[3]) {
		vertex[0] = xyz[0] / 64.0F;
		vertex[1] = xyz[1] / 64.0F;
		vertex[2] = xyz[2] / 64.0F;
	}

	// Get the normal
	inline void GetNormal(float normal[3]) {
		float flat = latitude * M_PI/128;
		float flong = longitude * M_PI/128;
		normal[0] = std::cos(flat) * std::sin(flong);
		normal[1] = std::sin(flat) * std::sin(flong);
		normal[2] = std::cos(flong);
	}	
};


//
// Vertices (Memory)
//
struct MD3_Vertex {

	// Format in memory
	float	xyz[3];
	float	normal[3];

	inline void ConvertDisk(MD3_DiskVertex &disk) {
		disk.GetVertex(xyz);
		disk.GetNormal(normal);
	}
};

//
// Surface Headar
//
struct MD3_SurfaceHeader {
	char	type[4];			// "IDP3"
	char	name[64];			// Name of surface

	sint32	unknown0;			// Usage???
	
	uint32	num_frames;			// Number of frames. Must be the same as in header
	uint32	num_shaders;		// Number of shaders/textures for this surface
	uint32	num_verts;			// Number of verts for this surface
	uint32	num_tris;			// Number of triangles for this surface

	uint32	offset_tris;		// Offset to tris (from start of surf)
	uint32	offset_shaders;		// Offset to shaders
	uint32	offset_st;			// Offset to texture coords 
	uint32	offset_verts;		// Offset to verts and normals

	uint32	offset_next;		// Offset from start of this surface to next surface

	//
	// Methods
	//

	bool	Read(DataSource*);

};


//
// Surfaces (Inherits MD3_SurfaceHeader)
//
struct MD3_Surface : public MD3_SurfaceHeader
{
	MD3_Triangle	*triangles;	// The Triangles the belong to this surface
	MD3_Shader		*shaders;	// The shaders belonging to this surface
	MD3_TexCoord	*texcoords;	// Texture coords for this surface
	MD3_Vertex		**vertices;	// All the verts for all the frames

	//
	// Methods
	//

	void			FreeData();

	bool			Read(DataSource*);
};

//
// MD3 Model (Inherits MD3_Header)
//
class MD3_Model : public MD3_Header {
public:

	// Frame info [num_frames]
	MD3_FrameInfo	*frameinfo;	

	// Tags [num_frames][num_tags]
	MD3_Tag			**tags;
	
	// Surfaces [num_surfs]
	MD3_Surface		*surfaces;

	//
	// Methods
	//

	// Default Constructor
	MD3_Model();

	// Destructor
	~MD3_Model();

	// Free the model
	void	FreeData();

	// Read from a DataSource
	bool 	Read(DataSource*);

	// Output info to the console
	void	OutputInfo();
};

#endif //MD3_MODEL_H
