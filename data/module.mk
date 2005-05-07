# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := data

LSRC := 
LPRODUCTS := 

DATA_FILES := \
	data/fixedfont.ini \
	data/fixedfont.tga \
	data/u8armour.ini \
	data/u8bindings.ini \
	data/u8french.ini \
	data/u8german.ini \
	data/u8.ini \
	data/u8monsters.ini \
	data/u8spanish.ini \
	data/u8weapons.ini \
	data/VeraBd.ttf \
	data/Vera.ttf

$(LPATH)/data.h: $(top_builddir)/tools/data2c/data2c$(BUILDEXEEXT) $(DATA_FILES)
	@$(MKDIR) $(@D)
	$(top_builddir)/tools/data2c/data2c$(BUILDEXEEXT) $@ $(patsubst %,$(top_srcdir)/%,$(DATA_FILES))

# Common rules
include $(srcdir)/common.mk
