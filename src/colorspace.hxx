/*
	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

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

#ifndef __COLORSPACE_H
#define __COLORSPACE_H

extern int cliptab[1024]; // lut for clipping because YUV2R,G,B formula outputs can go from -317 to 573.
inline uint8 fastclip(int x) {return cliptab[x+384];} // be careful with this !
// these convert from reduced-range YUV to full-range RGB
#define YUV2R(Y,U,V) fastclip( (298*(Y-16)               + 409*(V-128) +128)>>8)
#define YUV2G(Y,U,V) fastclip( (298*(Y-16) - 100*(U-128) - 208*(V-128) +128)>>8)
#define YUV2B(Y,U,V) fastclip( (298*(Y-16) + 516*(U-128)               +128)>>8)
#define YUV2RGB(b,Y,U,V) (b)[0]=YUV2R(Y,U,V); (b)[1]=YUV2G(Y,U,V); (b)[2]=YUV2B(Y,U,V);
// these convert from reduced-range YUV to full-range YUV
#define YUV2Y(Y,U,V) fastclip( (298*(Y-16)                             +128)>>8)
#define YUV2U(Y,U,V) fastclip(((             293*(U-128)               +128)>>8)+128)
#define YUV2V(Y,U,V) fastclip(((                           293*(V-128) +128)>>8)+128)
#define YUV2YUV(b,Y,U,V) (b)[0]=YUV2Y(Y,U,V); (b)[1]=YUV2U(Y,U,V); (b)[2]=YUV2V(Y,U,V);
// these are for reading different memory layouts of YUV
#define GET420P(x) do {Y1=bufy[(x)+0]; U=bufu[(x)/2]; Y2=bufy[(x)+1]; V=bufv[(x)/2];} while (0)
#define GETYUYV(x) do {Y1=bufy[(x)+0]; U=bufy[(x)+1]; Y2=bufy[(x)+2]; V=bufy[(x)+3];} while (0)

// these convert from full-range RGB to full-range YUV
// macros for reduced-range YUV would be needed in order to really support the 'magic' colorspace everywhere.
#define RGB2Y(R,G,B)  fastclip(    ((  76*R + 150*G +  29*B +128)>>8))
#define RGB2U(R,G,B)  fastclip(128+((- 44*R -  85*G + 108*B +128)>>8))
#define RGB2V(R,G,B)  fastclip(128+(( 128*R - 108*G -  21*B +128)>>8))

// these convert from full-range RGB to reduced-range YUV
#define RGB2Y_(R,G,B) fastclip(    ((  66*R + 129*G +  25*B +128)>>8))
#define RGB2U_(R,G,B) fastclip(128+((- 38*R -  74*G + 112*B +128)>>8))
#define RGB2V_(R,G,B) fastclip(128+(( 112*R -  94*G -  18*B +128)>>8))

#endif // __COLORSPACE_H
