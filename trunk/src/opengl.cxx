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

#include "gridflow.hxx.fcs"
#include <GL/gl.h>
#include <ctype.h>

/* supported by GEM in openGL dir :
Accum
ActiveTextureARB
AlphaFunc
AreTexturesResident
ArrayElement
Base
Begin
BindProgramARB
BindTexture
Bitmap
BlendEquation
BlendFunc
CallList
ClearAccum
ClearColor
ClearDepth
Clear
ClearIndex
ClearStencil
ClipPlane
Color3[bdfis]v? Color3u[bis]v?
Color4[bdfis]v? Color4u[bis]v?
ColorMask
ColorMaterial
CopyPixels
CopyTexImage[12]D
CopyTexSubImage[12]D
CullFace
DeleteTextures
DepthFunc
DepthMask
DepthRange
DisableClientState
Disable
DrawArrays
DrawBuffer
DrawElements
EdgeFlag
EnableClientState
Enable
End
EndList
EvalCoord[12][df]v?
EvalMesh[12]
EvalPoint[12]
FeedbackBuffer
Finish
Flush
Fog[fi]v?
FrontFace
Frustum
GenLists
GenProgramsARB
GenTextures
GetError
GetFloatv
GetIntegerv
GetMap[dfi]v
GetPointerv
GetString
Hint
Index(dfi)v?
IndexMask
Indexsv?
Indexubv?
InitNames
IsEnabled
IsList
IsTexture
Light[fi]
LightModel[fi]
LineStipple
LineWidth
LoadIdentity
LoadMatrix[df]
LoadName
LoadTransposeMatrix[df]
LogicOp
Map[12][df]
MapGrid[12][df]
Materialfv?
Materiali
MatrixMode
MultiTexCoord2fARB
MultMatrix[df]
MultTransposeMatrix[df]
NewList
Normal3[bdfis]v?
Ortho
PassThrough
PixelStore[fi]
PixelTransfer[fi]
PixelZoom
PointSize
PolygonMode
PolygonOffset
PopAttrib
PopClientAttrib
PopMatrix
PopName
PrioritizeTextures
ProgramEnvParameter4dARB
ProgramEnvParameter4fvARB
ProgramLocalParameter4fvARB
ProgramStringARB
PushAttrib
PushClientAttrib
PushMatrix
PushName
RasterPos2[dfis]v?
RasterPos3[dfis]fv?
RasterPos4[dfis]v?
Rect[dfis]
RenderMode
ReportError
Rotate[df]
Scale[df]
Scissor
SelectBuffer
ShadeModel
StencilFunc
StencilMask
StencilOp
TexCoord[1234][dfis]v?
TexEnv[fi]
TexGen[dfi]
TexGenfv
TexImage2D
TexParameter[fi]
TexSubImage[12]D
Translate[df]
gluLookAt
Uniform1fARB
gluPerspective
UseProgramObjectARB
Vertex[234][dfis]v?
Viewport
GLdefine
*/
typedef std::map<t_symbol *,int> primtype_map_t; primtype_map_t primtype_map;

static t_symbol *gl_gensym (const char *s) {
	char t[64];
	strcpy(t,s+3);
	for (int i=0; t[i]; i++) t[i]=tolower(t[i]);
	return gensym(t);
}

static int to_primtype (const t_atom2 &a) {
	if (a.a_type==A_FLOAT) {
		float f = (float)a;
		if (f<0 || f>9 || f!=int(f)) RAISE("primitive_type must be an integer from 0 to 9");
		return f;
	} else if (a.a_type==A_SYMBOL) {
		primtype_map_t::iterator it = primtype_map.find((t_symbol *)a);
		if (it==primtype_map.end()) RAISE("unknown primitive type");
		return it->second;
	} else RAISE("to primtype: expected float or symbol");
}

\class GFGL : FObject {
	\constructor () {}
	~GFGL() {}
	\decl 0 begin (t_atom2 primtype) {glBegin(to_primtype(primtype));}
	\decl 0 color (...) {
		switch (argc) {
			case 3: glColor3f(argv[0],argv[1],argv[2]); break;
			case 4: glColor4f(argv[0],argv[1],argv[2],argv[3]); break;
			default: RAISE("need 3 or 4 args");
		}
	}
	\decl 0 end () {glEnd();}
	\decl 0 vertex (...) {
		switch (argc) {
			case 2: glVertex2f(argv[0],argv[1]); break;
			case 3: glVertex3f(argv[0],argv[1],argv[2]); break;
			case 4: glVertex4f(argv[0],argv[1],argv[2],argv[3]); break;
			default: RAISE("need 2, 3 or 4 args");
		}
	}
};
\end class {install("gf/gl",1,1);}

void startup_opengl () {
	#define define(I,NAME) primtype_map[gl_gensym(#NAME)]=I;
	define(0,GL_POINTS)
	define(1,GL_LINES)
	define(2,GL_LINE_LOOP)
	define(3,GL_LINE_STRIP)
	define(4,GL_TRIANGLES)
	define(5,GL_TRIANGLE_STRIP)
	define(6,GL_TRIANGLE_FAN)
	define(7,GL_QUADS)
	define(8,GL_QUAD_STRIP)
	define(9,GL_POLYGON)
	#undef define
	\startall
}
