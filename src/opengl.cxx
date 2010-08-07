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
\class GFGL : FObject {
	\constructor () {}
	~GFGL() {}
	\decl 0 begin () {}
	\decl 0 end () {}
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
	\startall
}
