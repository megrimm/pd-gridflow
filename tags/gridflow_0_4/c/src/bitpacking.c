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

#include "grid.h"

typedef uint8 *(*Packer)(BitPacking *$, int n, const Number *in, uint8 *out);
typedef Number *(*Unpacker)(BitPacking *$, int n, const uint8 *in, Number *out);

struct BitPacking {
	uint32 mask[3];
	int bytes;
	Packer pack;
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
uint8 *default_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	register uint32 t;
	int hb[3] = {
		high_bit($->mask[0]),
		high_bit($->mask[1]),
		high_bit($->mask[2])};
	int mask[3];
	memcpy(mask,$->mask,3*sizeof(int));
	if ($->bytes==2) {
		while (n>3) {
			t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
			t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
			t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
			t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
			n-=4;
		}
		while (n--) {
			t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
		}
	} else if ($->bytes==3) {
		while (n > 3) {
			t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;
			t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;
			t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;
			t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;
			n-=4;
		}
		while (n--) {
			t=CONVERT1; *((short *)out)=t; out[2]=t>>16; out+=3; in+=3;
		}
	} else if ($->bytes==4) {
		while (n > 3) {
			t=CONVERT1; *((long *)out)=t; out+=4; in+=3;
			t=CONVERT1; *((long *)out)=t; out+=4; in+=3;
			t=CONVERT1; *((long *)out)=t; out+=4; in+=3;
			t=CONVERT1; *((long *)out)=t; out+=4; in+=3;
			n-=4;
		}
		while (n--) {
			t=CONVERT1; *((long *)out)=t; out+=4; in+=3;
		}
	} else {
		while (n--) {
			int bytes = $->bytes;
			t = CONVERT1;
			while (bytes--) { *out++ = t; t >>= 8; }
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
		t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
		t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
		t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
		t=CONVERT1; *((short *)out)=t; out+=2; in+=3;
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
		out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3;
		out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3;
		out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3;
		out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3;
		n-=4;
	}
	while (n--) {
		out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3;
	}
	return out;
}

BitPacking builtin_bitpacks[] = {
	{ {0x0000f800,0x000007e0,0x0000001f}, 2, pack_5652 },
	{ {0x00ff0000,0x0000ff00,0x000000ff}, 3, pack_8883 },
};

/* **************************************************************** */

BitPacking *BitPacking_new(int bytes, uint32 r, uint32 g, uint32 b) {
	int i;
	BitPacking *$ = NEW(BitPacking,1);
	$->bytes = bytes;
	$->mask[0] = r;
	$->mask[1] = g;
	$->mask[2] = b;
	$->pack = default_pack;

	for (i=0; i<(int)(sizeof(builtin_bitpacks)/sizeof(BitPacking)); i++) {
		BitPacking *bp = builtin_bitpacks+i;
		if (bp->bytes == bytes &&
		bp->mask[0]==r && bp->mask[1]==g && bp->mask[2]==b) {
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

int BitPacking_bytes(BitPacking *$) {
	return $->bytes;
}

uint8 *BitPacking_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	return $->pack($,n,in,out);
}

Number *BitPacking_unpack(BitPacking *$, int n, const uint8 *in, Number *out) {
	int hb[3];
	hb[0] = high_bit($->mask[0]);
	hb[1] = high_bit($->mask[1]);
	hb[2] = high_bit($->mask[2]);
	while (n--) {
		int bytes=0, temp=0;
		/* while(bytes--) { temp = (temp<<8) | *in++; } *//* BE */
		while($->bytes>bytes) { temp |= *in++ << (bytes++)*8; } /* LE */
		*out++ = ((temp & $->mask[0]) << 7) >> hb[0];
		*out++ = ((temp & $->mask[1]) << 7) >> hb[1];
		*out++ = ((temp & $->mask[2]) << 7) >> hb[2];
	}
	return out;
}
