# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/shapeconv

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS := shapeconv

shapeconv_OBJ = ShapeConv.o ../../misc/Args.o ../../filesys/FileSystem.o ../../misc/Console.o ../../misc/Q_strcasecmp.o ../../convert/u8/ConvertShapeU8.o ../../convert/crusader/ConvertShapeCrusader.o ../../convert/ConvertShape.o

# Common rules
include common.mk
