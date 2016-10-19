#!/usr/bin/perl
use MIME::Base64::Perl;
my $string;
open(INPUT,"<input.b64") or die("failed to open input.\n");
open(OUTPUT,">output.jpg") or die("failed to open output.\n");
while(<INPUT>) {
    $string.=chomp($_);
}
print OUTPUT decode_base64($string);
