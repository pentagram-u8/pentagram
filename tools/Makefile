LPATH := $(shell basename `pwd`)

all: all-$(LPATH) ;
clean: clean-$(LPATH) ;

# Catch all rule - call Makefile in parent dir
%::
	cd .. && $(MAKE) $@

.PHONY: all clean
