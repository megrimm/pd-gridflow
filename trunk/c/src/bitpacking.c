/*
	$Id$

	Video4jmax
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

struct BitPacking {
	int n;
	int bytes;
	uint32 mask[3];
	int low_bit[3];
	int high_bit[3];
};

// returns the highest bit set in a word, or -1 if none
int high_bit(uint32 n) {
	int i;
	for (i=31; ((n & 0x80000000) == 0) && i>=0; i--) n <<= 1;
	return i;
}

// returns the lowest bit set in a word, or -1 if none
int low_bit(uint32 n) {
	return high_bit((~n+1)&n);
}

BitPacking *BitPacking_new(int bytes, uint32 r, uint32 g, uint32 b) {
	BitPacking *$ = NEW(BitPacking,1);
	int i;
	$->n = 3;
	$->bytes = bytes;
	$->mask[0] = r;
	$->mask[1] = g;
	$->mask[2] = b;
	for (i=0; i<3; i++) {
		$->low_bit[i]  =  low_bit($->mask[i]);
		$->high_bit[i] = high_bit($->mask[i]);
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
			$->low_bit[i],
			$->high_bit[i]);
	}
}

int BitPacking_bytes(BitPacking *$) {
	return $->bytes;
}

/*
  This piece of code is little-endian specific, sorry.
  Will fix it when I get asked for it.
*/
uint8 *BitPacking_pack(BitPacking *$, int n, const Number *in, uint8 *out) {
	while (n--) {
		unsigned int temp =
			(((in[0] << $->high_bit[0]) >> 7) & $->mask[0]) |
			(((in[1] << $->high_bit[1]) >> 7) & $->mask[1]) |
			(((in[2] << $->high_bit[2]) >> 7) & $->mask[2]);
		int bytes = $->bytes;
		while(bytes--) { *out++ = temp; temp >>= 8; }
		in += 3;
	}
	return out;
}

Number *BitPacking_unpack(BitPacking *$, int n, const uint8 *in, Number *out) {
	while (n--) {
		int bytes=0, temp=0;
		/* while(bytes--) { temp = (temp<<8) | *in++; } *//* BE */
		while($->bytes>bytes) { temp |= *in++ << (bytes++)*8; } /* LE */
		*out++ = ((temp & $->mask[0]) << 7) >> $->high_bit[0];
		*out++ = ((temp & $->mask[1]) << 7) >> $->high_bit[1];
		*out++ = ((temp & $->mask[2]) << 7) >> $->high_bit[2];
	}
	return out;
}
