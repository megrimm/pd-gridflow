/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "grid.h.fcs"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define CONVERT1 t = \
	(((in[0] << hb[0]) >> 7) & mask[0]) | \
	(((in[1] << hb[1]) >> 7) & mask[1]) | \
	(((in[2] << hb[2]) >> 7) & mask[2])

#define CONVERT2 \
	for (t=0,i=0; i<self->size; i++) t |= (((in[i] << hb[i]) >> 7) & mask[i]);

#define CONVERT3 \
	for (t=0,i=0; i<self->size; i++) { \
		t |= ((in[i]>>(7-hb[i]))|(in[i]<<(hb[i]-7))) & mask[i]; \
	}

#define WRITE_LE \
	for (int bytes = self->bytes; bytes; bytes--, t>>=8) *out++ = t;

#define WRITE_BE { int bytes; \
	for (bytes = self->bytes; bytes; bytes--, t>>=8) out[bytes] = t; \
	out += self->bytes; }

/* this macro would be faster if the _increment_
   was done only once every loop. or maybe gcc does it, i dunno */
#define NTIMES(_x_) \
	for (; n>3; n-=4) { _x_ _x_ _x_ _x_ } \
	for (; n; n--) { _x_ }

template <class T>
static void default_pack(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	uint32 t;
	int i;
	int hb[4];
	uint32 mask[4];
	int sameorder = self->endian==2 || self->endian==::is_le();
	int size = self->size;

	for (i=0; i<self->size; i++) hb[i] = highest_bit(self->mask[i]);
	memcpy(mask,self->mask,size*sizeof(uint32));

	if (sameorder && size==3) {
		switch(self->bytes) {
		case 2:
			NTIMES(t=CONVERT1; *((int16 *)out)=t; out+=2; in+=3;)
			return;
		case 4:
			NTIMES(t=CONVERT1; *((int32 *)out)=t; out+=4; in+=3;)
			return;
		}
	}
	
	if (self->is_le()) {
		if (size==3)
			for (; n--; in+=3) {CONVERT1; WRITE_LE;}
		else if (size==4) {
			for (; n--; in+=4) {CONVERT3; WRITE_LE;}
		} else
			for (; n--; in+=size) {CONVERT2; WRITE_LE;}
	} else {
		if (size==3)	
			for (; n--; in+=3) {CONVERT1; WRITE_BE;}
		else if (size==4)
			for (; n--; in+=4) {CONVERT3; WRITE_BE;}
		else
			for (; n--; in+=size) {CONVERT2; WRITE_BE;}
	}
}

#define LOOP_UNPACK(_reader_) \
	for (; n; n--) { \
		int bytes=0; uint32 temp=0; _reader_; \
		for (int i=0; i<self->size; i++, out++) { \
			uint32 t=temp&self->mask[i]; \
			*out = (t<<(7-hb[i]))|(t>>(hb[i]-7)); \
		} \
	}

//			*out++ = ((temp & self->mask[i]) << 7) >> hb[i];

template <class T>
static void default_unpack(BitPacking *self, int n, Pt<uint8> in, Pt<T> out) {
	int hb[4];
	for (int i=0; i<self->size; i++) hb[i] = highest_bit(self->mask[i]);

	if (is_le()) {
		/* smallest byte first */
		LOOP_UNPACK(
			for(; self->bytes>bytes; bytes++, in++) temp |= *in<<(8*bytes);
		)
	} else {
		/* biggest byte first */
		LOOP_UNPACK(
			bytes=self->bytes; for (; bytes; bytes--, in++) temp=(temp<<8)|*in;
		)
	}
}

/* **************************************************************** */

template <class T>
static void pack2_565(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	const int hb[3] = {15,10,4};
	const uint32 mask[3] = {0x0000f800,0x000007e0,0x0000001f};
	uint32 t;
	NTIMES( t=CONVERT1; *((short *)out)=t; out+=2; in+=3; )
}

template <class T>
static void pack3_888(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}

static uint32 bp_masks[][4] = {
	{0x0000f800,0x000007e0,0x0000001f,0},
	{0x00ff0000,0x0000ff00,0x000000ff,0},
};

static Packer bp_packers[] = {
	{default_pack, default_pack, default_pack},
	{pack2_565, pack2_565, pack2_565},
	{pack3_888, pack3_888, pack3_888},
};

static Unpacker bp_unpackers[] = {
	{default_unpack, default_unpack, default_unpack},
};	

static BitPacking builtin_bitpackers[] = {
	BitPacking(2, 2, 3, bp_masks[0], &bp_packers[1], &bp_unpackers[0]),
	BitPacking(1, 3, 3, bp_masks[1], &bp_packers[2], &bp_unpackers[0]),
};

/* **************************************************************** */

bool BitPacking::eq(BitPacking *o) {
	if (!(bytes == o->bytes)) return false;
	if (!(size == o->size)) return false;
	for (int i=0; i<size; i++) {
		if (!(mask[0] == o->mask[0])) return false;
	}
	if (endian==o->endian) return true;
	/* same==little on a little-endian; same==big on a big-endian */
	return (endian ^ o->endian ^ ::is_le()) == 2;
}

BitPacking::BitPacking(int endian, int bytes, int size, uint32 *mask,
Packer *packer, Unpacker *unpacker) {
	this->endian = endian;
	this->bytes = bytes;
	this->size = size;
	for (int i=0; i<size; i++) this->mask[i] = mask[i];
	if (packer) {
		this->packer = packer;
		this->unpacker = unpacker;
		return;
	}
	this->packer = &bp_packers[0];
	this->unpacker = &bp_unpackers[0];

	for (int i=0; i<(int)(sizeof(builtin_bitpackers)/sizeof(BitPacking)); i++) {
		BitPacking *bp = builtin_bitpackers+i;
		if (this->eq(bp)) {
			this->packer = bp->packer;
			this->unpacker = bp->unpacker;
			//::gfpost("Bitpacking: will be using special packer (#%d)",i);
		}
	}
}

bool BitPacking::is_le() {
	return endian==1 || (endian ^ ::is_le())==3;
}

template <class T>
void BitPacking::pack(int n, Pt<T> in, Pt<uint8> out) {
	switch (NumberTypeE_type_of(*in)) {
	case uint8_type_i: packer->as_uint8(this,n,(Pt<uint8>)in,out); break;
	case int16_type_i: packer->as_int16(this,n,(Pt<int16>)in,out); break;
	case int32_type_i: packer->as_int32(this,n,(Pt<int32>)in,out); break;
	default: RAISE("argh");
	}
}

template <class T>
void BitPacking::unpack(int n, Pt<uint8> in, Pt<T> out) {
	switch (NumberTypeE_type_of(*out)) {
	case uint8_type_i: unpacker->as_uint8(this,n,in,(Pt<uint8>)out); break;
	case int16_type_i: unpacker->as_int16(this,n,in,(Pt<int16>)out); break;
	case int32_type_i: unpacker->as_int32(this,n,in,(Pt<int32>)out); break;
	default: RAISE("argh");
	}
}

/* this could be faster (use asm) */
void swap32 (int n, Pt<uint32> data) {
	NTIMES({
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	})
}

/* this could be faster (use asm or do it in int32 chunks) */
void swap16 (int n, Pt<uint16> data) {
	NTIMES({ uint16 x = *data; *data++ = (x<<8) | (x>>8); })
}


/* i'm sorry... see the end of grid.c for an explanation... */
//static
void make_hocus_pocus () {
//	exit(1);
#define FOO(S) \
	((BitPacking*)0)->pack(0,Pt<S>(),Pt<uint8>()); \
	((BitPacking*)0)->unpack(0,Pt<uint8>(),Pt<S>());
EACH_NUMBER_TYPE(FOO)
#undef FOO
}