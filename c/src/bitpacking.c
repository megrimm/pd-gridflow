/*
	$Id$

	GridFlow
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
  NOTE: most essential code in this file is used very often, and
  so most of the rest of the code is special-case accelerations of it.
*/

#include "grid.h"

typedef uint8 *(*Packer)(BitPacking *$, int n, const Number *in, uint8 *out);
typedef Number *(*Unpacker)(BitPacking *$, int n, const uint8 *in, Number *out);

struct BitPacking {
	Packer pack;
	int endian; /* 0=big, 1=little, 2=same, 3=different */
	int bytes;
	uint32 mask[3];
};

// returns the highest bit set in a word
// ... or -1 if none (no longer, sorry)
int high_bit(uint32 n) {
	int i=0;
	if (n&0xffff0000) { n>>=16; i+=16; }
	if (n&0x0000ff00) { n>>= 8; i+= 8; }
	if (n&0x000000f0) { n>>= 4; i+= 4; }
	if (n&0x0000000c) { n>>= 2; i+= 2; }
	if (n&0x00000002) { n>>= 1; i+= 1; }
	return i;
}

// returns the lowest bit set in a word, or -1 if none
int low_bit(uint32 n) {
	return high_bit((~n+1)&n);
}

/*
  This piece of code is little-endian specific, sorry.
  Will fix it when I get asked for it.
*/
#define CONVERT1 \
	(((in[0] << hb[0]) >> 7) & mask[0]) | \
	(((in[1] << hb[1]) >> 7) & mask[1]) | \
	(((in[2] << hb[2]) >> 7) & mask[2])

#define FOURTIMES(_x_) _x_ _x_ _x_ _x_

#define NTIMES(_x_) \
	while (n>3) { FOURTIMES(_x_) n-=4; } \
	while (n--) { _x_ }

uint8 *default_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	register uint32 t;
	int hb[3] = {
		high_bit($->mask[0]),
		high_bit($->mask[1]),
		high_bit($->mask[2])};
	int mask[3];
	int sameorder = $->endian==2 || $->endian==is_le();
	memcpy(mask,$->mask,3*sizeof(int));
	if (sameorder && $->bytes==2) {
		NTIMES(t=CONVERT1; *((short *)out)=t; out+=2; in+=3;)
	} else if (sameorder && $->bytes==3) {
		NTIMES(t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;)
	} else if (sameorder && $->bytes==4) {
		NTIMES(t=CONVERT1; *((long *)out)=t; out+=4; in+=3;)
	} else if (BitPacking_is_le($)) {
		/* smallest byte first */
		whine("hello!");
		while (n--) {
			int bytes = $->bytes;
			t = CONVERT1;
			while (bytes--) { *out++ = t; t >>= 8; }
			in+=3;
		}
	} else {
		/* largest byte first */
		while (n--) {
			int bytes = $->bytes;
			t = CONVERT1;
			while (bytes--) { out[bytes] = t; t >>= 8; }
			out += $->bytes;
			in+=3;
		}
	}
	return out;
}

/* **************************************************************** */

uint8 *pack_5652(BitPacking *$, int n, const Number *in, uint8 *out) {
	const int hb[3] = {15,10,4};
	const int mask[3] = {0x0000f800,0x000007e0,0x0000001f};
	register uint32 t;
	while (n>3) {
		FOURTIMES( t=CONVERT1; *((short *)out)=t; out+=2; in+=3; )
		n-=4;
	}
	while (n--) {
		t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
	}
	return out;
}

uint8 *pack_8883(BitPacking *$, int n, const Number *in, uint8 *out) {
	const int hb[3] = {23,15,7};
	const int mask[3] = {0x00ff0000,0x0000ff00,0x000000ff};
	register uint32 t;
	while (n>3) {
		FOURTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
		n-=4;
	}
	while (n--) {
		out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3;
	}
	return out;
}

BitPacking builtin_bitpacks[] = {
	{ pack_5652, 2, 2, {0x0000f800,0x000007e0,0x0000001f} },
	{ pack_8883, 1, 3, {0x00ff0000,0x0000ff00,0x000000ff} },
};

/* **************************************************************** */

static bool BitPacking_eq(BitPacking *$, BitPacking *o) {
	if (!($->bytes == o->bytes)) return false;
	if (!($->mask[0] == o->mask[0] &&
		$->mask[1] == o->mask[1] &&
		$->mask[2] == o->mask[2])) return false;
	if ($->endian==o->endian) return true;
	/* same==little on a little-endian; same==big on a big-endian */
	return ($->endian ^ o->endian ^ is_le()) == 2;
}

BitPacking *BitPacking_new(int endian, int bytes, uint32 r, uint32
g, uint32 b) {
	int i;
	BitPacking *$ = NEW(BitPacking,1);
	$->endian = endian;
	$->bytes = bytes;
	$->mask[0] = r;
	$->mask[1] = g;
	$->mask[2] = b;
	$->pack = default_pack;

	for (i=0; i<(int)(sizeof(builtin_bitpacks)/sizeof(BitPacking)); i++) {
		BitPacking *bp = builtin_bitpacks+i;
		if (BitPacking_eq($,bp)) {
			$->pack = bp->pack;
			whine("Bitpacking: will be using special packer (#%d)",i);
		}
	}
	return $;
}

void BitPacking_whine(BitPacking *$) {
	int i;
	whine("BitPacking:");
	whine("    bytes: %d", $->bytes);
	for (i=0;i<3;i++) {
		static const char *colour_name[] = {"red","green","blue"};
		whine("    mask[%5s]: %08x (bits from %2d up to %2d)",
			colour_name[i],
			$->mask[i],
			low_bit($->mask[i]),
			high_bit($->mask[i]));
	}
}

int  BitPacking_bytes(BitPacking *$) { return $->bytes; }
bool BitPacking_is_le(BitPacking *$) {
	return $->endian==1 || ($->endian ^ is_le())==3; }

uint8 *BitPacking_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	return $->pack($,n,in,out);
}

Number *BitPacking_unpack(BitPacking *$, int n, const uint8 *in, Number *out) {
	int hb[3];
	hb[0] = high_bit($->mask[0]);
	hb[1] = high_bit($->mask[1]);
	hb[2] = high_bit($->mask[2]);
#define LOOP_UNPACK(_reader_) \
	while (n--) { \
		int bytes=0, temp=0; \
		_reader_; \
		*out++ = ((temp & $->mask[0]) << 7) >> hb[0]; \
		*out++ = ((temp & $->mask[1]) << 7) >> hb[1]; \
		*out++ = ((temp & $->mask[2]) << 7) >> hb[2]; \
	}

	if (BitPacking_is_le($)) {
		/* smallest byte first */
		LOOP_UNPACK(while($->bytes>bytes) { temp |= *in++ << (bytes++)*8; })
	} else {
		/* biggest byte first */
		LOOP_UNPACK(while(bytes--) { temp = (temp<<8) | *in++; })
	}
	return out;
}
