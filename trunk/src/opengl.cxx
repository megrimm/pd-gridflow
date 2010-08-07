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
	GLenum operator () (const t_atom &a) {return (*this)(*(const t_atom2 *)&a);}
	GLenum operator () (const t_atom2 &a) {
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
#define MAKETYPE(NAME) struct EnumType NAME(#NAME);
MAKETYPE(primitive_type)
MAKETYPE(capability)
MAKETYPE(client_state_capability)
MAKETYPE(which_side)
MAKETYPE(texture_target)
MAKETYPE(blend_equation)
MAKETYPE(blend_func)
MAKETYPE(copy_pixels_type)
MAKETYPE(depth_func)
MAKETYPE(front_face_mode)
MAKETYPE(logic_op)
MAKETYPE(stencil_func)
MAKETYPE(pixel_store)
MAKETYPE(pixel_transfer)
MAKETYPE(shade_model)
MAKETYPE(stencil_op)
MAKETYPE(render_mode)
MAKETYPE(buffer_mode)
MAKETYPE(material_mode)
MAKETYPE(copy_tex_target)
MAKETYPE(copy_tex_format)
static void init_enums () {
	#define define(NAME) THAT.add(tolower_gensym(#NAME+3),NAME);
	#define THAT primitive_type
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
	#define THAT which_side
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
	#define THAT pixel_store
	define(GL_PACK_SWAP_BYTES)
	define(GL_PACK_LSB_FIRST)
	define(GL_PACK_ROW_LENGTH)
	define(GL_PACK_IMAGE_HEIGHT)
	define(GL_PACK_SKIP_PIXELS)
	define(GL_PACK_SKIP_ROWS)
	define(GL_PACK_SKIP_IMAGES)
	define(GL_PACK_ALIGNMENT)
	define(GL_UNPACK_SWAP_BYTES)
	define(GL_UNPACK_LSB_FIRST)
	define(GL_UNPACK_ROW_LENGTH)
	define(GL_UNPACK_IMAGE_HEIGHT)
	define(GL_UNPACK_SKIP_PIXELS)
	define(GL_UNPACK_SKIP_ROWS)
	define(GL_UNPACK_SKIP_IMAGES)
	define(GL_UNPACK_ALIGNMENT)
	#undef THAT
	#define THAT pixel_transfer
	define(GL_MAP_COLOR)
	define(GL_MAP_STENCIL)
	define(GL_INDEX_SHIFT)
	define(GL_INDEX_OFFSET)
	define(GL_RED_SCALE)
	define(GL_RED_BIAS)
	define(GL_GREEN_SCALE)
	define(GL_GREEN_BIAS)
	define(GL_BLUE_SCALE)
	define(GL_BLUE_BIAS)
	define(GL_ALPHA_SCALE)
	define(GL_ALPHA_BIAS)
	define(GL_DEPTH_SCALE)
	define(GL_DEPTH_BIAS)
	#ifdef ARB
	define(GL_POST_COLOR_MATRIX_RED_SCALE)
	define(GL_POST_COLOR_MATRIX_GREEN_SCALE)
	define(GL_POST_COLOR_MATRIX_BLUE_SCALE)
	define(GL_POST_COLOR_MATRIX_ALPHA_SCALE)
	define(GL_POST_COLOR_MATRIX_RED_BIAS)
	define(GL_POST_COLOR_MATRIX_GREEN_BIAS)
	define(GL_POST_COLOR_MATRIX_BLUE_BIAS)
	define(GL_POST_COLOR_MATRIX_ALPHA_BIAS)
	define(GL_POST_CONVOLUTION_RED_SCALE)
	define(GL_POST_CONVOLUTION_GREEN_SCALE)
	define(GL_POST_CONVOLUTION_BLUE_SCALE)
	define(GL_POST_CONVOLUTION_ALPHA_SCALE)
	define(GL_POST_CONVOLUTION_RED_BIAS)
	define(GL_POST_CONVOLUTION_GREEN_BIAS)
	define(GL_POST_CONVOLUTION_BLUE_BIAS)
	define(GL_POST_CONVOLUTION_ALPHA_BIAS)
	#endif
	#undef THAT
	#define THAT shade_model
	define(GL_FLAT)
	define(GL_SMOOTH)
	#undef THAT
	#define THAT stencil_op
	define(GL_KEEP)
	define(GL_ZERO)
	define(GL_REPLACE)
	define(GL_INCR)
	define(GL_INCR_WRAP)
	define(GL_DECR)
	define(GL_DECR_WRAP)
	define(GL_INVERT)
	#undef THAT
	#define THAT render_mode
	define(GL_RENDER)
	define(GL_SELECT)
	define(GL_FEEDBACK)
	#undef THAT
	#define THAT buffer_mode
	define(GL_NONE) // not always acceptable
	define(GL_FRONT_AND_BACK) // not always acceptable
	define(GL_FRONT_LEFT)
	define(GL_FRONT_RIGHT)
	define(GL_BACK_LEFT)
	define(GL_BACK_RIGHT)
	define(GL_FRONT)
	define(GL_BACK)
	define(GL_LEFT)
	define(GL_RIGHT)
	define(GL_AUX0)
	define(GL_AUX1)
	define(GL_AUX2)
	define(GL_AUX3)
	#undef THAT
	#define THAT material_mode
	define(GL_EMISSION)
	define(GL_AMBIENT)
	define(GL_DIFFUSE)
	define(GL_SPECULAR)
	define(GL_AMBIENT_AND_DIFFUSE)
	#undef THAT
	#define THAT copy_tex_target
	define(GL_TEXTURE_2D)
	define(GL_TEXTURE_CUBE_MAP_POSITIVE_X)
	define(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
	define(GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
	define(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
	define(GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
	define(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	#undef THAT
	#define THAT copy_tex_format
	define(GL_ALPHA)
	define(GL_ALPHA4)
	define(GL_ALPHA8)
	define(GL_ALPHA12)
	define(GL_ALPHA16)
	define(GL_COMPRESSED_ALPHA)
	define(GL_COMPRESSED_LUMINANCE)
	define(GL_COMPRESSED_LUMINANCE_ALPHA)
	define(GL_COMPRESSED_INTENSITY)
	define(GL_COMPRESSED_RGB)
	define(GL_COMPRESSED_RGBA)
	define(GL_DEPTH_COMPONENT)
	define(GL_DEPTH_COMPONENT16)
	define(GL_DEPTH_COMPONENT24)
	define(GL_DEPTH_COMPONENT32)
	define(GL_LUMINANCE)
	define(GL_LUMINANCE4)
	define(GL_LUMINANCE8)
	define(GL_LUMINANCE12)
	define(GL_LUMINANCE16)
	define(GL_LUMINANCE_ALPHA)
	define(GL_LUMINANCE4_ALPHA4)
	define(GL_LUMINANCE6_ALPHA2)
	define(GL_LUMINANCE8_ALPHA8)
	define(GL_LUMINANCE12_ALPHA4)
	define(GL_LUMINANCE12_ALPHA12)
	define(GL_LUMINANCE16_ALPHA16)
	define(GL_INTENSITY)
	define(GL_INTENSITY4)
	define(GL_INTENSITY8)
	define(GL_INTENSITY12)
	define(GL_INTENSITY16)
	define(GL_RGB)
	define(GL_R3_G3_B2)
	define(GL_RGB4)
	define(GL_RGB5)
	define(GL_RGB8)
	define(GL_RGB10)
	define(GL_RGB12)
	define(GL_RGB16)
	define(GL_RGBA)
	define(GL_RGBA2)
	define(GL_RGBA4)
	define(GL_RGB5_A1)
	define(GL_RGBA8)
	define(GL_RGB10_A2)
	define(GL_RGBA12)
	define(GL_RGBA16)
	define(GL_SLUMINANCE)
	define(GL_SLUMINANCE8)
	define(GL_SLUMINANCE_ALPHA)
	define(GL_SLUMINANCE8_ALPHA8)
	define(GL_SRGB)
	define(GL_SRGB8)
	define(GL_SRGB_ALPHA)
	define(GL_SRGB8_ALPHA8)
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
	\decl 0 begin (t_atom2 a) {glBegin(primitive_type(a));}
	// BindProgramARB
	// BindTexture //GLAPI void GLAPIENTRY glBindTexture(t_atom target, GLuint texture);
	\decl 0 bind_texture (t_atom target, uint32 texture) {glBindTexture(texture_target(target),texture);}
	// Bitmap //glBitmap(int width, int height, float xorig, float yorig, float xmove, float ymove, const GLubyte *bitmap );
	\decl 0 blend_equation (t_atom mode) {glBlendEquation(blend_equation(mode));}
	\decl 0 blend_func (t_atom sfactor, t_atom dfactor) {glBlendFunc(blend_func(sfactor),blend_func(dfactor));}
	\decl 0 call_list (uint32 list) {glCallList(list);}
	// CallLists // GLAPI void GLAPIENTRY glCallLists(int n, t_atom type, const GLvoid *lists ); // not in GEM
	\decl 0 clear_accum (float r, float g, float b, float a) {glClearAccum(r,g,b,a);}
	\decl 0 clear_color (float r, float g, float b, float a) {glClearColor(r,g,b,a);} // clamp
	\decl 0 clear_depth (float depth) {glClearDepth(depth);} // clamp
	\decl 0 clear (int mask) {glClear(mask);} // bitfield
	\decl 0 clear_index (float c) {glClearIndex(c);}
	\decl 0 clear_stencil (int s) {glClearStencil(s);}
	// ClipPlane // GLAPI void GLAPIENTRY glClipPlane(t_atom plane, const GLdouble *equation);
	\decl 0 color (...) {switch (argc) {
		case 3: glColor3f(argv[0],argv[1],argv[2]); break;
		case 4: glColor4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 3 or 4 args");
	}}
	\decl 0 color_mask (bool r, bool g, bool b, bool a) {glColorMask(r,g,b,a);}
	\decl 0 color_material (t_atom face, t_atom mode) {glColorMaterial(which_side(face),material_mode(mode));}
	\decl 0 copy_pixels (int x, int y, int width, int height, t_atom type) {
		glCopyPixels(x,y,width,height,copy_pixels_type(type));}
	// CopyTexImage1D
	\decl 0 copy_tex_image_2D (t_atom target, int level, t_atom format, int x, int y, int width, int height, int border) {
		glCopyTexImage2D(copy_tex_target(target),level,copy_tex_format(format),x,y,width,height,border);}
	// CopyTexSubImage1D
	\decl 0 copy_tex_sub_image_2D (t_atom target, int level, int xoffset, int yoffset, int x, int y, int width, int height) {
		glCopyTexSubImage2D(copy_tex_target(target),level,xoffset,yoffset,x,y,width,height);} // enum
	\decl 0 cull_face (t_atom mode) {glCullFace(which_side(mode));}
	\decl 0 delete_lists (uint32 list, int range) {glDeleteLists(list,range);}  // not in GEM
	// DeleteTextures // GLAPI void GLAPIENTRY glDeleteTextures(int n, const GLuint *textures);
	\decl 0 depth_func (t_atom func) {glDepthFunc(depth_func(func));}
	\decl 0 depth_mask (bool flag) {glDepthMask(flag);}
	\decl 0 depth_range (float near_val, float far_val) {glDepthRange(near_val,far_val);} // clamp
	\decl 0 disable_client_state (t_atom cap) {glDisable(client_state_capability(cap));}
	\decl 0 disable (t_atom cap) {glDisable(capability(cap));}
	\decl 0 draw_arrays (t_atom mode, int first, int count) {glDrawArrays(primitive_type(mode),first,count);}
	\decl 0 draw_buffer (t_atom mode) {glDrawBuffer(buffer_mode(mode));}
	// DrawElements // GLAPI void GLAPIENTRY glDrawElements(t_atom mode, int count, t_atom type, const GLvoid *indices);
	// GLAPI void GLAPIENTRY glDrawPixels(int width, int height, t_atom format, t_atom type, const GLvoid *pixels); // not in GEM
	// EdgeFlag // GLAPI void GLAPIENTRY glEdgeFlagv(const GLboolean *flag);
	\decl 0 enable_client_state (t_atom cap) {glEnable(client_state_capability(cap));}
	\decl 0 enable (t_atom cap) {glEnable(capability(cap));}
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
	// FeedbackBuffer // void glFeedbackBuffer(int size, t_atom type, float *buffer);
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
	\decl 0 front_face (t_atom mode) {glFrontFace(front_face_mode(mode));}
	\decl 0 frustum(float left, float right, float bottom, float top, float near_val, float far_val) {
		glFrustum(left,right,bottom,top,near_val,far_val);
	}
	// GenLists
	// GenProgramsARB
	// GenTextures
	// GetError
	// GetFloatv
	// GetIntegerv
	// GLAPI void GLAPIENTRY glGetLightfv(t_atom light, t_atom pname, float *params); // not in GEM
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
	// Light[fi] // GLAPI void GLAPIENTRY glLightfv(t_atom light, t_atom pname, const float *params);
	// LightModel[fi] // GLAPI void GLAPIENTRY glLightModelfv(t_atom pname, const float *params);
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
	\decl 0 logic_op (t_atom opcode) {glLogicOp(logic_op(opcode));}
	// glMap1f(t_atom target, float u1, float u2, int stride, int order, const float *points );
	// glMap2f(t_atom target, float u1, float u2, int ustride, int uorder, float v1, float v2, int vstride, int vorder, const float *points );
	\decl 0 map_grid (...) {switch (argc) {
		case 3: glMapGrid1f(argv[0],argv[1],argv[2]); break; // enum...
		case 6: glMapGrid2f(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]); break; // enum...
		default: RAISE("need 3 or 6 args");
	}}
	// Materialfv? // GLAPI void GLAPIENTRY glMaterialfv(t_atom face, t_atom pname, const float *params);
	// MatrixMode // GLAPI void GLAPIENTRY glMatrixMode(t_atom mode);
	// MultiTexCoord2fARB
	// MultMatrix[df] // GLAPI void GLAPIENTRY glMultMatrixf( const float *m );
	\decl 0 mult_matrix (...) {
		if (argc!=16) RAISE("need 16 args");
		float fv[16]; for (int i=0; i<16; i++) fv[i]=argv[i];
		glMultMatrixf(fv);
	}
	\decl 0 mult_transpose_matrix (...) {
		if (argc!=16) RAISE("need 16 args");
		float fv[16]; for (int i=0; i<16; i++) fv[i]=argv[i];
		glMultTransposeMatrixf(fv);
	}
	// NewList // GLAPI void GLAPIENTRY glNewList(uint32 list, t_atom mode);
	\decl 0 normal (float x, float y, float z) {glNormal3f(x,y,z);}
	\decl 0 ortho(float left, float right, float bottom, float top, float near_val, float far_val) {
		glOrtho(left,right,bottom,top,near_val,far_val);
	}
	\decl 0 pass_through (float token) {glPassThrough(token);}
	\decl 0 pixel_store    (t_atom pname, float param) {glPixelStoref(   pixel_store   (pname),param);}
	\decl 0 pixel_transfer (t_atom pname, float param) {glPixelTransferf(pixel_transfer(pname),param);}
	\decl 0 pixel_zoom (float xfactor, float yfactor) {glPixelZoom(xfactor,yfactor);}
	\decl 0 point_size (float size) {glPointSize(size);}
	\decl 0 polygon_mode (float face, float mode) {glPolygonMode(face,mode);} // enum
	\decl 0 polygon_offset (float factor, float units) {glPolygonOffset(factor,units);}
	\decl 0 pop_attrib () {glPopAttrib();}
	\decl 0 pop_client_attrib () {glPopClientAttrib();}
	\decl 0 pop_matrix ()  {glPopMatrix();}
	\decl 0 pop_name () {glPopName();}
	// PrioritizeTextures // GLAPI void GLAPIENTRY glPrioritizeTextures(int n, const GLuint *textures, const GLclampf *priorities );
	// ProgramEnvParameter4dARB
	// ProgramEnvParameter4fvARB
	// ProgramLocalParameter4fvARB
	// ProgramStringARB
	\decl 0 push_attrib (int mask) {glPushAttrib(mask);} // bitfield
	\decl 0 push_client_attrib (int mask) {glPushClientAttrib(mask);} // bitfield
	\decl 0 push_matrix () {glPushMatrix();}
	\decl 0 push_name (uint32 name) {glPushName(name);}
	\decl 0 raster_pos (...) {switch (argc) {
		case 2: glRasterPos2f(argv[0],argv[1]); break;
		case 3: glRasterPos3f(argv[0],argv[1],argv[2]); break;
		case 4: glRasterPos4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 2, 3 or 4 args");
	}}
	\decl 0 rect (float x1, float y1, float x2, float y2) {glRectf(x1,y1,x2,y2);}
	\decl 0 read_buffer (t_atom mode) {glReadBuffer(buffer_mode(mode));} // not in GEM
	\decl 0 render_mode (t_atom mode) {glRenderMode(render_mode(mode));}
	// ReportError ???
	\decl 0 rotate (float a, float x, float y, float z) {glRotatef(a,x,y,z);}
	\decl 0 scale           (float x, float y, float z) {glScalef(x,y,z);}
	\decl 0 scissor(int x, int y, int width, int height) {glScissor(x,y,width,height);}
	// SelectBuffer // GLAPI void GLAPIENTRY glSelectBuffer(int size, GLuint *buffer );
	\decl 0 shade_model (t_atom mode) {shade_model(mode);}
	\decl 0 stencil_func (t_atom func, int ref, uint32 mask) {
		glStencilFunc(stencil_func(func),ref,mask);}
	\decl 0 stencil_mask (uint32 mask) {glStencilMask(mask);}
	\decl 0 stencil_op (t_atom fail, t_atom zfail, t_atom zpass) {
		glStencilOp(stencil_op(fail),stencil_op(zfail),stencil_op(zpass));}
	\decl 0 tex_coord (...) {switch (argc) {
		case 1: glTexCoord1f(argv[0]); break;
		case 2: glTexCoord2f(argv[0],argv[1]); break;
		case 3: glTexCoord3f(argv[0],argv[1],argv[2]); break;
		case 4: glTexCoord4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 1, 2, 3 or 4 args");
	}}
	// TexEnv[fi] // GLAPI void GLAPIENTRY glTexEnvfv(t_atom target, t_atom pname, const float *params);
	// TexGenfv // GLAPI void GLAPIENTRY glTexGenfv(t_atom coord, t_atom pname, const float *params);
	// TexImage2D // GLAPI void GLAPIENTRY glTexImage2D(t_atom target, int level, int internalFormat,
		// int width, int height, int border, t_atom format, t_atom type, const GLvoid *pixels);
	// TexParameter[fi] // GLAPI void GLAPIENTRY glTexParameterfv(t_atom target, t_atom pname, const float *params);
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
