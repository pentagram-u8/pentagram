#!/usr/bin/perl -w

use strict;

#
# This file is invoked by the Makefile and generates make rules
# for end products (usually application binaries).
# This currently only uses $(CXX) for the linking, but could be
# customized for other things, too.
#

print "# This file is generated automatically by genrules.pl. Do not edit!\n\n";
foreach my $product (@ARGV) {
	# Generate the rules for each product
	print $product.'_OBJ := $(patsubst %,$(LPATH)/%,$('.$product.'_OBJ))'."\n";
	print $product.': $(LPATH)/'.$product.'$(EXEEXT)'."\n";
	print '$(LPATH)/'.$product.'$(EXEEXT): $('.$product.'_OBJ)'."\n";
	print '	$(CXX) -g -o $@ $+ $(LDFLAGS) $(SDL_LIBS)'."\n";
	print "\n";
}
