# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/aseprite_plugin

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LCONPRODUCTS := pent_shp

pent_shp_OBJ   = \
	tools/aseprite_plugin/pent_shp.o \
	$(CONVERT) \
	$(MISC) \
	filesys/FileSystem.o \
	graphics/Palette.o \
	graphics/XFormBlend.o \
	graphics/Shape.o \
	graphics/ShapeFrame.o

PNG_CFLAGS := $(shell pkg-config --cflags libpng)
PNG_LIBS := $(shell pkg-config --libs libpng)

# Add PNG to console libs
CON_LIBS := $(CON_LIBS) -lpng

# Common rules
include $(srcdir)/common.mk
