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
	EnumType &add (t_symbol *s, GLenum i) {forward[s]=i; backward[i]=s; return *this;}
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
MAKETYPE(tex_target)
MAKETYPE(copy_tex_target)
MAKETYPE(tex_iformat)
MAKETYPE(tex_format)
MAKETYPE(tex_type)
MAKETYPE(polygon_mode)
MAKETYPE(hint_target)
MAKETYPE(hint_mode)
MAKETYPE(accum_op)
MAKETYPE(fog_param)
MAKETYPE(list_mode)
MAKETYPE(texture_parameter)
MAKETYPE(texture_min_filter)
MAKETYPE(texture_mag_filter)
MAKETYPE(texture_wrap)
MAKETYPE(tex_env_target)
MAKETYPE(tex_env_parameter)
static void init_enums () {
	#define D(NAME) add(tolower_gensym(#NAME+3),NAME)
	primitive_type
	.D(GL_POINTS)
	.D(GL_LINES)
	.D(GL_LINE_LOOP)
	.D(GL_LINE_STRIP)
	.D(GL_TRIANGLES)
	.D(GL_TRIANGLE_STRIP)
	.D(GL_TRIANGLE_FAN)
	.D(GL_QUADS)
	.D(GL_QUAD_STRIP)
	.D(GL_POLYGON)
	;
	capability
	.D(GL_ALPHA_TEST)
	.D(GL_AUTO_NORMAL)
	.D(GL_BLEND)
	.D(GL_CLIP_PLANE0)
	.D(GL_CLIP_PLANE1)
	.D(GL_CLIP_PLANE2)
	.D(GL_CLIP_PLANE3)
	.D(GL_CLIP_PLANE4)
	.D(GL_CLIP_PLANE5)
	.D(GL_COLOR_LOGIC_OP)
	.D(GL_COLOR_MATERIAL)
	.D(GL_COLOR_SUM)
	.D(GL_COLOR_TABLE)
	.D(GL_CONVOLUTION_1D)
	.D(GL_CONVOLUTION_2D)
	.D(GL_CULL_FACE)
	.D(GL_DEPTH_TEST)
	.D(GL_DITHER)
	.D(GL_FOG)
	.D(GL_HISTOGRAM)
	.D(GL_INDEX_LOGIC_OP)
	.D(GL_LIGHT0)
	.D(GL_LIGHT1)
	.D(GL_LIGHT2)
	.D(GL_LIGHT3)
	.D(GL_LIGHT4)
	.D(GL_LIGHT5)
	.D(GL_LIGHT6)
	.D(GL_LIGHT7)
	.D(GL_LIGHTING)
	.D(GL_LINE_SMOOTH)
	.D(GL_LINE_STIPPLE)
	.D(GL_MAP1_COLOR_4)
	.D(GL_MAP1_INDEX)
	.D(GL_MAP1_NORMAL)
	.D(GL_MAP1_TEXTURE_COORD_1)
	.D(GL_MAP1_TEXTURE_COORD_2)
	.D(GL_MAP1_TEXTURE_COORD_3)
	.D(GL_MAP1_TEXTURE_COORD_4)
	.D(GL_MAP1_VERTEX_3)
	.D(GL_MAP1_VERTEX_4)
	.D(GL_MAP2_COLOR_4)
	.D(GL_MAP2_INDEX)
	.D(GL_MAP2_NORMAL)
	.D(GL_MAP2_TEXTURE_COORD_1)
	.D(GL_MAP2_TEXTURE_COORD_2)
	.D(GL_MAP2_TEXTURE_COORD_3)
	.D(GL_MAP2_TEXTURE_COORD_4)
	.D(GL_MAP2_VERTEX_3)
	.D(GL_MAP2_VERTEX_4)
	.D(GL_MINMAX)
	.D(GL_MULTISAMPLE)
	.D(GL_NORMALIZE)
	.D(GL_POINT_SMOOTH)
	.D(GL_POINT_SPRITE)
	.D(GL_POLYGON_OFFSET_FILL)
	.D(GL_POLYGON_OFFSET_LINE)
	.D(GL_POLYGON_OFFSET_POINT)
	.D(GL_POLYGON_SMOOTH)
	.D(GL_POLYGON_STIPPLE)
	.D(GL_POST_COLOR_MATRIX_COLOR_TABLE)
	.D(GL_POST_CONVOLUTION_COLOR_TABLE)
	.D(GL_RESCALE_NORMAL)
	.D(GL_SAMPLE_ALPHA_TO_COVERAGE)
	.D(GL_SAMPLE_ALPHA_TO_ONE)
	.D(GL_SAMPLE_COVERAGE)
	.D(GL_SEPARABLE_2D)
	.D(GL_SCISSOR_TEST)
	.D(GL_STENCIL_TEST)
	.D(GL_TEXTURE_1D)
	.D(GL_TEXTURE_2D)
	.D(GL_TEXTURE_3D)
	.D(GL_TEXTURE_CUBE_MAP)
	.D(GL_TEXTURE_GEN_Q)
	.D(GL_TEXTURE_GEN_R)
	.D(GL_TEXTURE_GEN_S)
	.D(GL_TEXTURE_GEN_T)
	.D(GL_VERTEX_PROGRAM_POINT_SIZE)
	.D(GL_VERTEX_PROGRAM_TWO_SIDE)
	;
	client_state_capability
	.D(GL_COLOR_ARRAY)
	.D(GL_EDGE_FLAG_ARRAY)
	.D(GL_FOG_COORD_ARRAY)
	.D(GL_INDEX_ARRAY)
	.D(GL_NORMAL_ARRAY)
	.D(GL_SECONDARY_COLOR_ARRAY)
	.D(GL_TEXTURE_COORD_ARRAY)
	.D(GL_VERTEX_ARRAY)
	;
	which_side
	.D(GL_FRONT)
	.D(GL_BACK)
	.D(GL_FRONT_AND_BACK)
	;
	texture_target
	.D(GL_TEXTURE_1D)
	.D(GL_TEXTURE_2D)
	.D(GL_TEXTURE_3D)
	.D(GL_TEXTURE_CUBE_MAP)
	;
	blend_equation
	.D(GL_FUNC_ADD)
	.D(GL_FUNC_SUBTRACT)
	.D(GL_FUNC_REVERSE_SUBTRACT)
	.D(GL_MIN)
	.D(GL_MAX)
	;
	blend_func
	.D(GL_ZERO)
	.D(GL_ONE)
	.D(GL_SRC_COLOR)
	.D(GL_ONE_MINUS_SRC_COLOR)
	.D(GL_DST_COLOR)
	.D(GL_ONE_MINUS_DST_COLOR)
	.D(GL_SRC_ALPHA)
	.D(GL_ONE_MINUS_SRC_ALPHA)
	.D(GL_DST_ALPHA)
	.D(GL_ONE_MINUS_DST_ALPHA)
	.D(GL_CONSTANT_COLOR)
	.D(GL_ONE_MINUS_CONSTANT_COLOR)
	.D(GL_CONSTANT_ALPHA)
	.D(GL_ONE_MINUS_CONSTANT_ALPHA)
	.D(GL_SRC_ALPHA_SATURATE) // not supposed to be available as dfactor
	;
	copy_pixels_type
	.D(GL_COLOR)
	.D(GL_DEPTH)
	.D(GL_STENCIL)
	;
	depth_func
	.D(GL_NEVER)
	.D(GL_LESS)
	.D(GL_EQUAL)
	.D(GL_LEQUAL)
	.D(GL_GREATER)
	.D(GL_NOTEQUAL)
	.D(GL_GEQUAL)
	.D(GL_ALWAYS)
	;
	front_face_mode
	.D(GL_CW)
	.D(GL_CCW)
	;
	logic_op
	.D(GL_CLEAR)
	.D(GL_SET)
	.D(GL_COPY)
	.D(GL_COPY_INVERTED)
	.D(GL_NOOP)
	.D(GL_INVERT)
	.D(GL_AND)
	.D(GL_NAND)
	.D(GL_OR)
	.D(GL_NOR)
	.D(GL_XOR)
	.D(GL_EQUIV)
	.D(GL_AND_REVERSE)
	.D(GL_AND_INVERTED)
	.D(GL_OR_REVERSE)
	.D(GL_OR_INVERTED)
	;
	stencil_func // same as depth func
	.D(GL_NEVER)
	.D(GL_LESS)
	.D(GL_LEQUAL)
	.D(GL_GREATER)
	.D(GL_GEQUAL)
	.D(GL_EQUAL)
	.D(GL_NOTEQUAL)
	.D(GL_ALWAYS)
	;
	pixel_store
	.D(GL_PACK_SWAP_BYTES)
	.D(GL_PACK_LSB_FIRST)
	.D(GL_PACK_ROW_LENGTH)
	.D(GL_PACK_IMAGE_HEIGHT)
	.D(GL_PACK_SKIP_PIXELS)
	.D(GL_PACK_SKIP_ROWS)
	.D(GL_PACK_SKIP_IMAGES)
	.D(GL_PACK_ALIGNMENT)
	.D(GL_UNPACK_SWAP_BYTES)
	.D(GL_UNPACK_LSB_FIRST)
	.D(GL_UNPACK_ROW_LENGTH)
	.D(GL_UNPACK_IMAGE_HEIGHT)
	.D(GL_UNPACK_SKIP_PIXELS)
	.D(GL_UNPACK_SKIP_ROWS)
	.D(GL_UNPACK_SKIP_IMAGES)
	.D(GL_UNPACK_ALIGNMENT)
	;
	pixel_transfer
	.D(GL_MAP_COLOR)
	.D(GL_MAP_STENCIL)
	.D(GL_INDEX_SHIFT)
	.D(GL_INDEX_OFFSET)
	.D(GL_RED_SCALE)
	.D(GL_RED_BIAS)
	.D(GL_GREEN_SCALE)
	.D(GL_GREEN_BIAS)
	.D(GL_BLUE_SCALE)
	.D(GL_BLUE_BIAS)
	.D(GL_ALPHA_SCALE)
	.D(GL_ALPHA_BIAS)
	.D(GL_DEPTH_SCALE)
	.D(GL_DEPTH_BIAS)
	#ifdef ARB
	.D(GL_POST_COLOR_MATRIX_RED_SCALE)
	.D(GL_POST_COLOR_MATRIX_GREEN_SCALE)
	.D(GL_POST_COLOR_MATRIX_BLUE_SCALE)
	.D(GL_POST_COLOR_MATRIX_ALPHA_SCALE)
	.D(GL_POST_COLOR_MATRIX_RED_BIAS)
	.D(GL_POST_COLOR_MATRIX_GREEN_BIAS)
	.D(GL_POST_COLOR_MATRIX_BLUE_BIAS)
	.D(GL_POST_COLOR_MATRIX_ALPHA_BIAS)
	.D(GL_POST_CONVOLUTION_RED_SCALE)
	.D(GL_POST_CONVOLUTION_GREEN_SCALE)
	.D(GL_POST_CONVOLUTION_BLUE_SCALE)
	.D(GL_POST_CONVOLUTION_ALPHA_SCALE)
	.D(GL_POST_CONVOLUTION_RED_BIAS)
	.D(GL_POST_CONVOLUTION_GREEN_BIAS)
	.D(GL_POST_CONVOLUTION_BLUE_BIAS)
	.D(GL_POST_CONVOLUTION_ALPHA_BIAS)
	#endif
	;
	shade_model
	.D(GL_FLAT)
	.D(GL_SMOOTH)
	;
	stencil_op
	.D(GL_KEEP)
	.D(GL_ZERO)
	.D(GL_REPLACE)
	.D(GL_INCR)
	.D(GL_INCR_WRAP)
	.D(GL_DECR)
	.D(GL_DECR_WRAP)
	.D(GL_INVERT)
	;
	render_mode
	.D(GL_RENDER)
	.D(GL_SELECT)
	.D(GL_FEEDBACK)
	;
	buffer_mode
	.D(GL_NONE) // not always acceptable
	.D(GL_FRONT_AND_BACK) // not always acceptable
	.D(GL_FRONT_LEFT)
	.D(GL_FRONT_RIGHT)
	.D(GL_BACK_LEFT)
	.D(GL_BACK_RIGHT)
	.D(GL_FRONT)
	.D(GL_BACK)
	.D(GL_LEFT)
	.D(GL_RIGHT)
	.D(GL_AUX0)
	.D(GL_AUX1)
	.D(GL_AUX2)
	.D(GL_AUX3)
	;
	material_mode
	.D(GL_EMISSION)
	.D(GL_AMBIENT)
	.D(GL_DIFFUSE)
	.D(GL_SPECULAR)
	.D(GL_AMBIENT_AND_DIFFUSE)
	;
	tex_target
	.D(GL_TEXTURE_2D)
	.D(GL_PROXY_TEXTURE_2D)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_X)
	.D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
	.D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
	.D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	.D(GL_PROXY_TEXTURE_CUBE_MAP)
	;
	copy_tex_target
	.D(GL_TEXTURE_2D)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_X)
	.D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
	.D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
	.D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	;
	tex_iformat
	.D(GL_ALPHA)
	.D(GL_ALPHA4)
	.D(GL_ALPHA8)
	.D(GL_ALPHA12)
	.D(GL_ALPHA16)
	.D(GL_COMPRESSED_ALPHA)
	.D(GL_COMPRESSED_LUMINANCE)
	.D(GL_COMPRESSED_LUMINANCE_ALPHA)
	.D(GL_COMPRESSED_INTENSITY)
	.D(GL_COMPRESSED_RGB)
	.D(GL_COMPRESSED_RGBA)
	.D(GL_DEPTH_COMPONENT)
	.D(GL_DEPTH_COMPONENT16)
	.D(GL_DEPTH_COMPONENT24)
	.D(GL_DEPTH_COMPONENT32)
	.D(GL_LUMINANCE)
	.D(GL_LUMINANCE4)
	.D(GL_LUMINANCE8)
	.D(GL_LUMINANCE12)
	.D(GL_LUMINANCE16)
	.D(GL_LUMINANCE_ALPHA)
	.D(GL_LUMINANCE4_ALPHA4)
	.D(GL_LUMINANCE6_ALPHA2)
	.D(GL_LUMINANCE8_ALPHA8)
	.D(GL_LUMINANCE12_ALPHA4)
	.D(GL_LUMINANCE12_ALPHA12)
	.D(GL_LUMINANCE16_ALPHA16)
	.D(GL_INTENSITY)
	.D(GL_INTENSITY4)
	.D(GL_INTENSITY8)
	.D(GL_INTENSITY12)
	.D(GL_INTENSITY16)
	.D(GL_RGB)
	.D(GL_R3_G3_B2)
	.D(GL_RGB4)
	.D(GL_RGB5)
	.D(GL_RGB8)
	.D(GL_RGB10)
	.D(GL_RGB12)
	.D(GL_RGB16)
	.D(GL_RGBA)
	.D(GL_RGBA2)
	.D(GL_RGBA4)
	.D(GL_RGB5_A1)
	.D(GL_RGBA8)
	.D(GL_RGB10_A2)
	.D(GL_RGBA12)
	.D(GL_RGBA16)
	.D(GL_SLUMINANCE)
	.D(GL_SLUMINANCE8)
	.D(GL_SLUMINANCE_ALPHA)
	.D(GL_SLUMINANCE8_ALPHA8)
	.D(GL_SRGB)
	.D(GL_SRGB8)
	.D(GL_SRGB_ALPHA)
	.D(GL_SRGB8_ALPHA8)
	;
	tex_format
	.D(GL_COLOR_INDEX)
	.D(GL_RED)
	.D(GL_GREEN)
	.D(GL_BLUE)
	.D(GL_ALPHA)
	.D(GL_RGB)
	.D(GL_BGR)
	.D(GL_RGBA)
	.D(GL_BGRA)
	.D(GL_LUMINANCE)
	.D(GL_LUMINANCE_ALPHA)
	;
	tex_type
	.D(GL_UNSIGNED_BYTE)
	.D(GL_BYTE)
	.D(GL_BITMAP)
	.D(GL_UNSIGNED_SHORT)
	.D(GL_SHORT)
	.D(GL_UNSIGNED_INT)
	.D(GL_INT)
	.D(GL_FLOAT)
	.D(GL_UNSIGNED_BYTE_3_3_2)
	.D(GL_UNSIGNED_BYTE_2_3_3_REV)
	.D(GL_UNSIGNED_SHORT_5_6_5)
	.D(GL_UNSIGNED_SHORT_5_6_5_REV)
	.D(GL_UNSIGNED_SHORT_4_4_4_4)
	.D(GL_UNSIGNED_SHORT_4_4_4_4_REV)
	.D(GL_UNSIGNED_SHORT_5_5_5_1)
	.D(GL_UNSIGNED_SHORT_1_5_5_5_REV)
	.D(GL_UNSIGNED_INT_8_8_8_8)
	.D(GL_UNSIGNED_INT_8_8_8_8_REV)
	.D(GL_UNSIGNED_INT_10_10_10_2)
	.D(GL_UNSIGNED_INT_2_10_10_10_REV)
	;
	polygon_mode
	.D(GL_POINT)
	.D(GL_LINE)
	.D(GL_FILL) // only for mesh2
	;
	hint_target
	.D(GL_FOG_HINT)
        .D(GL_GENERATE_MIPMAP_HINT)
        .D(GL_LINE_SMOOTH_HINT)
        .D(GL_PERSPECTIVE_CORRECTION_HINT)
	.D(GL_POINT_SMOOTH_HINT)
	.D(GL_POLYGON_SMOOTH_HINT)
	.D(GL_TEXTURE_COMPRESSION_HINT)
	.D(GL_FRAGMENT_SHADER_DERIVATIVE_HINT)
	;
	hint_mode
	.D(GL_FASTEST)
	.D(GL_NICEST)
	.D(GL_DONT_CARE)
	;
	accum_op
	.D(GL_ACCUM)
	.D(GL_LOAD)
	.D(GL_ADD)
	.D(GL_MULT)
	.D(GL_RETURN)
	;
	fog_param
	.D(GL_FOG_MODE)
	.D(GL_FOG_DENSITY)
	.D(GL_FOG_START)
	.D(GL_FOG_END)
	.D(GL_FOG_INDEX)
	.D(GL_FOG_COORD_SRC)
	;
	list_mode
	.D(GL_COMPILE)
	.D(GL_COMPILE_AND_EXECUTE)
	;
	texture_parameter
	.D(GL_TEXTURE_MIN_FILTER)
	.D(GL_TEXTURE_MAG_FILTER)
	.D(GL_TEXTURE_MIN_LOD)
	.D(GL_TEXTURE_MAX_LOD)
	.D(GL_TEXTURE_BASE_LEVEL)
	.D(GL_TEXTURE_MAX_LEVEL)
	.D(GL_TEXTURE_WRAP_S)
	.D(GL_TEXTURE_WRAP_T)
	.D(GL_TEXTURE_WRAP_R)
	.D(GL_TEXTURE_PRIORITY)
	.D(GL_TEXTURE_COMPARE_MODE)
	.D(GL_TEXTURE_COMPARE_FUNC)
	.D(GL_DEPTH_TEXTURE_MODE)
	.D(GL_GENERATE_MIPMAP)
	;
	texture_min_filter
	.D(GL_NEAREST)
	.D(GL_LINEAR)
	.D(GL_NEAREST_MIPMAP_NEAREST)
	.D(GL_LINEAR_MIPMAP_NEAREST)
	.D(GL_NEAREST_MIPMAP_LINEAR)
	.D(GL_LINEAR_MIPMAP_LINEAR)
	;
	texture_mag_filter
	.D(GL_NEAREST)
	.D(GL_LINEAR)
	;
	texture_wrap
	.D(GL_CLAMP)
	.D(GL_CLAMP_TO_BORDER)
	.D(GL_CLAMP_TO_EDGE)
	.D(GL_MIRRORED_REPEAT)
	.D(GL_REPEAT)
	;
	tex_env_target
	.D(GL_TEXTURE_ENV)
	.D(GL_TEXTURE_FILTER_CONTROL)
	.D(GL_POINT_SPRITE)
	;
	tex_env_parameter
	.D(GL_TEXTURE_ENV_MODE)
	.D(GL_TEXTURE_LOD_BIAS)
	.D(GL_COMBINE_RGB)
	.D(GL_COMBINE_ALPHA)
	.D(GL_SRC0_RGB)
	.D(GL_SRC1_RGB)
	.D(GL_SRC2_RGB)
	.D(GL_SRC0_ALPHA)
	.D(GL_SRC1_ALPHA)
	.D(GL_SRC2_ALPHA)
	.D(GL_OPERAND0_RGB)
	.D(GL_OPERAND1_RGB)
	.D(GL_OPERAND2_RGB)
	.D(GL_OPERAND0_ALPHA)
	.D(GL_OPERAND1_ALPHA)
	.D(GL_OPERAND2_ALPHA)
	.D(GL_RGB_SCALE)
	.D(GL_ALPHA_SCALE)
	.D(GL_COORD_REPLACE)
	;
}
// comments in the class body list those functions not supported by GF but supported by GEM in openGL dir.
\class GFGL : FObject {
	\constructor () {}
	~GFGL() {}
	\decl 0 accum (t_atom op, float value) {glAccum(accum_op(op),value);}
	// ActiveTextureARB
	\decl 0 alpha_func (t_atom func, float ref) {glAlphaFunc(depth_func(func),ref);} // clamp
	// AreTexturesResident
	\decl 0 array_element (int i) {glArrayElement(i);}
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
	\decl 0 clip_plane(int plane, float a0, float a1, float a2, float a3) {
		if (plane<0 || plane>=6) RAISE("plane must be a number between 0 and 5 incl");
		double dv[] = {a0,a1,a2,a3};
		glClipPlane(GL_CLIP_PLANE0+plane,dv);
	}
	\decl 0 color (...) {switch (argc) {
		case 3: glColor3f(argv[0],argv[1],argv[2]); break;
		case 4: glColor4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 3 or 4 args");
	}}
	\decl 0 color_mask (bool r, bool g, bool b, bool a) {glColorMask(r,g,b,a);}
	\decl 0 color_material (t_atom face, t_atom mode) {glColorMaterial(which_side(face),material_mode(mode));}
	\decl 0 copy_pixels (int x, int y, int width, int height, t_atom type) {
		glCopyPixels(x,y,width,height,copy_pixels_type(type));}

	// please review and check that format and internalFormat (iformat) are not confused
	\decl 0 copy_tex_image_1D (t_atom target, int level, t_atom iformat, int x, int y, int width, int border ) {
		glCopyTexImage1D(copy_tex_target(target),level,tex_iformat(iformat),x,y,width,border);}
	\decl 0 copy_tex_image_2D (t_atom target, int level, t_atom iformat, int x, int y, int width, int height, int border) {
		glCopyTexImage2D(copy_tex_target(target),level,tex_iformat(iformat),x,y,width,height,border);}		
	\decl 0 copy_tex_sub_image_1D (t_atom target, int level, int xoffset, int x, int y, int width) {
		if (copy_tex_target(target)!=GL_TEXTURE_1D) RAISE("must be texture_1d");
		glCopyTexSubImage1D(copy_tex_target(target),level,xoffset,x,y,width);}
	\decl 0 copy_tex_sub_image_2D (t_atom target, int level, int xoffset, int yoffset, int x, int y, int width, int height) {
		glCopyTexSubImage2D(copy_tex_target(target),level,xoffset,yoffset,x,y,width,height);}
	\decl 0 copy_tex_sub_image_3D (t_atom target, int level, int xoffset, int yoffset, int zoffset, int x, int y, int width, int height) {
		if (copy_tex_target(target)!=GL_TEXTURE_3D) RAISE("must be texture_3d");
		glCopyTexSubImage3D(copy_tex_target(target),level,xoffset,yoffset,zoffset,x,y,width,height);}

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
		case 3: glEvalMesh1(polygon_mode(argv[0]),argv[1],argv[2]); break;
		case 5: glEvalMesh2(polygon_mode(argv[0]),argv[1],argv[2],argv[3],argv[4]); break;
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
	\decl 0 fog (...) {
		if (argc<2) RAISE("at least 2 args");
		GLenum pname = fog_param(argv[0]);
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
	\decl 0 hint (t_atom target, t_atom mode) {glHint(hint_target(target),hint_mode(mode));}
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
		case 3: glMapGrid1f(argv[0],argv[1],argv[2]); break;
		case 6: glMapGrid2f(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]); break;
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
	\decl 0 new_list (uint32 list, t_atom mode) {glNewList(list,list_mode(mode));}
	\decl 0 normal (float x, float y, float z) {glNormal3f(x,y,z);}
	\decl 0 ortho(float left, float right, float bottom, float top, float near_val, float far_val) {
		glOrtho(left,right,bottom,top,near_val,far_val);
	}
	\decl 0 pass_through (float token) {glPassThrough(token);}
	\decl 0 pixel_store    (t_atom pname, float param) {glPixelStoref(   pixel_store   (pname),param);}
	\decl 0 pixel_transfer (t_atom pname, float param) {glPixelTransferf(pixel_transfer(pname),param);}
	\decl 0 pixel_zoom (float xfactor, float yfactor) {glPixelZoom(xfactor,yfactor);}
	\decl 0 point_size (float size) {glPointSize(size);}
	\decl 0 polygon_mode (t_atom face, t_atom mode) {glPolygonMode(which_side(face),polygon_mode(mode));}
	\decl 0 polygon_offset (float factor, float units) {glPolygonOffset(factor,units);}
	\decl 0 pop_attrib () {glPopAttrib();}
	\decl 0 pop_client_attrib () {glPopClientAttrib();}
	\decl 0 pop_matrix ()  {glPopMatrix();}
	\decl 0 pop_name () {glPopName();}
	// PrioritizeTextures // GLAPI void GLAPIENTRY glPrioritizeTextures(int n, const GLuint *textures, const float *priorities); // clamp
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

	\decl 0 select_buffer (int size, void *buffer) {glSelectBuffer(size,(uint32 *)buffer);}

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

	// glTexEnvfv(t_atom target, t_atom pname, const float *params);
	\decl 0 tex_env (...) {
		if (argc<3) RAISE("minimum 3 args");
		GLenum target = tex_env_target(argv[0]);
		GLenum pname = tex_env_parameter(argv[1]);
		switch (pname) {
		  
		  default: RAISE("...");
		}
		//glTexEnvfv(target,pname,params);
		//glTexEnviv(target,pname,params);
	}
	// glTexGenfv(t_atom coord,  t_atom pname, const float *params);
	\decl 0 tex_gen (...) {
		if (argc<3) RAISE("minimum 3 args");
		GLenum target = texture_target(argv[0]);
		GLenum pname = texture_parameter(argv[1]);
		switch (pname) {
		  default: RAISE("...");
		}
		//glTexGenfv(target,pname,params);
	}

	\decl 0 tex_image_1D(t_atom target, int level, t_atom iformat, int width,                        int border, t_atom format, t_atom type, const GLvoid *pixels) {
		if (tex_target(target)!=GL_TEXTURE_1D) RAISE("must be texture_1d");
		glTexImage1D(tex_target(target),level,tex_iformat(iformat),width,             border,tex_format(format),tex_type(type),pixels);}
	\decl 0 tex_image_2D(t_atom target, int level, t_atom iformat, int width, int height,            int border, t_atom format, t_atom type, const GLvoid *pixels) {
		glTexImage2D(tex_target(target),level,tex_iformat(iformat),width,height,      border,tex_format(format),tex_type(type),pixels);}
	\decl 0 glTexImage3D(t_atom target, int level, t_atom iformat, int width, int height, int depth, int border, t_atom format, t_atom type, const GLvoid *pixels) {
		if (tex_target(target)!=GL_TEXTURE_3D) RAISE("must be texture_3d");
		glTexImage3D(tex_target(target),level,tex_iformat(iformat),width,height,depth,border,tex_format(format),tex_type(type),pixels);} // not in GEM

	\decl 0 tex_parameter (...) {
		if (argc<3) RAISE("minimum 3 args");
		GLenum target = texture_target(argv[0]);
		GLenum pname = texture_parameter(argv[1]);
		switch (pname) {
		  case GL_TEXTURE_MIN_FILTER: break;
		  case GL_TEXTURE_MAG_FILTER: break;
		  case GL_TEXTURE_MIN_LOD: break;
		  case GL_TEXTURE_MAX_LOD: break;
		  case GL_TEXTURE_BASE_LEVEL: break;
		  case GL_TEXTURE_MAX_LEVEL: break;
		  case GL_TEXTURE_WRAP_S: case GL_TEXTURE_WRAP_T: case GL_TEXTURE_WRAP_R: break;
		  case GL_TEXTURE_PRIORITY: break;
		  case GL_TEXTURE_COMPARE_MODE: break;
		  case GL_TEXTURE_COMPARE_FUNC: break;
		  case GL_DEPTH_TEXTURE_MODE: break;
		  case GL_GENERATE_MIPMAP: break;
		  default: RAISE("...");
		}
		//glTexParameteriv(target,pname,params);
		//glTexParameterfv(target,pname,params);
	}

	\decl 0 tex_sub_image_1D(t_atom target, int level, int xoffset,                           int width,                        t_atom format, t_atom type, const GLvoid *pixels) {
		if (tex_target(target)!=GL_TEXTURE_1D) RAISE("must be texture_1d");
		glTexSubImage1D(tex_target(target),level,xoffset,                width,             tex_format(format),tex_type(type),pixels);}
	\decl 0 tex_sub_image_2D(t_atom target, int level, int xoffset, int yoffset,              int width, int height,            t_atom format, t_atom type, const GLvoid *pixels) {
		glTexSubImage2D(tex_target(target),level,xoffset,yoffset,        width,height,      tex_format(format),tex_type(type),pixels);}
	\decl 0 tex_sub_image_3D(t_atom target, int level, int xoffset, int yoffset, int zoffset, int width, int height, int depth, t_atom format, t_atom type, const GLvoid *pixels) {
		if (tex_target(target)!=GL_TEXTURE_3D) RAISE("must be texture_3d");
		glTexSubImage3D(tex_target(target),level,xoffset,yoffset,zoffset,width,height,depth,tex_format(format),tex_type(type),pixels);} // not in GEM

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
