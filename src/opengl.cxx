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

struct EnumType {
	const char *name;
	typedef std::map<t_symbol *,int>  forward_t;  forward_t  forward;
	typedef std::map<int,t_symbol *> backward_t; backward_t backward;
	EnumType (const char *name) {this->name = name;}
	GLenum to_enum (const t_atom &a) {return to_enum(*(const t_atom2 *)&a);}
	GLenum to_enum (const t_atom2 &a) {
		if (a.a_type==A_FLOAT) {
			float f = (float)a;
			backward_t::iterator it = backward.find((int)a);
			if (it==backward.end()) RAISE("%d must be an integer from 0 to 9",(int)a);
			return f;
		} else if (a.a_type==A_SYMBOL) {
			forward_t::iterator it = forward.find((t_symbol *)a);
			if (it==forward.end()) RAISE("unknown %s '%s'",name,((t_symbol *)a)->s_name);
			return it->second;
		} else RAISE("to %s: expected float or symbol",name);
	}
	void add (t_symbol *s, GLenum i) {forward[s]=i; backward[i]=s;}
};
static t_symbol *tolower_gensym (const char *s) {
	char t[64]; strcpy(t,s);
	for (int i=0; t[i]; i++) t[i]=tolower(t[i]);
	return gensym(t);
}
struct EnumType primtype("primitive type");
struct EnumType capability("capability");
struct EnumType client_state_capability("client state capability");
struct EnumType cull_mode("cull mode");
struct EnumType texture_target("texture target");
struct EnumType blend_equation("blend equation");
struct EnumType blend_func("blend func");
struct EnumType copy_pixels_type("copy pixels type");
struct EnumType depth_func("depth func");
struct EnumType front_face_mode("front face mode");
struct EnumType logic_op("logic_op");
struct EnumType stencil_func("stencil_func");
static void init_enums () {
	#define define(NAME) primtype.add(tolower_gensym(#NAME+3),NAME);
	#define THAT primtype
	define(GL_POINTS)
	define(GL_LINES)
	define(GL_LINE_LOOP)
	define(GL_LINE_STRIP)
	define(GL_TRIANGLES)
	define(GL_TRIANGLE_STRIP)
	define(GL_TRIANGLE_FAN)
	define(GL_QUADS)
	define(GL_QUAD_STRIP)
	define(GL_POLYGON)
	#undef THAT
	#define THAT capability
	define(GL_ALPHA_TEST)
	define(GL_AUTO_NORMAL)
	define(GL_BLEND)
	define(GL_CLIP_PLANE0)
	define(GL_CLIP_PLANE1)
	define(GL_CLIP_PLANE2)
	define(GL_CLIP_PLANE3)
	define(GL_CLIP_PLANE4)
	define(GL_CLIP_PLANE5)
	define(GL_COLOR_LOGIC_OP)
	define(GL_COLOR_MATERIAL)
	define(GL_COLOR_SUM)
	define(GL_COLOR_TABLE)
	define(GL_CONVOLUTION_1D)
	define(GL_CONVOLUTION_2D)
	define(GL_CULL_FACE)
	define(GL_DEPTH_TEST)
	define(GL_DITHER)
	define(GL_FOG)
	define(GL_HISTOGRAM)
	define(GL_INDEX_LOGIC_OP)
	define(GL_LIGHT0)
	define(GL_LIGHT1)
	define(GL_LIGHT2)
	define(GL_LIGHT3)
	define(GL_LIGHT4)
	define(GL_LIGHT5)
	define(GL_LIGHT6)
	define(GL_LIGHT7)
	define(GL_LIGHTING)
	define(GL_LINE_SMOOTH)
	define(GL_LINE_STIPPLE)
	define(GL_MAP1_COLOR_4)
	define(GL_MAP1_INDEX)
	define(GL_MAP1_NORMAL)
	define(GL_MAP1_TEXTURE_COORD_1)
	define(GL_MAP1_TEXTURE_COORD_2)
	define(GL_MAP1_TEXTURE_COORD_3)
	define(GL_MAP1_TEXTURE_COORD_4)
	define(GL_MAP1_VERTEX_3)
	define(GL_MAP1_VERTEX_4)
	define(GL_MAP2_COLOR_4)
	define(GL_MAP2_INDEX)
	define(GL_MAP2_NORMAL)
	define(GL_MAP2_TEXTURE_COORD_1)
	define(GL_MAP2_TEXTURE_COORD_2)
	define(GL_MAP2_TEXTURE_COORD_3)
	define(GL_MAP2_TEXTURE_COORD_4)
	define(GL_MAP2_VERTEX_3)
	define(GL_MAP2_VERTEX_4)
	define(GL_MINMAX)
	define(GL_MULTISAMPLE)
	define(GL_NORMALIZE)
	define(GL_POINT_SMOOTH)
	define(GL_POINT_SPRITE)
	define(GL_POLYGON_OFFSET_FILL)
	define(GL_POLYGON_OFFSET_LINE)
	define(GL_POLYGON_OFFSET_POINT)
	define(GL_POLYGON_SMOOTH)
	define(GL_POLYGON_STIPPLE)
	define(GL_POST_COLOR_MATRIX_COLOR_TABLE)
	define(GL_POST_CONVOLUTION_COLOR_TABLE)
	define(GL_RESCALE_NORMAL)
	define(GL_SAMPLE_ALPHA_TO_COVERAGE)
	define(GL_SAMPLE_ALPHA_TO_ONE)
	define(GL_SAMPLE_COVERAGE)
	define(GL_SEPARABLE_2D)
	define(GL_SCISSOR_TEST)
	define(GL_STENCIL_TEST)
	define(GL_TEXTURE_1D)
	define(GL_TEXTURE_2D)
	define(GL_TEXTURE_3D)
	define(GL_TEXTURE_CUBE_MAP)
	define(GL_TEXTURE_GEN_Q)
	define(GL_TEXTURE_GEN_R)
	define(GL_TEXTURE_GEN_S)
	define(GL_TEXTURE_GEN_T)
	define(GL_VERTEX_PROGRAM_POINT_SIZE)
	define(GL_VERTEX_PROGRAM_TWO_SIDE)
	#undef THAT
	#define THAT client_state_capability
	define(GL_COLOR_ARRAY)
	define(GL_EDGE_FLAG_ARRAY)
	define(GL_FOG_COORD_ARRAY)
	define(GL_INDEX_ARRAY)
	define(GL_NORMAL_ARRAY)
	define(GL_SECONDARY_COLOR_ARRAY)
	define(GL_TEXTURE_COORD_ARRAY)
	define(GL_VERTEX_ARRAY)
	#undef THAT
	#define THAT cull_mode
	define(GL_FRONT)
	define(GL_BACK)
	define(GL_FRONT_AND_BACK)
	#undef THAT
	#define THAT texture_target
	define(GL_TEXTURE_1D)
	define(GL_TEXTURE_2D)
	define(GL_TEXTURE_3D)
	define(GL_TEXTURE_CUBE_MAP)
	#undef THAT
	#define THAT blend_equation
	define(GL_FUNC_ADD)
	define(GL_FUNC_SUBTRACT)
	define(GL_FUNC_REVERSE_SUBTRACT)
	define(GL_MIN)
	define(GL_MAX)
	#undef THAT
	#define THAT blend_func
	define(GL_ZERO)
	define(GL_ONE)
	define(GL_SRC_COLOR)
	define(GL_ONE_MINUS_SRC_COLOR)
	define(GL_DST_COLOR)
	define(GL_ONE_MINUS_DST_COLOR)
	define(GL_SRC_ALPHA)
	define(GL_ONE_MINUS_SRC_ALPHA)
	define(GL_DST_ALPHA)
	define(GL_ONE_MINUS_DST_ALPHA)
	define(GL_CONSTANT_COLOR)
	define(GL_ONE_MINUS_CONSTANT_COLOR)
	define(GL_CONSTANT_ALPHA)
	define(GL_ONE_MINUS_CONSTANT_ALPHA)
	define(GL_SRC_ALPHA_SATURATE) // not supposed to be available as dfactor
	#undef THAT
	#define THAT copy_pixels_type
	define(GL_COLOR)
	define(GL_DEPTH)
	define(GL_STENCIL)
	#undef THAT
	#define THAT depth_func
	define(GL_NEVER)
	define(GL_LESS)
	define(GL_EQUAL)
	define(GL_LEQUAL)
	define(GL_GREATER)
	define(GL_NOTEQUAL)
	define(GL_GEQUAL)
	define(GL_ALWAYS)
	#undef THAT
	#define THAT front_face_mode
	define(GL_CW)
	define(GL_CCW)
	#undef THAT
	#define THAT logic_op
	define(GL_CLEAR)
	define(GL_SET)
	define(GL_COPY)
	define(GL_COPY_INVERTED)
	define(GL_NOOP)
	define(GL_INVERT)
	define(GL_AND)
	define(GL_NAND)
	define(GL_OR)
	define(GL_NOR)
	define(GL_XOR)
	define(GL_EQUIV)
	define(GL_AND_REVERSE)
	define(GL_AND_INVERTED)
	define(GL_OR_REVERSE)
	define(GL_OR_INVERTED)
	#undef THAT
	#define THAT stencil_func
	define(GL_NEVER)
	define(GL_LESS)
	define(GL_LEQUAL)
	define(GL_GREATER)
	define(GL_GEQUAL)
	define(GL_EQUAL)
	define(GL_NOTEQUAL)
	define(GL_ALWAYS)
	#undef THAT
}
// comments in the class body list those functions not supported by GF but supported by GEM in openGL dir.
\class GFGL : FObject {
	\constructor () {}
	~GFGL() {}
	\decl 0 accum (float op, float value) {glAccum(op,value);} // op should be enum
	// ActiveTextureARB
	\decl 0 alpha_func (float func, float ref) {glAlphaFunc(func,ref);} // enum
	// AreTexturesResident
	\decl 0 array_element (int i) {glArrayElement(i);}
	// Base
	\decl 0 begin (t_atom2 a) {glBegin(primtype.to_enum(a));}
	// BindProgramARB
	// BindTexture //GLAPI void GLAPIENTRY glBindTexture( GLenum target, GLuint texture );
	\decl 0 bind_texture (t_atom target, uint32 texture) {glBindTexture(texture_target.to_enum(target),texture);}
	// Bitmap //glBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap );
	\decl 0 blend_equation (t_atom mode) {glBlendEquation(blend_equation.to_enum(mode));}
	\decl 0 blend_func (t_atom sfactor, t_atom dfactor) {glBlendFunc(blend_func.to_enum(sfactor),blend_func.to_enum(dfactor));}
	\decl 0 call_list (uint32 list) {glCallList(list);}
	// CallLists // GLAPI void GLAPIENTRY glCallLists( GLsizei n, GLenum type, const GLvoid *lists ); // not in GEM
	\decl 0 clear_accum (float r, float g, float b, float a) {glClearAccum(r,g,b,a);}
	\decl 0 clear_color (float r, float g, float b, float a) {glClearColor(r,g,b,a);} // clamp
	\decl 0 clear_depth (float depth) {glClearDepth(depth);} // clamp
	\decl 0 clear (int mask) {glClear(mask);} // bitfield
	\decl 0 clear_index (float c) {glClearIndex(c);}
	\decl 0 clear_stencil (int s) {glClearStencil(s);}
	// ClipPlane // GLAPI void GLAPIENTRY glClipPlane( GLenum plane, const GLdouble *equation );
	\decl 0 color (...) {switch (argc) {
		case 3: glColor3f(argv[0],argv[1],argv[2]); break;
		case 4: glColor4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 3 or 4 args");
	}}
	\decl 0 color_mask (bool r, bool g, bool b, bool a) {glColorMask(r,g,b,a);}
	// ColorMaterial // GLAPI void GLAPIENTRY glColorMaterial( GLenum face, GLenum mode );
	\decl 0 copy_pixels (int x, int y, int width, int height, t_atom type) {
		glCopyPixels(x,y,width,height,copy_pixels_type.to_enum(type));}
	// CopyTexImage1D
	\decl 0 copy_tex_image_2D (GLenum target, int level, GLenum format, int x, int y, int width, int height, int border) {
		glCopyTexImage2D(target,level,format,x,y,width,height,border);} // enum
	// CopyTexSubImage1D
	\decl 0 copy_tex_sub_image_2D (GLenum target, int level, int xoffset, int yoffset, int x, int y, int width, int height) {
		glCopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height);} // enum
	\decl 0 cull_face (t_atom mode) {glCullFace(cull_mode.to_enum(mode));}
	\decl 0 delete_lists (uint32 list, int range) {glDeleteLists(list,range);}  // not in GEM
	// DeleteTextures // GLAPI void GLAPIENTRY glDeleteTextures( GLsizei n, const GLuint *textures);
	\decl 0 depth_func (t_atom func) {glDepthFunc(depth_func.to_enum(func));}
	\decl 0 depth_mask (bool flag) {glDepthMask(flag);}
	\decl 0 depth_range (float near_val, float far_val) {glDepthRange(near_val,far_val);} // clamp
	\decl 0 disable_client_state (t_atom cap) {glDisable(client_state_capability.to_enum(cap));}
	\decl 0 disable (t_atom cap) {glDisable(capability.to_enum(cap));}
	\decl 0 draw_arrays (GLenum mode, int first, int count) {glDrawArrays(mode,first,count);} // enum
	\decl 0 draw_buffer (GLenum mode) {glDrawBuffer(mode);} // enum
	// DrawElements // GLAPI void GLAPIENTRY glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
	// GLAPI void GLAPIENTRY glDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ); // not in GEM
	// EdgeFlag // GLAPI void GLAPIENTRY glEdgeFlagv( const GLboolean *flag );
	\decl 0 enable_client_state (t_atom cap) {glEnable(client_state_capability.to_enum(cap));}
	\decl 0 enable (t_atom cap) {glEnable(capability.to_enum(cap));}
	\decl 0 end () {glEnd();}
	\decl 0 end_list () {glEndList();}
	\decl 0 eval_coord (...) {switch (argc) {
		case 1: glEvalCoord1f(argv[0]); break;
		case 2: glEvalCoord2f(argv[0],argv[1]); break;
		default: RAISE("need 1 or 2 args");
	}}
	\decl 0 eval_mesh (...) {switch (argc) {
		case 3: glEvalMesh1(argv[0],argv[1],argv[2]); break; // enum...
		case 5: glEvalMesh2(argv[0],argv[1],argv[2],argv[3],argv[4]); break; // enum...
		default: RAISE("need 3 or 5 args");
	}}
	\decl 0 eval_point (...) {switch (argc) {
		case 1: glEvalPoint1(argv[0]); break;
		case 2: glEvalPoint2(argv[0],argv[1]); break;
		default: RAISE("need 1 or 2 args");
	}}
	// FeedbackBuffer // void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);
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
	\decl 0 front_face (t_atom mode) {glFrontFace(front_face_mode.to_enum(mode));}
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
	\decl 0 logic_op (t_atom opcode) {glLogicOp(logic_op.to_enum(opcode));}
	// Map[12][df]
	\decl 0 map_grid (...) {switch (argc) {
		case 3: glMapGrid1f(argv[0],argv[1],argv[2]); break; // enum...
		case 6: glMapGrid2f(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]); break; // enum...
		default: RAISE("need 3 or 6 args");
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
	\decl 0 raster_pos (...) {switch (argc) {
		case 2: glRasterPos2f(argv[0],argv[1]); break;
		case 3: glRasterPos3f(argv[0],argv[1],argv[2]); break;
		case 4: glRasterPos4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 2, 3 or 4 args");
	}}
	\decl 0 rect (float x1, float y1, float x2, float y2) {glRectf(x1,y1,x2,y2);}
	// GLAPI void GLAPIENTRY glReadBuffer( GLenum mode ); // not in GEM
	// RenderMode
	// ReportError
	\decl 0 rotate (float a, float x, float y, float z) {glRotatef(a,x,y,z);}
	\decl 0 scale           (float x, float y, float z) {glScalef(x,y,z);}
	\decl 0 scissor(int x, int y, int width, int height) {glScissor(x,y,width,height);}
	// SelectBuffer // GLAPI void GLAPIENTRY glSelectBuffer( GLsizei size, GLuint *buffer );
	// ShadeModel // GLAPI void GLAPIENTRY glShadeModel( GLenum mode );
	\decl 0 stencil_func (t_atom func, int ref, uint32 mask) {
		glStencilFunc(stencil_func.to_enum(func),ref,mask);}
	\decl 0 stencil_mask (uint32 mask) {glStencilMask(mask);}
	// StencilOp // GLAPI void GLAPIENTRY glStencilOp( GLenum fail, GLenum zfail, GLenum zpass );
	\decl 0 tex_coord (...) {switch (argc) {
		case 1: glTexCoord1f(argv[0]); break;
		case 2: glTexCoord2f(argv[0],argv[1]); break;
		case 3: glTexCoord3f(argv[0],argv[1],argv[2]); break;
		case 4: glTexCoord4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 1, 2, 3 or 4 args");
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
	init_enums();
	\startall
}
