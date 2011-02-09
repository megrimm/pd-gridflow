/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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

#include "bundled/m_pd.h"
extern "C" void sys_load_lib(t_canvas *,const char *);

//struct GemVersion {static const char *versionString();};
struct GemState    {GemState(); char trabant[666];};
struct imageStruct {imageStruct(); char lada[666];};
//#define sys_load_lib(A,B) do {post("pre sys_load_lib(%s)",B); sys_load_lib(A,B); post("post sys_load_lib(%s)",B);} while(0)

extern "C" void gridflow_gem_loader_setup () {
	//post("GF sizeof(imageStruct)=%d sizeof(pixBlock)=%d sizeof(GemState)=%d",sizeof(imageStruct),sizeof(pixBlock),sizeof(GemState));
	//int major,minor; sscanf(GemVersion::versionString(),"%d.%d",&major,&minor); gem = major*1000+minor;
	int GemState_version = -1;
 	GemState *dummy = new GemState();
	float *stupide = (float *)dummy;
	int i;
	for (i=0; i<16; i++) if (stupide[i]==50.f) break;
	if (i==16) {error("GridFlow: can't find GemState::tickTime"); return;}
	int j = i-2-2*sizeof(void*)/sizeof(float);
	//post("GemState::tickTime found at [%d], so pixBlock is probably at [%d]",i,j);	
	if      (j==3        ) {GemState_version = 93;}
	else if (j==5 || j==6) {GemState_version = 92;}
	else {error("GridFlow: can't detect this version of GEM: i=%d j=%d",i,j); return;}
	//delete dummy;
	/* note that j==6 is because in 64-bit mode you have one int of padding in GemState92 just before the pixBlock* */
	// imageStruct 92 starts with int xsize=ysize=0; imageStruct 93 starts with a C++ class pointer != 0 */
	int imageStruct_version = *(long *)new imageStruct() ? 93 : 92;
	post("GridFlow/GEM bridge : GemState version %d, imageStruct version %d",GemState_version,imageStruct_version);
	if (GemState_version==92)          sys_load_lib(0,"gridflow/gridflow_gem9292");
	else if (imageStruct_version==92)  sys_load_lib(0,"gridflow/gridflow_gem9293");
	else                               sys_load_lib(0,"gridflow/gridflow_gem9393");
}
