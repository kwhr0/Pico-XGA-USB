#! /usr/bin/perl

$yn = 12;

open(S, "mplus_bitmap_fonts/fonts_e/mplus_f${yn}r.bdf") || die;
while (<S>) {
	if (/^ENCODING\s+(\d+)/) {
		$a = $yn * ($1 - 0x20);
		$count = 0;
	}
	elsif ($a >= 0 && $a < 0x1000 && /^[0-9A-Fa-f]{2}$/ && $count++) {
		$t = hex($&);
		$d[$a++] = $t >> 7 & 1 | $t >> 5 & 2 | $t >> 3 & 4 | $t >> 1 & 8 | $t << 1 & 0x10 | $t << 3 & 0x20 | $t << 5 & 0x40 | $t << 7 & 0x80;
	}
}
close S;
open(D, "> font.c") || die;
printf D "const unsigned char gFont[] = {\n";
for ($i = 0; $i < @d; $i++) {
	printf D "0x%02x,", $d[$i];
	printf D "//0x%02x\n", $i / $yn + 0x20 if $i % $yn == $yn - 1;
}
printf D "};\n";
close D;
exit 0;
