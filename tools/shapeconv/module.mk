# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/shapeconv

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LPRODUCTS := 
#shapeconv

shapeconv_OBJ = \
	tools/shapeconv/ShapeConv.o \
	filesys/FileSystem.o \
	misc/Args.o \
	misc/Console.o \
	misc/Q_strcasecmp.o \
	convert/u8/ConvertShapeU8.o \
	convert/crusader/ConvertShapeCrusader.o \
	convert/ConvertShape.o \
	kernel/CoreApp.o

# Common rules
include $(srcdir)/common.mk
