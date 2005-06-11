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

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include <cstring>

#include "pentpal.h"

#include "FileSystem.h"
#include "ConvertShape.h"
#include "Shape.h"
#include "ShapeFrame.h"
#include "Palette.h"

// And not too shockingly similiar to the exult plugin!

#define LOAD_PROC "file_pshp_load"
#define SAVE_PROC "file_pshp_save"
#define EXT "PSHP"
#define AUTHORS "The Pentagram Team"
#define HELP "Pentagram Shape plugin loads and saves shapes which consist of multiple images, sometimes used as animation frames, for use with the Pentagram engine and the games it supports. Frames are handled in separate layers by this plugin"


static void query(void);
static void run(const gchar *name, gint nparams, const GimpParam * param,
				gint *nreturn_vals, GimpParam **return_vals);
static gint32 load_image(IDataSource * ids, const gchar * filename);
static void load_frame(Shape * s, uint32 framenum, GimpDrawable * drawable);
static void paintFrame(Shape * s, uint32 framenum, void * pixels,
				uint32 pitch, sint32 x, sint32 y, GimpPixelRgn * clip_window);

static gint32 save_image(gchar *filename, gint32 image_ID,
						 gint32 drawable_ID, gint32 orig_image_ID);

GimpPlugInInfo PLUG_IN_INFO =
{
	NULL,  /* init_proc  */
	NULL,  /* quit_proc  */
	query, /* query_proc */
	run,   /* run_proc   */
};

MAIN ()

static void query(void)
{
	static GimpParamDef load_args[] = {
		{ GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
		{ GIMP_PDB_STRING, "filename", "The name of the file to load" },
		{ GIMP_PDB_STRING, "raw_filename", "The name entered" }
	};

	static GimpParamDef load_return_vals[] = {
		{ GIMP_PDB_IMAGE, "image", "Output image" }
	};

	static GimpParamDef save_args[] = {
		{ GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive" },
		{ GIMP_PDB_IMAGE,    "image",           "Image to save" },
		{ GIMP_PDB_DRAWABLE, "drawable",        "Drawable to save" },
		{ GIMP_PDB_STRING, "filename", "The name of the file to save" },
		{ GIMP_PDB_STRING, "raw_filename", "The name entered" }
	};

	gimp_install_procedure (LOAD_PROC,
		"Load files in Pentagram SHP format",
		HELP,
		AUTHORS,
		AUTHORS,
		"2004",
		// Funny, this is actually legal syntax if EXT is not a variable
		"<Load>/" EXT,
		NULL,
		GIMP_PLUGIN,
		G_N_ELEMENTS (load_args), G_N_ELEMENTS (load_return_vals),
		load_args, load_return_vals);

	gimp_register_magic_load_handler (LOAD_PROC, EXT, "", "");

/*
	gimp_install_procedure (SAVE_PROC,
		"Save files in Pentagram SHP format",
		HELP,
		AUTHORS,
		AUTHORS,
		"2004",
		// Funny, this is actually legal syntax if EXT is not a variable
		"<Save>/" EXT,
		"INDEXEDA",
		GIMP_PLUGIN,
		G_N_ELEMENTS (save_args), 0,
		save_args, NULL);

	gimp_register_save_handler (SAVE_PROC, EXT, "");
*/
}

static void run(const gchar *name, gint nparams, const GimpParam * param,
				gint *nreturn_vals, GimpParam **return_vals)
{
	static GimpParam values[2];
	GimpPDBStatusType status = GIMP_PDB_SUCCESS;
	gint32 image_ID;
	gint32 drawable_ID;
	gint32 orig_image_ID;
	gchar * name_buf;
	gchar * filename;
	GimpRunMode run_mode;

	*nreturn_vals = 1;
	*return_vals = values;
	values[0].type = GIMP_PDB_STATUS;
	run_mode = (GimpRunMode) param[0].data.d_int32;

	if (! strcmp(name, LOAD_PROC))
	{
		FileSystem filesys(true);
		IDataSource * ids = filesys.ReadFile(param[1].data.d_string);
		gimp_ui_init("pentshp", FALSE);

		if (ids)
		{
			image_ID = load_image(ids, param[1].data.d_string);
			if (image_ID != -1)
			{
				*nreturn_vals = 2;
				values[1].type = GIMP_PDB_IMAGE;
				values[1].data.d_image = image_ID;
			}
			else
			{
				status = GIMP_PDB_EXECUTION_ERROR;
			}

			delete ids;
			ids = 0;
		}
		else
		{
			status = GIMP_PDB_EXECUTION_ERROR;
		}
	}
	else if (! strcmp(name, SAVE_PROC))
	{
		orig_image_ID = param[1].data.d_int32;
		image_ID = orig_image_ID;
		drawable_ID = param[2].data.d_int32;
		filename = param[3].data.d_string;
		if (run_mode != GIMP_RUN_NONINTERACTIVE)
		{
			name_buf = g_strdup_printf("Saving %s:", filename);
			gimp_progress_init(name_buf);
			g_free(name_buf);
		}
		save_image(filename, image_ID,
				   drawable_ID, orig_image_ID);
	}
	else
	{
		status = GIMP_PDB_CALLING_ERROR;
	}

	values[0].data.d_status = status;
}

static gint32 load_image(IDataSource * ids, const gchar * filename)
{
	gint32 image_ID, layer_ID;
	sint32 height, width;
	sint32 min_x, min_y;
	uint32 i;
	ShapeFrame *frame;
	Pentagram::Palette pal;
	guchar cmap[768];
	const ConvertShapeFormat *read_format;
	uint32 read_size = ids->getSize();
	gchar *framename;
	GimpDrawable *drawable;

	read_format = Shape::DetectShapeFormat(ids, read_size);
	if (!read_format)
	{
		return -1;
	}

	if (loadPalette(&pal, read_format))
	{
		for (int i = 0; i < 256; ++i)
		{
			cmap[i* 3] = pal.palette[i * 3];
			cmap[i* 3 + 1] = pal.palette[i * 3 + 1];
			cmap[i* 3 + 2] = pal.palette[i * 3 + 2];
		}
	}
	else
	{
		return -1;
	}

	Shape shape(ids, read_format);
	shape.setPalette(&pal);

	min_x = 0;
	min_y = 0;
	height = 0;
	width = 0;
	for (i = 0; i < shape.frameCount(); ++i)
	{
		frame = shape.getFrame(i);

		if (frame->xoff < min_x)
			min_x = frame->xoff;
		if (frame->yoff < min_y)
			min_y = frame->yoff;
		if (frame->height + frame->yoff > height)
			height = frame->height + frame->yoff;
		if (frame->width + frame->xoff > width)
			width = frame->width + frame->xoff;
	}
	image_ID = gimp_image_new(width - min_x, height - min_y, GIMP_INDEXED);
	gimp_image_set_filename(image_ID, filename);
	gimp_image_set_colormap(image_ID, cmap, 256);

	for (i = 0; i < shape.frameCount(); ++i)
	{
		frame = shape.getFrame(i);
		framename = g_strdup_printf("Frame %d", i);
		layer_ID = gimp_layer_new(image_ID, framename, frame->width, frame->height, GIMP_INDEXEDA_IMAGE, 100, GIMP_NORMAL_MODE);
		g_free(framename);
		gimp_image_add_layer(image_ID, layer_ID, 0);
		gimp_layer_translate(layer_ID, frame->xoff - min_x, frame->yoff - min_y);

		drawable = gimp_drawable_get(layer_ID);
		if (i > 0)
		{
			gimp_drawable_set_visible(layer_ID, FALSE);
		}
		load_frame(&shape, i, drawable);

		gimp_drawable_flush(drawable);
		gimp_drawable_detach(drawable);
	}

	return image_ID;
}

static void load_frame(Shape * s, uint32 framenum, GimpDrawable * drawable)
{
	GimpPixelRgn clip_window;
	ShapeFrame * frame;

	gimp_pixel_rgn_init(&clip_window, drawable, 0, 0, drawable->width, drawable->height, TRUE, FALSE);

	uint32 pitch = drawable->width;
	gulong psize = drawable->width * drawable->height;
	guchar *pixels = g_new0(guchar, psize * 2);

	frame = s->getFrame(framenum);
	paintFrame(s, framenum, (uint16 *) pixels, pitch * 2, frame->xoff, frame->yoff, &clip_window);

	gimp_pixel_rgn_set_rect(&clip_window, pixels, 0, 0, drawable->width, drawable->height);
	g_free(pixels);

//	pout << "Frame " << framenum << " loaded." << std::endl;
}

#include <cstdio>

static void paintFrame(Shape * s, uint32 framenum, void * pixels,
				uint32 pitch, sint32 x, sint32 y, GimpPixelRgn * clip_window)
{
	#define untformed_pal true
	#define NO_CLIPPING
	#define uintX uint16
	// EVIL!!!!!!
	#define clip_window (*clip_window)

	#include "SoftRenderSurface.inl"

	#undef clip_window
	#undef uintX
	#undef NO_CLIPPING
}

static gint32 save_image(gchar *filename, gint32 image_ID,
						 gint32 drawable_ID, gint32 orig_image_ID)
{
	return -1;
}
