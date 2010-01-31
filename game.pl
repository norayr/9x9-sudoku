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
# Last modified: Sun 31 Jan 2010 20:37:39 EET too

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

# table content: negative -- generated, positive -- user input, ref -- multiple

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
	$tablec[$i / 9][$i % 9] = 0 - $_;
	$tablep[$i / 9][$i % 9] = 0 - $_;
	$i++;
    }
}

sub send_puzzle()
{
    my @list;
    for (my $i = 0; $i < 9; $i++) {
	for (my $j = 0; $j < 9; $j++) {
	    my $v = - $tablec[$i][$j];
	    if ($v != 0) {
		push @list, "#$i$j/$v";
	    }
	}
    }
    print "@list\n";
}

gen_puzzle;
send_puzzle;

sub num_match($$)
{
    my $v = $tablec[$_[0]][$_[1]];
    return 0 if ref $v;
    return 1 if $v == $pbv || $v == -$pbv;
    return 0;
}

sub number_fit($$)
{
    my ($x, $y) = @_;
    my ($i, $j);
    
    for ($i = 0; $i < 9; $i++) {
	return 0 if num_match $x, $i;
    }
    for ($i = 0; $i < 9; $i++) {
	return 0 if num_match $i, $y;
    }
    $x = int ($x / 3) * 3;
    $y = int ($y / 3) * 3;
    for ($i = 0; $i < 3; $i++) {
	for ($j = 0; $j < 3; $j++) {
	    return 0 if num_match $x + $i, $y + $j;	    
	}}
    return 1;
}

while (<STDIN>) {
    #print STDERR "-- $pbv --perl input: $_";
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
	my $v = $tablec[$x][$y];
	next if ( ! ref $v && $v < 0);
	print "$pbv $v\n";
	if ($pbv == $v) {
	    $v = $tablep[$x][$y];
	    $tablec[$x][$y] = $tablep[$x][$y];
	    print "#$x$y+$v\n";
	    next;
	}
	next unless $pbv == 0 || number_fit $x, $y;
	print "#$x$y+$pbv\n";
	$tablep[$x][$y] = $tablec[$x][$y];
	$tablec[$x][$y] = $pbv;
    }
}
