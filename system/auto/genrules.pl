#!/usr/bin/perl -w

use strict;

#
# This file is invoked by the Makefile and generates make rules
# for end products (usually application binaries).
# This currently only uses $(CXX) for the linking, but could be
# customized for other things, too.
#

# passing '--gui' means all following products are built as full GUI apps
# passing '--con' means all following products are built as console apps
# (and they cancel eachother)

my $gui = 1;

print "# This file is generated automatically by genrules.pl. Do not edit!\n\n";
foreach my $product (@ARGV) {
	if ($product eq "--gui") {
		$gui = 1;
		next;
	}
	if ($product eq "--con") {
		$gui = 0;
		next;
	}

	# Generate the rules for each product
	print 'ifneq ($(LPATH)$(EXEEXT),.)'."\n";
	print $product.': $(LPATH)/'.$product.'$(EXEEXT)'."\n";
	print "endif\n";
	print '$(LPATH)/'.$product.'$(EXEEXT): $('.$product.'_OBJ)'."\n";
	if ($gui == 1) {
		print "\t".'$(CXX) -g -o $@ $+ $(LDFLAGS) $(SYS_LIBS) $(GUI_LIBS)'."\n";
	} else {
		print "\t".'$(CXX) -g -o $@ $+ $(LDFLAGS) $(SYS_LIBS) $(CON_LIBS)'."\n";
	}

	print "\n";
}
