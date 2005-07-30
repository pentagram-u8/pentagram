# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/shapeconv

LSRC := $(wildcard $(srcdir)/$(LPATH)/*.cpp)
LCONPRODUCTS := shapeconv

shapeconv_OBJ = \
	tools/shapeconv/ShapeConv.o \
	$(FILESYS) \
	$(MISC) \
	$(GAMES) \
	$(CONF) \
	$(ARGS) \
	$(CONVERT) \
	kernel/CoreApp.o

# Common rules
include $(srcdir)/common.mk
