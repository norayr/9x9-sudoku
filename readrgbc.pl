#!/usr/bin/perl

use strict;
use warnings;

my $pfx = shift @ARGV;

open C, '>', "$pfx.c" or die $!;
open H, '>', "$pfx.h" or die $!;

my ($w, $h, $b );

while (<>) {
    ($w, $h, $b) = ( $1, $2, $3), last if /(\d+),\s+(\d+),\s+(\d+)/;
}

print H "extern int ${pfx}_width;\n";
print H "extern int ${pfx}_height;\n";
print H "extern int ${pfx}_bytes;\n\n";

print H "extern const unsigned char ${pfx}_pixel_data[ $w * $h * $b + 1 ];\n";

print C "#include \"$pfx.h\"\n\n";

print C "int ${pfx}_width = $w;\n";
print C "int ${pfx}_height = $h;\n";
print C "int ${pfx}_bytes = $b;\n\n";

print C "const unsigned char ${pfx}_pixel_data[ $w * $h * $b + 1 ] =\n";

while (<>) {
    last if s/,\s*$//;
    print C $_;
}
print C "$_;\n";
