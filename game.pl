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
# Last modified: Sun 31 Jan 2010 19:23:24 EET too

use strict;
use warnings;
use integer;

$| = 1;

my $pbx = 4;
my $pby = 1;
my $pbs = 0;
my $pbv = 0;

my (@tablep, @tablec);
for (1..9)
{
    push @tablep, [ 0, 0, 0,  0, 0, 0,  0, 0, 0 ];
    push @tablec, [ 0, 0, 0,  0, 0, 0,  0, 0, 0 ];
}

sub gen_puzzle()
{
    open I, '<', 'precalc' or die;
    my $line = int (rand 1000);
    while (<I>)
    {
	chomp ($line = $_), last if $. == $line;
    }
    close I;
    $line =~ s/^\S+\s+//;
    #print "$line\n";
    my @line = split //, $line;
    my $i = 0;
    foreach (@line) {
	$tablec[$i / 9][$i % 9] = $_ + 0;
	$tablep[$i / 9][$i % 9] = $_ + 0;
	$i++;
    }
}

sub send_puzzle()
{
    my @list;
    for (my $i = 0; $i < 9; $i++) {
	for (my $j = 0; $j < 9; $j++) {
	    my $v = $tablec[$i][$j];
	    if ($v != 0) {
		push @list, "#$i$j/$v";
	    }
	}
    }
    print "@list\n";
}

gen_puzzle;
send_puzzle;

# table value above, positive, negative, or arrayref; use ref() in perl to see

while (<STDIN>) {
    print STDERR "----------------perl input: $_\n";
    my ($w, $x, $y, @r) = split;

    if ($w eq '*') { # button
	if ($pbx != $x || $pby != $y) {
	    print "*$pbx$pby/\n";
	    print "*$x$y+\n";
	    $pbx = $x; $pby = $y; $pbs = 0;
	}
	else {
	    if ($pbs) {	print "*$x$y+\n"; $pbs = 0; }
	    else {	print "*$x$y.\n"; $pbs = 1; }
	}
	$pbv = $x + $y * 5 + 1;
	$pbv = 0 if $pbv > 9;
    }
    elsif ($w eq '#') {
	    print "#$x$y+$pbv\n";
    }
}

