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
# Last modified: Sun 31 Jan 2010 23:13:05 EET too

use strict;
use warnings;
use integer;

$| = 1;

my ($pbx, $pby) = (4, 1); # button x&y, for reset.
my $pbs = 0; # button state
my $bv = 0; # button value

my ($pmx, $pmy, $pmv) = ( 9, 9, undef ); # old multi accidentaly overwritten.

my @table;
for (1..9)
{
    push @table, [ 0, 0, 0,  0, 0, 0,  0, 0, 0 ];
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
	$table[$i / 9][$i % 9] = 0 - $_;
	$i++;
    }
}

sub send_puzzle()
{
    my @list;
    for (my $i = 0; $i < 9; $i++) {
	for (my $j = 0; $j < 9; $j++) {
	    my $v = - $table[$i][$j];
	    if ($v != 0) {
		push @list, "#$i$j/$v";
	    }
	}
    }
    print "@list\n";
}

gen_puzzle;
send_puzzle;
print "*41+\n";

sub num_match($$)
{
    my $v = $table[$_[0]][$_[1]];
    return 0 if ref $v;
    return 1 if $v == $bv || $v == -$bv;
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
    print STDERR "-- $bv --perl input: $_";
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
	$bv = $x + $y * 5 + 1;
	$bv = 0 if $bv > 9;
    }
    elsif ($w eq '#') {
	my $v = $table[$x][$y];
	if ($pbs) { # multi...
	    unless (ref $v) {
		next if $v != 0;
		$v = [ '', '', '', '', '', '', '', '', '', 0 ];
		$table[$x][$y] = $v;
	    }
	    if ($v->[$bv]) { $table[$x][$y]->[$bv] = ''; }
	    else { $table[$x][$y]->[$bv] = $bv; }
	    print "#$x$y.", join '', @{$table[$x][$y]}, "\n";
	    next;
	}
	# else
	if (! ref $v) {
	    next if $v < 0; # initial value.
	    if ($bv && $v > 0) {
		if ($bv == $v && $pmx == $x && $pmy == $y) {
		    $table[$x][$y] = $pmv;
		    print "#$x$y.", join '', @{$pmv}, "\n";
		}
		next;
	    }
	}
	next unless $bv == 0 || number_fit $x, $y;
	$pmx = $x, $pmy = $y, $pmv = $v if ref $v;
	print "#$x$y+$bv\n";
	$table[$x][$y] = $bv;
    }
}
