# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to know our path anyway.
LPATH := tools/data2c

clean-data2cbin:
	-$(RM) tools/data2c/data2c$(BUILDEXEEXT)

clean: clean-data2cbin

$(LPATH)/data2c$(BUILDEXEEXT): $(srcdir)/$(LPATH)/data2c.c
	@$(MKDIR) $(@D)
	$(CC_FOR_BUILD) -o $@ $<

.PHONY: clean-data2cbin
