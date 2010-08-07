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

// comments in the class body list those functions not supported by GF but supported by GEM in openGL dir.
\class GFGL : FObject {
	\constructor () {}
	~GFGL() {}
	\decl 0 accum (float op, float value) {glAccum(op,value);} // op should be enum
	// ActiveTextureARB
	// AlphaFunc
	\decl 0 alpha_func (float func, float ref) {glAlphaFunc(func,ref);} // enum
	// AreTexturesResident
	\decl 0 array_element (int i) {glArrayElement(i);}
	// Base
	\decl 0 begin (t_atom2 primtype) {glBegin(to_primtype(primtype));}
	// BindProgramARB
	// BindTexture //GLAPI void GLAPIENTRY glBindTexture( GLenum target, GLuint texture );
	// Bitmap //glBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap );
	// BlendEquation // GLAPI void GLAPIENTRY glBlendEquation( GLenum mode );
	// BlendFunc // GLAPI void GLAPIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor );
	// CallList // GLAPI void GLAPIENTRY glCallList( GLuint list );
	// CallLists // GLAPI void GLAPIENTRY glCallLists( GLsizei n, GLenum type, const GLvoid *lists ); // not in GEM
	\decl 0 clear_accum (float r, float g, float b, float a) {glClearAccum(r,g,b,a);}
	\decl 0 clear_color (float r, float g, float b, float a) {glClearColor(r,g,b,a);} // clamp
	// ClearDepth // GLAPI void GLAPIENTRY glClearDepth( GLclampd depth );
	// Clear // GLAPI void GLAPIENTRY glClear( GLbitfield mask );
	// ClearIndex // GLAPI void GLAPIENTRY glClearIndex( GLfloat c );
	// ClearStencil // GLAPI void GLAPIENTRY glClearStencil( GLint s );
	// ClipPlane // GLAPI void GLAPIENTRY glClipPlane( GLenum plane, const GLdouble *equation );
	\decl 0 color (...) {switch (argc) {
		case 3: glColor3f(argv[0],argv[1],argv[2]); break;
		case 4: glColor4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 3 or 4 args");
	}}
	\decl 0 color_mask (bool r, bool g, bool b, bool a) {glColorMask(r,g,b,a);}
	// ColorMaterial // GLAPI void GLAPIENTRY glColorMaterial( GLenum face, GLenum mode );
	// CopyPixels // GLAPI void GLAPIENTRY glCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type );
	// CopyTexImage[12]D
	// CopyTexSubImage[12]D
	// CullFace // GLAPI void GLAPIENTRY glCullFace( GLenum mode );
	// GLAPI void GLAPIENTRY glDeleteLists( GLuint list, GLsizei range ); // not in GEM
	// DeleteTextures // GLAPI void GLAPIENTRY glDeleteTextures( GLsizei n, const GLuint *textures);
	// DepthFunc // GLAPI void GLAPIENTRY glDepthFunc( GLenum func );
	// DepthMask // GLAPI void GLAPIENTRY glDepthMask( GLboolean flag );
	// DepthRange // GLAPI void GLAPIENTRY glDepthRange( GLclampd near_val, GLclampd far_val );
	// DisableClientState // GLAPI void GLAPIENTRY glDisableClientState( GLenum cap );  /* 1.1 */
	// Disable // GLAPI void GLAPIENTRY glDisable( GLenum cap );
	// DrawArrays // GLAPI void GLAPIENTRY glDrawArrays( GLenum mode, GLint first, GLsizei count );
	// DrawBuffer // GLAPI void GLAPIENTRY glDrawBuffer( GLenum mode );
	// DrawElements // GLAPI void GLAPIENTRY glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
	// GLAPI void GLAPIENTRY glDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ); // not in GEM
	// EdgeFlag // GLAPI void GLAPIENTRY glEdgeFlagv( const GLboolean *flag );
	// EnableClientState // GLAPI void GLAPIENTRY glEnableClientState( GLenum cap );  /* 1.1 */
	// Enable // GLAPI void GLAPIENTRY glEnable( GLenum cap );
	\decl 0 end () {glEnd();}
	\decl 0 end_list () {glEndList();}
	\decl 0 eval_coord (...) {switch (argc) {
		case 1: glEvalCoord1f(argv[0]); break;
		case 2: glEvalCoord2f(argv[0],argv[1]); break;
		default: RAISE("need 2, 3 or 4 args");
	}}
	\decl 0 eval_mesh (...) {switch (argc) {
		case 3: glEvalMesh1(argv[0],argv[1],argv[2]); break; // enum...
		case 5: glEvalMesh2(argv[0],argv[1],argv[2],argv[3],argv[4]); break; // enum...
		default: RAISE("need 2, 3 or 4 args");
	}}
	// EvalPoint[12]
	\decl 0 eval_point (...) {switch (argc) {
		case 1: glEvalPoint1(argv[0]); break;
		case 2: glEvalPoint2(argv[0],argv[1]); break;
	}}
	// FeedbackBuffer
	\decl 0 finish () {glFinish();}
	\decl 0 flush  () {glFlush();}
	\decl 0 fog (...) { // enum
		if (argc<2) RAISE("at least 2 args");
		int pname = argv[0];
		if (pname==GL_FOG_COLOR) {
			if (argc!=5) RAISE("fog color: need 4 floats after this $1");
			float fv[4]; fv[0]=argv[1]; fv[1]=argv[2]; fv[2]=argv[3]; fv[3]=argv[4];
			glFogfv(pname,fv);
		} else if (pname==GL_FOG_MODE || pname==GL_FOG_DENSITY || pname==GL_FOG_START || pname==GL_FOG_END ||
		pname==GL_FOG_INDEX || pname==GL_FOG_COORD_SRC) {
			if (argc!=2) RAISE("fog 0x%x: need 1 float after this $1",pname);
			glFogf(pname,argv[1]);
		} else RAISE("unknown fog command");
	}
	// FrontFace // GLAPI void GLAPIENTRY glFrontFace( GLenum mode );
	\decl 0 frustum(float left, float right, float bottom, float top, float near_val, float far_val) {
		glFrustum(left,right,bottom,top,near_val,far_val);
	}
	// GenLists
	// GenProgramsARB
	// GenTextures
	// GetError
	// GetFloatv
	// GetIntegerv
	// GLAPI void GLAPIENTRY glGetLightfv( GLenum light, GLenum pname, GLfloat *params ); // not in GEM
	// GetMap[dfi]v
	// GetPointerv
	// GetString
	\decl 0 hint (float target, float mode) {glHint(target,mode);} // enum
	// Index(dfi)v?
	// IndexMask
	// Indexsv?
	// Indexubv?
	\decl 0 init_names () {glInitNames();}
	// IsEnabled
	// IsList
	// IsTexture
	// Light[fi] // GLAPI void GLAPIENTRY glLightfv( GLenum light, GLenum pname, const GLfloat *params );
	// LightModel[fi] // GLAPI void GLAPIENTRY glLightModelfv( GLenum pname, const GLfloat *params );
	\decl 0 line_stipple (int factor, uint16 pattern) {glLineStipple(factor,pattern);}
	\decl 0 line_width (float width) {glLineWidth(width);}
	// ListBase // GLAPI void GLAPIENTRY glListBase( GLuint base ); // not in GEM
	\decl 0 load_identity () {glLoadIdentity();}
	\decl 0 load_matrix (...) {
		if (argc!=16) RAISE("need 16 args");
		float fv[16]; for (int i=0; i<16; i++) fv[i]=argv[i];
		glLoadMatrixf(fv);
	}
	\decl 0 load_name (uint32 name) {glLoadName(name);}
	\decl 0 load_transpose_matrix (...) {
		if (argc!=16) RAISE("need 16 args");
		float fv[16]; for (int i=0; i<16; i++) fv[i]=argv[i];
		glLoadTransposeMatrixf(fv);
	}
	// LogicOp // GLAPI void GLAPIENTRY glLogicOp( GLenum opcode );
	// Map[12][df]
	\decl 0 map_grid (...) {switch (argc) {
		case 3: glMapGrid1f(argv[0],argv[1],argv[2]); break; // enum...
		case 6: glMapGrid2f(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]); break; // enum...
		default: RAISE("need 2, 3 or 4 args");
	}}
	// Materialfv? // GLAPI void GLAPIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params );
	// MatrixMode // GLAPI void GLAPIENTRY glMatrixMode( GLenum mode );
	// MultiTexCoord2fARB
	// MultMatrix[df] // GLAPI void GLAPIENTRY glMultMatrixf( const GLfloat *m );
	// MultTransposeMatrix[df]
	// NewList // GLAPI void GLAPIENTRY glNewList( GLuint list, GLenum mode );
	// Normal3[bdfis]v?
	\decl 0 normal (float x, float y, float z) {glNormal3f(x,y,z);}
	\decl 0 ortho(float left, float right, float bottom, float top, float near_val, float far_val) {
		glOrtho(left,right,bottom,top,near_val,far_val);
	}
	\decl 0 pass_through (float token) {glPassThrough(token);}
	// PixelStore[fi]
	// PixelTransfer[fi]
	// PixelZoom
	\decl 0 point_size (float size) {glPointSize(size);}
	\decl 0 polygon_mode (float face, float mode) {glPolygonMode(face,mode);} // enum
	\decl 0 polygon_offset (float factor, float units) {glPolygonOffset(factor,units);}
	\decl 0 pop_attrib () {glPopAttrib();}
	\decl 0 pop_client_attrib () {glPopClientAttrib();}
	\decl 0 pop_matrix ()  {glPopMatrix();}
	\decl 0 pop_name () {glPopName();}
	// PrioritizeTextures
	// ProgramEnvParameter4dARB
	// ProgramEnvParameter4fvARB
	// ProgramLocalParameter4fvARB
	// ProgramStringARB
	// PushAttrib // GLAPI void GLAPIENTRY glPushAttrib( GLbitfield mask );
	// PushClientAttrib // GLAPI void GLAPIENTRY glPushClientAttrib( GLbitfield mask );  /* 1.1 */
	\decl 0 push_matrix () {glPushMatrix();}
	\decl 0 push_name (uint32 name) {glPushName(name);}
	// RasterPos2[dfis]v?
	// RasterPos3[dfis]fv?
	// RasterPos4[dfis]v?
	\decl 0 rect (float x1, float y1, float x2, float y2) {glRectf(x1,y1,x2,y2);}
	// GLAPI void GLAPIENTRY glReadBuffer( GLenum mode ); // not in GEM
	// RenderMode
	// ReportError
	\decl 0 rotate (float a, float x, float y, float z) {glRotatef(a,x,y,z);}
	\decl 0 scale           (float x, float y, float z) {glScalef(x,y,z);}
	\decl 0 scissor(int x, int y, int width, int height) {glScissor(x,y,width,height);}
	// SelectBuffer // GLAPI void GLAPIENTRY glSelectBuffer( GLsizei size, GLuint *buffer );
	// ShadeModel // GLAPI void GLAPIENTRY glShadeModel( GLenum mode );
	// StencilFunc // GLAPI void GLAPIENTRY glStencilFunc( GLenum func, GLint ref, GLuint mask );
	// StencilMask // GLAPI void GLAPIENTRY glStencilMask( GLuint mask );
	// StencilOp // GLAPI void GLAPIENTRY glStencilOp( GLenum fail, GLenum zfail, GLenum zpass );
	\decl 0 tex_coord (...) {switch (argc) {
		case 1: glTexCoord1f(argv[0]); break;
		case 2: glTexCoord2f(argv[0],argv[1]); break;
		case 3: glTexCoord3f(argv[0],argv[1],argv[2]); break;
		case 4: glTexCoord4f(argv[0],argv[1],argv[2],argv[3]); break;
	}}
	// TexEnv[fi] // GLAPI void GLAPIENTRY glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params );
	// TexGenfv // GLAPI void GLAPIENTRY glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params );
	// TexImage2D // GLAPI void GLAPIENTRY glTexImage2D( GLenum target, GLint level, GLint internalFormat,
		// GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
	// TexParameter[fi] // GLAPI void GLAPIENTRY glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params );
	// TexSubImage[12]D
	\decl 0 translate       (float x, float y, float z) {glTranslatef(x,y,z);}
	// Uniform1fARB
	\decl 0 vertex (...) {switch (argc) {
		case 2: glVertex2f(argv[0],argv[1]); break;
		case 3: glVertex3f(argv[0],argv[1],argv[2]); break;
		case 4: glVertex4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 2, 3 or 4 args");
	}}
	\decl 0 viewport (int x, int y, int width, int height) {glViewport(x,y,width,height);}
	// gluLookAt
	// gluPerspective
	// UseProgramObjectARB
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
