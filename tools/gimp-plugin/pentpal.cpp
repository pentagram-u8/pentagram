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

#include "pentpal.h"

#include "FileSystem.h"
#include "Palette.h"
#include "ConvertShape.h"
#include "crusader/ConvertShapeCrusader.h"
#include "u8/ConvertShapeU8.h"

bool loadPalette(Pentagram::Palette * pal, const ConvertShapeFormat * format)
{
	GimpRGB color;
	guchar palHack[2];
	palHack[1] = 255;
	bool newPal;
	const gchar * palName;

	for (int i = 0; i < 256; ++i)
	{
		palHack[0] = i;
		pal->native[i] = *((uint16 * ) palHack);
	}

	if (format == &PentagramShapeFormat)
	{
		palName = "Pentagram";
	}
	else if (format == &U8SKFShapeFormat)
	{
		palName = "U8SKF";
	}
	else if (format == &U8ShapeFormat)
	{
		palName = "Ultima8";
	}
	else if (format == &U82DShapeFormat)
	{
		palName = "Ultima8";
	}
	else if (format == &CrusaderShapeFormat)
	{
		palName = "Crusader";
	}
	else if (format == &Crusader2DShapeFormat)
	{
		palName = "Crusader";
	}
	else
	{
		return false;
	}

	newPal = ! gimp_context_set_palette(palName);
	if (newPal)
	{
		gint colors = 0;
		if (gimp_palette_get_info(palName, &colors))
		{
			newPal = false;
			if (colors != 256)
			{
				newPal = true;
				if (!gimp_palette_delete(palName))
					return false;
			}
		}
	}

	if (newPal)
	{
		if (g_ascii_strcasecmp(palName, gimp_palette_new(palName)))
			return false;
		if (!gimp_context_set_palette(palName))
			return false;

		IDataSource * ds = choosePalette();
		if (ds)
		{
			gint j = 0;
			ds->seek(4);
			pal->load(*ds);
			delete ds;
			for (gint i = 0; i < 256; ++i)
			{
				gimp_rgb_set_uchar(&color,
							pal->palette[i * 3],
							pal->palette[i * 3 + 1],
							pal->palette[i * 3 + 2]);
				gimp_palette_add_entry(palName, "Untitled", &color, &j);
				assert (j == i);
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		for (gint i = 0; i < 256; ++i)
		{
			gimp_palette_entry_get_color(palName, i, &color);
			gimp_rgb_get_uchar(&color,
						&(pal->palette[i * 3]),
						&(pal->palette[i * 3 + 1]),
						&(pal->palette[i * 3 + 2]));
		}
	}

	return true;
}

static void file_sel_delete( GtkWidget *widget, GtkWidget **file_sel )
{
	gtk_widget_destroy( *file_sel );
	*file_sel = NULL;
}

static void file_selected( GtkWidget *widget, gboolean *selected )
{
	*selected = TRUE;
}

IDataSource * choosePalette()
{
	FileSystem * filesys = FileSystem::get_instance();
	GtkWidget *file_sel;
	gchar *filename;
	gboolean selected = FALSE;

	file_sel = gtk_file_selection_new( "Choose a Palette" );
	gtk_window_set_modal( GTK_WINDOW( file_sel ), TRUE );

	gtk_signal_connect( GTK_OBJECT( file_sel ), "destroy",
                            GTK_SIGNAL_FUNC( file_sel_delete ), &file_sel );

	gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( file_sel )->cancel_button ), "clicked", GTK_SIGNAL_FUNC( file_sel_delete ), &file_sel );

	gtk_signal_connect( GTK_OBJECT( GTK_FILE_SELECTION( file_sel )->ok_button ), "clicked", GTK_SIGNAL_FUNC( file_selected ), &selected );

	gtk_widget_show( file_sel );

	while( ( ! selected ) && ( file_sel ) )
		gtk_main_iteration();

	/* canceled or window was closed */
	if( ! selected )
		return 0;

	/* ok */
	filename = g_strdup( gtk_file_selection_get_filename( GTK_FILE_SELECTION( file_sel ) ) );
	gtk_widget_destroy( file_sel );

	IDataSource * ds = filesys->ReadFile(filename);
	return ds;
}

