# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/gimp-plugin

LSRC := $(wildcard $(LPATH)/*.cpp)
LPRODUCTS :=

include $(srcdir)/objects.mk

pentshp_OBJ = \
	$(CONVERT) \
	$(MISC) \
	filesys/FileSystem.o \
	graphics/Palette.o \
	graphics/Shape.o \
	graphics/ShapeFrame.o

$(LPATH)/pentshp$(EXEEXT): $(pentshp_OBJ) $(LPATH)/pentshp.o
	$(CXX) -g -o $@ $+ $(LDFLAGS) $(GIMP_LIBS)

$(LPATH)/pentshp.o: $(LPATH)/pentshp.cpp
	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(WARNINGS) $(CXXFLAGS) $(SDL_CFLAGS) $(GIMP_INCLUDES) -c -o $@ $<

# Common rules
include $(srcdir)/common.mk

all-$(LPATH): $(LPATH)/pentshp$(EXEEXT) $(LOBJ)
