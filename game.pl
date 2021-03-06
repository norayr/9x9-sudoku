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
# Last modified: Tue 20 Jul 2010 17:55:04 EEST too

use strict;
use warnings;
use integer;

$| = 1;

my ($pbx, $pby) = (4, 1); # button x&y, for reset.
my $pbs = 0; # button state
my $bv = 0; # button value
my $level = 0;
my $time = time;

my (@table, $pmx, $pmy, $pmv);
sub init_puzzle() 
{
    ($pmx, $pmy, $pmv) = ( 9, 9, undef ); # old multi accidentaly overwritten.
    @table = ();
    push @table, [ 0, 0, 0,  0, 0, 0,  0, 0, 0 ] for (1..9);

}

# table content: negative -- generated, positive -- user input, ref -- multiple

sub read_puzzle()
{
    init_puzzle;
    open I, '<', "$ENV{HOME}/.config/9x9-sudoku.data" or return 0;
    $_ = <I>;
    return 0 unless /^9x9 sudoku data format 1\s/;

    sub asp($) {
	my @a = ( 0, '', '', '', '', '', '', '', '', '');
	foreach (split '', $_[0]) { $a[$_] = $_ + 0 if $_ > 0; }
	return \@a;
    }
    while (<I>) {
	my @c = split;
	my $key = shift @c;
	my $val = shift @c;
	if ($key eq 'row')  { 
	    my @row;
	    for (0..8) {
		if ($c[$_] < 0 || length $c[$_] == 1) { 
		    push @row, $c[$_]; 
		}
		else {
		    push @row, asp $c[$_]; 
		}
	    }
	    $table[$val] = \@row;
	}
	elsif ($key eq 'pbx') { $pbx = $val + 0; }
	elsif ($key eq 'pby') { $pby = $val + 0; }
	elsif ($key eq 'bv')  { $bv  = $val + 0; }
	elsif ($key eq 'pmx') { $pmx = $val + 0; }
	elsif ($key eq 'pmy') { $pmy = $val + 0; }
	elsif ($key eq 'level') { $level = $val + 0; }
	elsif ($key eq 'time') { $time = time - $val; }
	elsif ($key eq 'pmv') { $pmv = asp $val; }
    }
    return 1;
}    

sub gen_puzzle($)
{
    init_puzzle;
    my $line = int (rand 900) + 1;
    $level = $_[0] * 1000 + $line;
    chomp ($line = qx(gzip -dc pzl$_[0].gz | sed -n ${line}p));
    $line =~ s/^\S+\s+//;
    #print "$line\n"; 
    my @line = split //, $line;
    my $i = 0;
    foreach (@line) {
	$table[$i / 9][$i % 9] = 0 - $_;
	$i++;
    }
    $time = time;
}

sub clear_puzzle()
{
    for (my $i = 0; $i < 9; $i++) {
	for (my $j = 0; $j < 9; $j++) {
	    my $v = $table[$i][$j];
	    $table[$i][$j] = 0, next if ref $v || $v > 0;
	}
    }
    $time = time;
}

sub send_time()
{
    my $m = time - $time;
    my $s = $m % 60;
    $m = $m / 60;
    printf "^ %7d:%02d\n", $m, $s;
}

sub send_puzzle($)
{
    my @list;
    for (my $i = 0; $i < 9; $i++) {
	for (my $j = 0; $j < 9; $j++) {
	    my $v = $table[$i][$j];
	    if (ref $v) {
		push @list, "#$i$j." . join '', @{$v};
	    }
	    elsif ($v < 0) { $v = -$v; push @list, "#$i$j/$v"; }
	    elsif ($v > 0) { push @list, "#$i$j+$v"; }
	}
    }
    my $l2 = int ($level / 1000);
    print "> $l2 ", $level - $l2 * 1000 + 99, "\n";
    print "@\n" if $_[0];
    print "@list\n";
}

read_puzzle or gen_puzzle 1;
send_puzzle 0;
print "*$pbx$pby", $pbs? '.': '+', "\n";
send_time;

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
	    #print "*$x$y+\n";
	    $pbx = $x; $pby = $y; $pbs = !$pbs; # keep same.
	}
	if ($pbs) { print "*$x$y+\n"; $pbs = 0; }
	else { print "*$x$y.\n"; $pbs = 1; }
	$bv = $x + $y * 5 + 1;
	$bv = 0 if $bv > 9;
    }
    elsif ($w eq '#') {
	my $v = $table[$x][$y];
	if ($pbs && $bv) { # multi... (and not [ ] button)
	    unless (ref $v) {
		next if $v != 0;
		$v = [ 0, '', '', '', '', '', '', '', '', '' ];
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
	    if ($bv == $v) {
		if ($pmx == $x && $pmy == $y) {
		    $table[$x][$y] = $pmv;
		    print "#$x$y.", join '', @{$pmv}, "\n";
		}
		else {
		    $table[$x][$y] = 0;
		    print "#$x$y+0\n";
		}
		next;
	    }
	    next if $v > 0 && $bv != 0;
	}
	if ($bv != 0 && ! number_fit $x, $y) {
	    if (ref $v) {
		$table[$x][$y]->[$bv] = '';
		print "#$x$y.", join '', @{$table[$x][$y]}, "\n";
	    }
	    next;
	}
	$pmx = $x, $pmy = $y, $pmv = $v if ref $v;
	print "#$x$y+$bv\n";
	$table[$x][$y] = $bv;
    }
    elsif ($w eq '@') { # new game
	$x += 0;
	next if $x < 0 || $x > 5;
	if ($x == 0) {	clear_puzzle; }
	else {		gen_puzzle $x; }
	send_puzzle 1;
	print "*$pbx$pby", $pbs? '.': '+', "\n";
    }
    elsif ($w eq '-') { # active again, stop time
	my $ct = time;
	next if $ct < $x;
	my $t = $ct - $x;
	next if $t > $ct - $time;
	$time += $t;
    }
} continue { send_time; }

# write puzzle after eof.

chdir $ENV{'HOME'} or die $!;
unless (-d '.config') {
    mkdir '.config' or die $!;
}
chdir '.config';
open O, '>', '9x9-sudoku.data' or die $!;
select O;
print "9x9 sudoku data format 1\n";
print "pbx $pbx\n", "pby $pby\n", "bv $bv\n", "pmx $pmx\n", "pmy $pmy\n";
$time = time - $time; $time = 0 if $time < 0 or $time > 1e8;
print "level $level\n", 'time ', $time, "\n";
print 'pmv ', join '', @{$pmv}, "\n" if ref $pmv;

my ($i, $tcv);
for (0..8) {
    print "row $_ ";
    for ($i = 0; $i < 9; $i++) {
	$tcv = $table[$_][$i];
	if (ref $tcv) { print join '', @{$tcv}, ' '; }
	else { print $tcv, ' '; }
    }
    print "\n";
}
