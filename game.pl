#!/usr/bin/perl
#
# $Id; game.pl $
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2010 Tomi Ollila
#	    All rights reserved
#
# Created: Sat 30 Jan 2010 20:16:55 EET too
# Last modified: Sat 30 Jan 2010 21:08:14 EET too

use strict;
use warnings;

$| = 1;

#print "foo\n";
#print "ar\n";

while (<STDIN>) {
    my ($w, $x, $y, @r) = split;
    print "zap $_";
}

