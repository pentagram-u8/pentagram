# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := data

$(LPATH)/data.h: $(top_builddir)/tools/data2c/data2c$(BUILDEXEEXT) $(DATA_FILES)
	@$(MKDIR) $(@D)
	$(top_builddir)/tools/data2c/data2c$(BUILDEXEEXT) $@ $(patsubst %,$(top_srcdir)/%,$(DATA_FILES))

# Common rules
include $(srcdir)/common.mk
