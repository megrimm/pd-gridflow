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
#include <GL/glew.h>
#include <ctype.h>

struct EnumType {
	const char *name;
	struct EnumInfo {
		t_symbol *s;
		int n;
		// if n==0 : this is not the name of a 'get' attribute
		// if n>0  : this is the name of a 'get' attribute that has n arguments
		// if n<0  : this is the name of a 'get' attribute that is currently not supported.
	};
	typedef std::map<t_symbol *,GLenum>  forward_t;  forward_t  forward;
	typedef std::map<GLenum,EnumInfo>   backward_t; backward_t backward;
	EnumType (const char *name) {this->name = name;}
	GLenum operator () (const t_atom &a) {return (*this)(*(const t_atom2 *)&a);}
	GLenum operator () (const t_atom2 &a) {
		if (a.a_type==A_FLOAT) {
			float f = (float)a;
			backward_t::iterator it = backward.find(GLenum(a));
			if (it==backward.end()) RAISE("unknown %s GLenum %d (at least not allowed in this context)",name,int(a));
			return f;
		} else if (a.a_type==A_SYMBOL) {
			forward_t::iterator it = forward.find((t_symbol *)a);
			if (it==forward.end()) RAISE("unknown %s '%s'",name,((t_symbol *)a)->s_name);
			return it->second;
		} else RAISE("to %s: expected float or symbol",name);
	}
	t_symbol *reverse (GLenum e) {
		backward_t::iterator it = backward.find(e);
		if (it==backward.end()) RAISE("unknown %s GLenum %d (at least not allowed in this context)",name,int(e));
		return it->second.s;
	}
	int argc (GLenum e) {
		backward_t::iterator it = backward.find(e);
		if (it==backward.end()) RAISE("unknown %s GLenum %d (at least not allowed in this context)",name,int(e));
		return it->second.n;
	}
 	EnumType &add (t_symbol *s, GLenum i, int n=0) {forward[s]=i; EnumInfo &ei = backward[i]; ei.s=s; ei.n=n; return *this;}
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
MAKETYPE(tex_env_argument)
MAKETYPE(tex_env_function)
MAKETYPE(tex_gen_coord)
MAKETYPE(tex_gen_parameter)
MAKETYPE(tex_gen_mode)
MAKETYPE(matrix_mode)
MAKETYPE(material_parameter)
MAKETYPE(map_eval_type)
MAKETYPE(call_list_type)
MAKETYPE(draw_pixels_format)
MAKETYPE(light_parameter)
MAKETYPE(light_model_parameter)
MAKETYPE(light_model_color_control)
MAKETYPE(feedback_buffer_type)
MAKETYPE(get_parameter)
MAKETYPE(tex_depth_mode)
MAKETYPE(texture_compare_mode)
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
	.D(GL_CLIP_PLANE0).D(GL_CLIP_PLANE1)
	.D(GL_CLIP_PLANE2).D(GL_CLIP_PLANE3)
	.D(GL_CLIP_PLANE4).D(GL_CLIP_PLANE5)
	.D(GL_COLOR_LOGIC_OP).D(GL_COLOR_MATERIAL).D(GL_COLOR_SUM).D(GL_COLOR_TABLE)
	.D(GL_CONVOLUTION_1D).D(GL_CONVOLUTION_2D)
	.D(GL_CULL_FACE)
	.D(GL_DEPTH_TEST)
	.D(GL_DITHER)
	.D(GL_FOG)
	.D(GL_HISTOGRAM)
	.D(GL_INDEX_LOGIC_OP)
	.D(GL_LIGHT0).D(GL_LIGHT1).D(GL_LIGHT2).D(GL_LIGHT3)
	.D(GL_LIGHT4).D(GL_LIGHT5).D(GL_LIGHT6).D(GL_LIGHT7)
	.D(GL_LIGHTING)
	.D(GL_LINE_SMOOTH)
	.D(GL_LINE_STIPPLE)
	.D(GL_MAP1_COLOR_4).D(GL_MAP1_INDEX).D(GL_MAP1_NORMAL)
	.D(GL_MAP1_TEXTURE_COORD_1).D(GL_MAP1_TEXTURE_COORD_2)
	.D(GL_MAP1_TEXTURE_COORD_3).D(GL_MAP1_TEXTURE_COORD_4)
	.D(GL_MAP1_VERTEX_3).D(GL_MAP1_VERTEX_4)
	.D(GL_MAP2_COLOR_4).D(GL_MAP2_INDEX).D(GL_MAP2_NORMAL)
	.D(GL_MAP2_TEXTURE_COORD_1).D(GL_MAP2_TEXTURE_COORD_2)
	.D(GL_MAP2_TEXTURE_COORD_3).D(GL_MAP2_TEXTURE_COORD_4)
	.D(GL_MAP2_VERTEX_3).D(GL_MAP2_VERTEX_4)
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
	.D(GL_SRC_COLOR).D(GL_ONE_MINUS_SRC_COLOR)
	.D(GL_DST_COLOR).D(GL_ONE_MINUS_DST_COLOR)
	.D(GL_SRC_ALPHA).D(GL_ONE_MINUS_SRC_ALPHA)
	.D(GL_DST_ALPHA).D(GL_ONE_MINUS_DST_ALPHA)
	.D(GL_CONSTANT_COLOR).D(GL_ONE_MINUS_CONSTANT_COLOR)
	.D(GL_CONSTANT_ALPHA).D(GL_ONE_MINUS_CONSTANT_ALPHA)
	.D(GL_SRC_ALPHA_SATURATE) // not supposed to be available as dfactor
	;
	copy_pixels_type
	.D(GL_COLOR)
	.D(GL_DEPTH)
	.D(GL_STENCIL)
	;
	depth_func
	.D(GL_NEVER)	.D(GL_ALWAYS)
	.D(GL_LESS)	.D(GL_GEQUAL)
	.D(GL_EQUAL)	.D(GL_NOTEQUAL)
	.D(GL_LEQUAL)	.D(GL_GREATER)
	;
	front_face_mode
	.D(GL_CW)
	.D(GL_CCW)
	;
	logic_op
	.D(GL_CLEAR)		.D(GL_SET)
	.D(GL_AND)		.D(GL_NAND)
	.D(GL_AND_REVERSE)	.D(GL_OR_INVERTED)
	.D(GL_COPY)		.D(GL_COPY_INVERTED)
	.D(GL_AND_INVERTED)	.D(GL_OR_REVERSE)
	.D(GL_NOOP)		.D(GL_INVERT)
	.D(GL_XOR)		.D(GL_EQUIV)
	.D(GL_OR)		.D(GL_NOR)
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
	.D(GL_RED_SCALE)	.D(GL_RED_BIAS)
	.D(GL_GREEN_SCALE)	.D(GL_GREEN_BIAS)
	.D(GL_BLUE_SCALE)	.D(GL_BLUE_BIAS)
	.D(GL_ALPHA_SCALE)	.D(GL_ALPHA_BIAS)
	.D(GL_DEPTH_SCALE)	.D(GL_DEPTH_BIAS)
	#ifdef ARB
	.D(GL_POST_COLOR_MATRIX_RED_SCALE)	.D(GL_POST_COLOR_MATRIX_RED_BIAS)
	.D(GL_POST_COLOR_MATRIX_GREEN_SCALE)	.D(GL_POST_COLOR_MATRIX_GREEN_BIAS)
	.D(GL_POST_COLOR_MATRIX_BLUE_SCALE)	.D(GL_POST_COLOR_MATRIX_BLUE_BIAS)
	.D(GL_POST_COLOR_MATRIX_ALPHA_SCALE)	.D(GL_POST_COLOR_MATRIX_ALPHA_BIAS)
	.D(GL_POST_CONVOLUTION_RED_SCALE)	.D(GL_POST_CONVOLUTION_GREEN_BIAS)
	.D(GL_POST_CONVOLUTION_RED_BIAS)	.D(GL_POST_CONVOLUTION_GREEN_SCALE)
	.D(GL_POST_CONVOLUTION_BLUE_SCALE)	.D(GL_POST_CONVOLUTION_BLUE_BIAS)
	.D(GL_POST_CONVOLUTION_ALPHA_SCALE)	.D(GL_POST_CONVOLUTION_ALPHA_BIAS)
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
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_X).D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y).D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z).D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	.D(GL_PROXY_TEXTURE_CUBE_MAP)
	;
	copy_tex_target
	.D(GL_TEXTURE_2D)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_X).D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y).D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
	.D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z).D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
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
	.D(GL_TEXTURE_BORDER_COLOR) //?
	;
	texture_min_filter
	.D(GL_NEAREST).D(GL_NEAREST_MIPMAP_NEAREST).D(GL_NEAREST_MIPMAP_LINEAR)
	 .D(GL_LINEAR) .D(GL_LINEAR_MIPMAP_NEAREST) .D(GL_LINEAR_MIPMAP_LINEAR)
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
	.D(GL_SRC0_RGB).D(GL_SRC0_ALPHA).D(GL_OPERAND0_RGB).D(GL_OPERAND0_ALPHA)
	.D(GL_SRC1_RGB).D(GL_SRC1_ALPHA).D(GL_OPERAND1_RGB).D(GL_OPERAND1_ALPHA)
	.D(GL_SRC2_RGB).D(GL_SRC2_ALPHA).D(GL_OPERAND2_RGB).D(GL_OPERAND2_ALPHA)
	.D(GL_RGB_SCALE)
	.D(GL_ALPHA_SCALE)
	.D(GL_COORD_REPLACE)
	;
	tex_env_argument
	.D(GL_ADD) //
	.D(GL_ADD_SIGNED)
	.D(GL_INTERPOLATE)
	.D(GL_MODULATE) //
	.D(GL_DECAL) //
	.D(GL_BLEND) //
	.D(GL_REPLACE) //
	.D(GL_SUBTRACT)
	.D(GL_COMBINE) //
	.D(GL_TEXTURE)
	.D(GL_CONSTANT)
	.D(GL_PRIMARY_COLOR)
	.D(GL_PREVIOUS)
	.D(GL_SRC_COLOR).D(GL_ONE_MINUS_SRC_COLOR)
	.D(GL_SRC_ALPHA).D(GL_ONE_MINUS_SRC_ALPHA)
	;
	tex_env_function
	.D(GL_ADD)
	.D(GL_MODULATE)
	.D(GL_DECAL)
	.D(GL_BLEND)
	.D(GL_REPLACE)
	.D(GL_COMBINE)
        ;
	tex_gen_coord
	.D(GL_S)
	.D(GL_T)
	.D(GL_R)
	.D(GL_Q)
	;
	tex_gen_parameter
	.D(GL_TEXTURE_GEN_MODE)
	.D(GL_OBJECT_PLANE)
	.D(GL_EYE_PLANE)
	;
	tex_gen_mode
	.D(GL_OBJECT_LINEAR)
	.D(GL_EYE_LINEAR)
	.D(GL_SPHERE_MAP)
	.D(GL_NORMAL_MAP)
	.D(GL_REFLECTION_MAP)
	;
	tex_depth_mode
	.D(GL_LUMINANCE)
	.D(GL_INTENSITY)
	.D(GL_ALPHA)
	;
	texture_compare_mode
	.D(GL_COMPARE_R_TO_TEXTURE)
	.D(GL_NONE)
	;
	matrix_mode
	.D(GL_MODELVIEW)
        .D(GL_PROJECTION)
        .D(GL_TEXTURE)
	;
	material_parameter
	.D(GL_AMBIENT)
	.D(GL_DIFFUSE)
	.D(GL_SPECULAR)
	.D(GL_EMISSION)
	.D(GL_SHININESS)
	.D(GL_AMBIENT_AND_DIFFUSE)
	.D(GL_COLOR_INDEXES)
	;
	map_eval_type
	.D(GL_MAP1_VERTEX_3)
	.D(GL_MAP1_VERTEX_4)
	.D(GL_MAP1_INDEX)
	.D(GL_MAP1_COLOR_4)
	.D(GL_MAP1_NORMAL)
	.D(GL_MAP1_TEXTURE_COORD_1)
	.D(GL_MAP1_TEXTURE_COORD_2)
	.D(GL_MAP1_TEXTURE_COORD_3)
	.D(GL_MAP1_TEXTURE_COORD_4)
	;
	call_list_type
	.D(GL_BYTE)
	.D(GL_UNSIGNED_BYTE)
	.D(GL_SHORT)
	.D(GL_UNSIGNED_SHORT)
	.D(GL_INT)
	.D(GL_UNSIGNED_INT)
	.D(GL_FLOAT)
	.D(GL_2_BYTES)
	.D(GL_3_BYTES)
	.D(GL_4_BYTES)
	;
	draw_pixels_format
	.D(GL_COLOR_INDEX)
	.D(GL_STENCIL_INDEX)
	.D(GL_DEPTH_COMPONENT)
	.D(GL_RGB)
	.D(GL_BGR)
	.D(GL_RGBA)
	.D(GL_BGRA)
	.D(GL_RED)
	.D(GL_GREEN)
	.D(GL_BLUE)
	.D(GL_ALPHA)
	.D(GL_LUMINANCE)
	.D(GL_LUMINANCE_ALPHA)
	;
	light_parameter
	.D(GL_AMBIENT)
	.D(GL_DIFFUSE)
	.D(GL_SPECULAR)
	.D(GL_POSITION)
	.D(GL_SPOT_CUTOFF)
	.D(GL_SPOT_DIRECTION)
	.D(GL_SPOT_EXPONENT)
	.D(GL_CONSTANT_ATTENUATION)
	.D(GL_LINEAR_ATTENUATION)
	.D(GL_QUADRATIC_ATTENUATION)
	;
	light_model_parameter
	.D(GL_LIGHT_MODEL_AMBIENT)
	.D(GL_LIGHT_MODEL_LOCAL_VIEWER)
	.D(GL_LIGHT_MODEL_COLOR_CONTROL)
	.D(GL_LIGHT_MODEL_TWO_SIDE)
	;
	light_model_color_control
	.D(GL_SEPARATE_SPECULAR_COLOR)
	.D(GL_SINGLE_COLOR)
	;	
	feedback_buffer_type
	.D(GL_2D)
	.D(GL_3D)
	.D(GL_3D_COLOR)
	.D(GL_3D_COLOR_TEXTURE)
	.D(GL_4D_COLOR_TEXTURE)
	;
	#undef D
	#define D(NAME,ARGS...) add(tolower_gensym(#NAME+3),NAME,ARGS)
	get_parameter // 1
	.D(GL_ACCUM_ALPHA_BITS,1).D(GL_ACCUM_BLUE_BITS,1).D(GL_ACCUM_GREEN_BITS,1).D(GL_ACCUM_RED_BITS,1)
	.D(GL_BLEND_COLOR,4)
	.D(GL_ACCUM_CLEAR_VALUE,4)
	.D(GL_ACTIVE_TEXTURE,1)
	.D(GL_ALIASED_POINT_SIZE_RANGE,2).D(GL_ALIASED_LINE_WIDTH_RANGE,2)
	.D(GL_ALPHA_BIAS,1).D(GL_ALPHA_BITS,1).D(GL_ALPHA_SCALE,1).D(GL_ALPHA_TEST,1).D(GL_ALPHA_TEST_REF,1)
	.D(GL_ARRAY_BUFFER_BINDING,1)
	.D(GL_ATTRIB_STACK_DEPTH,1)
	.D(GL_AUTO_NORMAL,1)
	.D(GL_AUX_BUFFERS,1)
	.D(GL_BLEND,1)
	.D(GL_BLUE_BIAS,1).D(GL_BLUE_BITS,1).D(GL_BLUE_SCALE,1)
	.D(GL_CLIENT_ATTRIB_STACK_DEPTH,1)
	.D(GL_CLIP_PLANE0,1).D(GL_CLIP_PLANE1,1).D(GL_CLIP_PLANE2,1)
	.D(GL_CLIP_PLANE3,1).D(GL_CLIP_PLANE4,1).D(GL_CLIP_PLANE5,1)
	.D(GL_COLOR_ARRAY,1).D(GL_COLOR_ARRAY_BUFFER_BINDING,1).D(GL_COLOR_ARRAY_SIZE,1)
	.D(GL_COLOR_ARRAY_STRIDE,1)
	.D(GL_COLOR_CLEAR_VALUE,4)
	.D(GL_COLOR_LOGIC_OP,1)
	.D(GL_COLOR_MATERIAL,1).D(GL_COLOR_MATERIAL_FACE,1).D(GL_COLOR_MATERIAL_PARAMETER,1)
	.D(GL_COLOR_MATRIX,16)
	.D(GL_COLOR_MATRIX_STACK_DEPTH,1)
	.D(GL_COLOR_SUM,1).D(GL_COLOR_TABLE,1)
	.D(GL_COLOR_WRITEMASK,4)
	.D(GL_CONVOLUTION_1D,1).D(GL_CONVOLUTION_2D,1)
	.D(GL_CULL_FACE,1)
	.D(GL_CURRENT_COLOR,4)
	.D(GL_CURRENT_FOG_COORD,1)
	.D(GL_CURRENT_INDEX,1)
	.D(GL_CURRENT_NORMAL,3)
	.D(GL_CURRENT_PROGRAM,1)
	.D(GL_CURRENT_RASTER_DISTANCE,1).D(GL_CURRENT_RASTER_INDEX,1).D(GL_CURRENT_RASTER_POSITION_VALID,1)
	.D(GL_CURRENT_RASTER_COLOR,4).D(GL_CURRENT_RASTER_POSITION,4)
	.D(GL_CURRENT_RASTER_SECONDARY_COLOR,4).D(GL_CURRENT_RASTER_TEXTURE_COORDS,4)
	.D(GL_CURRENT_SECONDARY_COLOR,4)
	.D(GL_CURRENT_TEXTURE_COORDS,4)
	//.D(GL_ALPHA_TEST_FUNC,1e)
	//.D(GL_BLEND_DST_ALPHA,1e).D(GL_BLEND_DST_RGB,1e)
	//.D(GL_BLEND_EQUATION_RGB,1e).D(GL_BLEND_EQUATION_ALPHA,1e)
	//.D(GL_BLEND_SRC_ALPHA,1e).D(GL_BLEND_SRC_RGB,1e)
	//.D(GL_CLIENT_ACTIVE_TEXTURE,1e) (minus GL_TEXTURE0)
	//.D(GL_COLOR_ARRAY_TYPE,1e)
	//.D(GL_CULL_FACE_MODE,1e)
	//.D(GL_COMPRESSED_TEXTURE_FORMATS,-1) // goes with .D(GL_NUM_COMPRESSED_TEXTURE_FORMATS,1) and GLenum
	.D(GL_DEPTH_BIAS,1).D(GL_DEPTH_BITS,1).D(GL_DEPTH_CLEAR_VALUE,1)
	//GL_DEPTH_FUNC,1e
	.D(GL_DEPTH_RANGE,1).D(GL_DEPTH_SCALE,1).D(GL_DEPTH_TEST,1).D(GL_DEPTH_WRITEMASK,1)
	.D(GL_DITHER,1)
	.D(GL_DOUBLEBUFFER,1)
	.D(GL_DRAW_BUFFER,1)
	//GL_DRAW_BUFFERi,1e
	.D(GL_EDGE_FLAG,1).D(GL_EDGE_FLAG_ARRAY,1).D(GL_EDGE_FLAG_ARRAY_BUFFER_BINDING,1).D(GL_EDGE_FLAG_ARRAY_STRIDE,1)
	.D(GL_ELEMENT_ARRAY_BUFFER_BINDING,1)
	.D(GL_FEEDBACK_BUFFER_SIZE,1)
	//GL_FEEDBACK_BUFFER_TYPE,1e
	.D(GL_FOG,1)
	.D(GL_FOG_COORD_ARRAY,1).D(GL_FOG_COORD_ARRAY_BUFFER_BINDING,1)
	.D(GL_FOG_COORD_ARRAY_STRIDE,1).D(GL_FOG_COORD_ARRAY_TYPE,1)
	.D(GL_FOG_COORD_SRC,1).D(GL_FOG_COLOR,4).D(GL_FOG_DENSITY,1).D(GL_FOG_END,1)
	.D(GL_FOG_HINT,1).D(GL_FOG_INDEX,1).D(GL_FOG_MODE,1).D(GL_FOG_START,1)
	.D(GL_FRAGMENT_SHADER_DERIVATIVE_HINT,1)
	.D(GL_FRONT_FACE,1)
	.D(GL_GENERATE_MIPMAP_HINT,1)
	.D(GL_GREEN_BIAS,1).D(GL_GREEN_BITS,1).D(GL_GREEN_SCALE,1)
	.D(GL_HISTOGRAM,1)
	.D(GL_INDEX_ARRAY,1).D(GL_INDEX_ARRAY_BUFFER_BINDING,1).D(GL_INDEX_ARRAY_STRIDE,1).D(GL_INDEX_ARRAY_TYPE,1)
	.D(GL_INDEX_BITS,1).D(GL_INDEX_CLEAR_VALUE,1).D(GL_INDEX_LOGIC_OP,1).D(GL_INDEX_MODE,1)
	.D(GL_INDEX_OFFSET,1).D(GL_INDEX_SHIFT,1).D(GL_INDEX_WRITEMASK,1)
	//GL_LIGHTi,1
	.D(GL_LIGHTING,1)
	.D(GL_LIGHT_MODEL_AMBIENT,4)
	//.D(GL_LIGHT_MODEL_COLOR_CONTROL,1e)
	.D(GL_LIGHT_MODEL_LOCAL_VIEWER,1)
	.D(GL_LIGHT_MODEL_TWO_SIDE,1)
	.D(GL_LINE_SMOOTH,1).D(GL_LINE_SMOOTH_HINT,1)
	.D(GL_LINE_STIPPLE,1).D(GL_LINE_STIPPLE_PATTERN,1).D(GL_LINE_STIPPLE_REPEAT,1)
	.D(GL_LINE_WIDTH,1).D(GL_LINE_WIDTH_GRANULARITY,1).D(GL_LINE_WIDTH_RANGE,2)
	.D(GL_LIST_BASE,1).D(GL_LIST_INDEX,1).D(GL_LIST_MODE,1)
	.D(GL_LOGIC_OP_MODE,1)
	.D(GL_MAP1_COLOR_4,1)
	.D(GL_MAP1_GRID_DOMAIN,2).D(GL_MAP1_GRID_SEGMENTS,1).D(GL_MAP1_INDEX,1).D(GL_MAP1_NORMAL,1)
	.D(GL_MAP1_TEXTURE_COORD_1,1).D(GL_MAP1_TEXTURE_COORD_2,1).D(GL_MAP1_TEXTURE_COORD_3,1).D(GL_MAP1_TEXTURE_COORD_4,1)
	.D(GL_MAP1_VERTEX_3,1).D(GL_MAP1_VERTEX_4,1).D(GL_MAP2_COLOR_4,1)
	.D(GL_MAP2_GRID_DOMAIN,4).D(GL_MAP2_GRID_SEGMENTS,2).D(GL_MAP2_INDEX,1).D(GL_MAP2_NORMAL,1)
	.D(GL_MAP2_TEXTURE_COORD_1,1).D(GL_MAP2_TEXTURE_COORD_2,1).D(GL_MAP2_TEXTURE_COORD_3,1).D(GL_MAP2_TEXTURE_COORD_4,1)
	.D(GL_MAP2_VERTEX_3,1).D(GL_MAP2_VERTEX_4,1)
	.D(GL_MAP_COLOR,1).D(GL_MAP_STENCIL,1)
	.D(GL_MATRIX_MODE,1)
	.D(GL_MAX_3D_TEXTURE_SIZE,1)
	.D(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH,1)
	.D(GL_MAX_ATTRIB_STACK_DEPTH,1)
	.D(GL_MAX_CLIP_PLANES,1)
	.D(GL_MAX_COLOR_MATRIX_STACK_DEPTH,1)
	.D(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,1)
	.D(GL_MAX_CUBE_MAP_TEXTURE_SIZE,1)
	.D(GL_MAX_DRAW_BUFFERS,1)
	.D(GL_MAX_ELEMENTS_INDICES,1).D(GL_MAX_ELEMENTS_VERTICES,1)
	.D(GL_MAX_EVAL_ORDER,1)
	.D(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,1)
	.D(GL_MAX_LIGHTS,1)
	.D(GL_MAX_LIST_NESTING,1)
	.D(GL_MAX_MODELVIEW_STACK_DEPTH,1)
	.D(GL_MAX_NAME_STACK_DEPTH,1)
	.D(GL_MAX_PIXEL_MAP_TABLE,1)
	.D(GL_MAX_PROJECTION_STACK_DEPTH,1)
	.D(GL_MAX_TEXTURE_COORDS,1)
	.D(GL_MAX_TEXTURE_IMAGE_UNITS,1)
	.D(GL_MAX_TEXTURE_LOD_BIAS,1)
	.D(GL_MAX_TEXTURE_SIZE,1)
	.D(GL_MAX_TEXTURE_STACK_DEPTH,1)
	.D(GL_MAX_TEXTURE_UNITS,1)
	.D(GL_MAX_VARYING_FLOATS,1)
	.D(GL_MAX_VERTEX_ATTRIBS,1)
	.D(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,1)
	.D(GL_MAX_VERTEX_UNIFORM_COMPONENTS,1)
	.D(GL_MAX_VIEWPORT_DIMS,2)
	.D(GL_MINMAX,1)
	.D(GL_MODELVIEW_MATRIX,16)
	.D(GL_MODELVIEW_STACK_DEPTH,1)
	.D(GL_NAME_STACK_DEPTH,1)
	.D(GL_NORMAL_ARRAY,1)
	.D(GL_NORMAL_ARRAY_BUFFER_BINDING,1)
	.D(GL_NORMAL_ARRAY_STRIDE,1)
	//.D(GL_NORMAL_ARRAY_TYPE,1e)
	.D(GL_NORMALIZE,1)
	.D(GL_PACK_ALIGNMENT,1)
	.D(GL_PACK_IMAGE_HEIGHT,1)
	.D(GL_PACK_LSB_FIRST,1)
	.D(GL_PACK_ROW_LENGTH,1)
	.D(GL_PACK_SKIP_IMAGES,1)
	.D(GL_PACK_SKIP_PIXELS,1)
	.D(GL_PACK_SKIP_ROWS,1)
	.D(GL_PACK_SWAP_BYTES,1)
	.D(GL_PERSPECTIVE_CORRECTION_HINT,1)
	.D(GL_PIXEL_MAP_A_TO_A_SIZE,1).D(GL_PIXEL_MAP_B_TO_B_SIZE,1).D(GL_PIXEL_MAP_G_TO_G_SIZE,1).D(GL_PIXEL_MAP_R_TO_R_SIZE,1)
	.D(GL_PIXEL_MAP_I_TO_A_SIZE,1).D(GL_PIXEL_MAP_I_TO_B_SIZE,1).D(GL_PIXEL_MAP_I_TO_G_SIZE,1).D(GL_PIXEL_MAP_I_TO_R_SIZE,1)
	.D(GL_PIXEL_MAP_I_TO_I_SIZE,1).D(GL_PIXEL_MAP_S_TO_S_SIZE,1)
	.D(GL_PIXEL_PACK_BUFFER_BINDING,1)
	.D(GL_PIXEL_UNPACK_BUFFER_BINDING,1)
	.D(GL_POINT_DISTANCE_ATTENUATION,3)
	.D(GL_POINT_FADE_THRESHOLD_SIZE,1)
	.D(GL_POINT_SIZE,1)
	.D(GL_POINT_SIZE_GRANULARITY,1)
	.D(GL_POINT_SIZE_MAX,1)
	.D(GL_POINT_SIZE_MIN,1)
	.D(GL_POINT_SIZE_RANGE,2)
	.D(GL_POINT_SMOOTH,1)
	.D(GL_POINT_SMOOTH_HINT,1)
	.D(GL_POINT_SPRITE,1)
	//.D(GL_POLYGON_MODE,2e)
	.D(GL_POLYGON_OFFSET_FACTOR,1)
	.D(GL_POLYGON_OFFSET_UNITS,1)
	.D(GL_POLYGON_OFFSET_FILL,1)
	.D(GL_POLYGON_OFFSET_LINE,1)
	.D(GL_POLYGON_OFFSET_POINT,1)
	.D(GL_POLYGON_SMOOTH,1)
	.D(GL_POLYGON_SMOOTH_HINT,1)
	.D(GL_POLYGON_STIPPLE,1)
	.D(GL_POST_COLOR_MATRIX_COLOR_TABLE,1)
	.D(GL_POST_COLOR_MATRIX_RED_BIAS,1)
	.D(GL_POST_COLOR_MATRIX_GREEN_BIAS,1)
	.D(GL_POST_COLOR_MATRIX_BLUE_BIAS,1)
	.D(GL_POST_COLOR_MATRIX_ALPHA_BIAS,1)
	.D(GL_POST_COLOR_MATRIX_RED_SCALE,1)
	.D(GL_POST_COLOR_MATRIX_GREEN_SCALE,1)
	.D(GL_POST_COLOR_MATRIX_BLUE_SCALE,1)
	.D(GL_POST_COLOR_MATRIX_ALPHA_SCALE,1)
	.D(GL_POST_CONVOLUTION_COLOR_TABLE,1)
	.D(GL_POST_CONVOLUTION_RED_BIAS,1)
	.D(GL_POST_CONVOLUTION_GREEN_BIAS,1)
	.D(GL_POST_CONVOLUTION_BLUE_BIAS,1)
	.D(GL_POST_CONVOLUTION_ALPHA_BIAS,1)
	.D(GL_POST_CONVOLUTION_RED_SCALE,1)
	.D(GL_POST_CONVOLUTION_GREEN_SCALE,1)
	.D(GL_POST_CONVOLUTION_BLUE_SCALE,1)
	.D(GL_POST_CONVOLUTION_ALPHA_SCALE,1)
	.D(GL_PROJECTION_MATRIX,16)
	.D(GL_PROJECTION_STACK_DEPTH,1)
	//.D(GL_READ_BUFFER,1e)
	.D(GL_RED_BIAS,1).D(GL_RED_BITS,1).D(GL_RED_SCALE,1)
	//.D(GL_RENDER_MODE,1e)
	.D(GL_RESCALE_NORMAL,1)
	.D(GL_RGBA_MODE,1)
	.D(GL_SAMPLE_BUFFERS,1)
	.D(GL_SAMPLE_COVERAGE_VALUE,1)
	.D(GL_SAMPLE_COVERAGE_INVERT,1)
	.D(GL_SAMPLES,1)
	.D(GL_SCISSOR_BOX,4)
	.D(GL_SCISSOR_TEST,1)
	.D(GL_SECONDARY_COLOR_ARRAY,1)
	.D(GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING,1)
	.D(GL_SECONDARY_COLOR_ARRAY_SIZE,1)
	.D(GL_SECONDARY_COLOR_ARRAY_STRIDE,1)
	.D(GL_SECONDARY_COLOR_ARRAY_TYPE,1)
	.D(GL_SELECTION_BUFFER_SIZE,1)
	.D(GL_SEPARABLE_2D,1)
	.D(GL_SHADE_MODEL,1)
	.D(GL_SMOOTH_LINE_WIDTH_RANGE,2).D(GL_SMOOTH_LINE_WIDTH_GRANULARITY,1)
	.D(GL_SMOOTH_POINT_SIZE_RANGE,2).D(GL_SMOOTH_POINT_SIZE_GRANULARITY,1)
	.D(GL_STENCIL_BACK_FAIL,1)
	.D(GL_STENCIL_BACK_PASS_DEPTH_FAIL,1)
	.D(GL_STENCIL_BACK_FUNC,1)
	.D(GL_STENCIL_BACK_PASS_DEPTH_PASS,1)
	.D(GL_STENCIL_BACK_REF,1)
	.D(GL_STENCIL_BACK_VALUE_MASK,1)
	.D(GL_STENCIL_BACK_WRITEMASK,1)
	.D(GL_STENCIL_BITS,1)
	.D(GL_STENCIL_CLEAR_VALUE,1)
	//.D(GL_STENCIL_FAIL,1e)
	//.D(GL_STENCIL_FUNC,1e)
	//.D(GL_STENCIL_PASS_DEPTH_FAIL,1e)
	//.D(GL_STENCIL_PASS_DEPTH_PASS,1e)
	.D(GL_STENCIL_REF,1)
	.D(GL_STENCIL_TEST,1)
	.D(GL_STENCIL_VALUE_MASK,1)
	.D(GL_STENCIL_WRITEMASK,1)
	.D(GL_STEREO,1)
	.D(GL_SUBPIXEL_BITS,1)
	.D(GL_TEXTURE_1D,1)
	.D(GL_TEXTURE_BINDING_1D,1)
	.D(GL_TEXTURE_2D,1)
	.D(GL_TEXTURE_BINDING_2D,1)
	.D(GL_TEXTURE_3D,1)
	.D(GL_TEXTURE_BINDING_3D,1)
	.D(GL_TEXTURE_BINDING_CUBE_MAP,1)
	//.D(GL_TEXTURE_COMPRESSION_HINT,1e)
	.D(GL_TEXTURE_COORD_ARRAY,1)
	.D(GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING,1)
	.D(GL_TEXTURE_COORD_ARRAY_SIZE,1)
	.D(GL_TEXTURE_COORD_ARRAY_STRIDE,1)
	//.D(GL_TEXTURE_COORD_ARRAY_TYPE,1e)
	.D(GL_TEXTURE_CUBE_MAP,1)
	.D(GL_TEXTURE_GEN_Q,1)
	.D(GL_TEXTURE_GEN_R,1)
	.D(GL_TEXTURE_GEN_S,1)
	.D(GL_TEXTURE_GEN_T,1)
	.D(GL_TEXTURE_MATRIX,16)
	.D(GL_TEXTURE_STACK_DEPTH,1)
	.D(GL_TRANSPOSE_COLOR_MATRIX,16)
	.D(GL_TRANSPOSE_MODELVIEW_MATRIX,16)
	.D(GL_TRANSPOSE_PROJECTION_MATRIX,16)
	.D(GL_TRANSPOSE_TEXTURE_MATRIX,16)
	.D(GL_UNPACK_ALIGNMENT,1)
	.D(GL_UNPACK_IMAGE_HEIGHT,1)
	.D(GL_UNPACK_LSB_FIRST,1)
	.D(GL_UNPACK_ROW_LENGTH,1)
	.D(GL_UNPACK_SKIP_IMAGES,1)
	.D(GL_UNPACK_SKIP_PIXELS,1)
	.D(GL_UNPACK_SKIP_ROWS,1)
	.D(GL_UNPACK_SWAP_BYTES,1)
	.D(GL_VERTEX_ARRAY,1)
	.D(GL_VERTEX_ARRAY_BUFFER_BINDING,1)
	.D(GL_VERTEX_ARRAY_SIZE,1)
	.D(GL_VERTEX_ARRAY_STRIDE,1)
	.D(GL_VERTEX_ARRAY_TYPE,1)
	.D(GL_VERTEX_PROGRAM_POINT_SIZE,1)
	.D(GL_VERTEX_PROGRAM_TWO_SIDE,1)
	.D(GL_VIEWPORT,4)
	.D(GL_ZOOM_X,1)
	.D(GL_ZOOM_Y,1)
	;
	// strings:
	//.D(GL_VENDOR,1).D(GL_RENDERER,1).D(GL_VERSION,1).D(GL_SHADING_LANGUAGE_VERSION,1).D(GL_EXTENSIONS,1) // string

	// pointers:
	//.D(GL_COLOR_ARRAY_POINTER,1).D(GL_EDGE_FLAG_ARRAY_POINTER,1).D(GL_FOG_COORD_ARRAY_POINTER,1)
	//.D(GL_FEEDBACK_BUFFER_POINTER,1).D(GL_INDEX_ARRAY_POINTER,1).D(GL_NORMAL_ARRAY_POINTER,1)
	//.D(GL_SECONDARY_COLOR_ARRAY_POINTER,1).D(GL_SELECTION_BUFFER_POINTER,1)
	//.D(GL_TEXTURE_COORD_ARRAY_POINTER,1).D(GL_VERTEX_ARRAY_POINTER,1)

	// maps:
	//.D(GL_MAP1_COLOR_4).D(GL_MAP1_INDEX).D(GL_MAP1_NORMAL)
	//.D(GL_MAP1_TEXTURE_COORD_1).D(GL_MAP1_TEXTURE_COORD_2).D(GL_MAP1_TEXTURE_COORD_3).D(GL_MAP1_TEXTURE_COORD_4)
	//.D(GL_MAP1_VERTEX_3).D(GL_MAP1_VERTEX_4)
	//.D(GL_MAP2_COLOR_4).D(GL_MAP2_INDEX).D(GL_MAP2_NORMAL)
	//.D(GL_MAP2_TEXTURE_COORD_1).D(GL_MAP2_TEXTURE_COORD_2).D(GL_MAP2_TEXTURE_COORD_3).D(GL_MAP2_TEXTURE_COORD_4)
	//.D(GL_MAP2_VERTEX_3).D(GL_MAP2_VERTEX_4)
}
// comments in the class body list those functions not supported by GF but supported by GEM in openGL dir.
\class GFGL : FObject {
	\constructor () {}
	~GFGL() {}
	\decl 0 accum (t_atom op, float value) {glAccum(accum_op(op),value);}
	\decl 0 alpha_func (t_atom func, float ref) {glAlphaFunc(depth_func(func),ref);} // clamp
	\decl 0 are_textures_resident (...) {
		uint32 textures[argc];
		GLboolean residences[argc];
		t_atom2 a[argc];
		for (int i=0; i<argc; i++) textures[i] = argv[i];
		if (glAreTexturesResident(argc,textures,residences)) for (int i=0; i<argc; i++) set_atom(a+i,1);
		else						     for (int i=0; i<argc; i++) set_atom(a+i,residences[i]);
		outlet_list(outlets[0],&s_list,argc,a);
	}
	\decl 0 array_element (int i) {glArrayElement(i);}
	\decl 0 begin (t_atom2 a) {glBegin(primitive_type(a));}
	\decl 0 bind_texture (t_atom target, uint32 texture) {glBindTexture(texture_target(target),texture);}
	\decl 0 bitmap (int width, int height, float xorig, float yorig, float xmove, float ymove, void *bitmap) {
		glBitmap(width,height,xorig,yorig,xmove,ymove,(const GLubyte *)bitmap);}
	\decl 0 blend_func (t_atom sfactor, t_atom dfactor) {glBlendFunc(blend_func(sfactor),blend_func(dfactor));}
	\decl 0 call_list (uint32 list) {glCallList(list);}
	\decl 0 call_lists (int n, t_atom type, void *lists) {glCallLists(n,call_list_type(type),lists);} // not in GEM
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
		if (!glTexSubImage3D) RAISE("need OpenGL 1.2");
		if (copy_tex_target(target)!=GL_TEXTURE_3D) RAISE("must be texture_3d");
		glCopyTexSubImage3D(copy_tex_target(target),level,xoffset,yoffset,zoffset,x,y,width,height);}

	\decl 0 cull_face (t_atom mode) {glCullFace(which_side(mode));}
	\decl 0 delete_lists (uint32 list, int range) {glDeleteLists(list,range);}  // not in GEM
	\decl 0 delete_textures (...) {
		uint32 textures[argc];
		for (int i=0; i<argc; i++) textures[i] = argv[i];
		glDeleteTextures(argc,textures);
	}
	\decl 0 depth_func (t_atom func) {glDepthFunc(depth_func(func));}
	\decl 0 depth_mask (bool flag) {glDepthMask(flag);}
	\decl 0 depth_range (float near_val, float far_val) {glDepthRange(near_val,far_val);} // clamp
	\decl 0 disable_client_state (t_atom cap) {glDisable(client_state_capability(cap));}
	\decl 0 disable (t_atom cap) {glDisable(capability(cap));}
	\decl 0 draw_arrays (t_atom mode, int first, int count) {glDrawArrays(primitive_type(mode),first,count);}
	\decl 0 draw_buffer (t_atom mode) {glDrawBuffer(buffer_mode(mode));}
	// DrawElements // GLAPI void GLAPIENTRY glDrawElements(t_atom mode, int count, t_atom type, const GLvoid *indices);
	//\decl 0 draw_elements (t_atom mode, int count, t_atom type, const GLvoid *indices) {
		//glDrawElements(t_atom mode, int count, t_atom type, const GLvoid *indices);}
	// GLAPI void GLAPIENTRY glDrawPixels(int width, int height, t_atom format, t_atom type, const GLvoid *pixels); // not in GEM
	\decl 0 draw_pixels (int width, int height, t_atom format, t_atom type, void *pixels) {
		glDrawPixels(width,height,draw_pixels_format(format),tex_type(type),pixels);
	}
	\decl 0 edge_flag (bool flag) {glEdgeFlag(flag);}
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
			float fv[4]={argv[1],argv[2],argv[3],argv[4]};
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
	\decl 0 gen_lists (int n) {
		if (n<1) RAISE("$1 must be at least 1");
		uint32 list = glGenLists(n);
		//t_atom a[n]; for (int i=0; i<n; i++) set_atom(a+i,list+i);
		//outlet_anything(outlets[0],&s_list,n,a);
		outlet_float(outlets[0],list);
	}
	\decl 0 gen_textures (int n) {
		if (n<1) RAISE("$1 must be at least 1");
		uint32 textures[n]; glGenTextures(n,textures);
		t_atom a[n]; for (int i=0; i<n; i++) set_atom(a+i,textures[i]);
		outlet_anything(outlets[0],&s_list,n,a);
	}
	// GetError
	// GLAPI void GLAPIENTRY glGetLightfv(t_atom light, t_atom pname, float *params); // not in GEM
	// GetMap[dfi]v
	// GetPointerv
	// GetString
	\decl 0 get (t_atom pname) {
		GLenum e = get_parameter(pname);
		t_symbol *s = get_parameter.reverse(e);
		float fv[16];
		int n=get_parameter.argc(e); // only support single-value arguments for now
		if (n<=0) RAISE("reading this property isn't currently supported by GridFlow");
		//post("reading property %d 0x%04X %s n=%d",e,e,s->s_name,n);
		glGetFloatv(e,fv);
		t_atom a[n]; for (int i=0; i<n; i++) set_atom(a+i,fv[i]);
		outlet_anything(outlets[0],s,n,a);
	}
	\decl 0 hint (t_atom target, t_atom mode) {glHint(hint_target(target),hint_mode(mode));}
	\decl 0 index (float value) {glIndexf(value);}
	\decl 0 index_mask (bool flag) {glIndexMask(flag);}
	\decl 0 init_names () {glInitNames();}
	\decl 0 is_enabled (t_atom cap) {outlet_float(outlets[0],glIsEnabled(capability(cap)));}
	\decl 0 is_list    (uint32 list   ) {outlet_float(outlets[0],glIsList(   list   ));}
	\decl 0 is_texture (uint32 texture) {outlet_float(outlets[0],glIsTexture(texture));}
	\decl 0 light (...) {
		if (argc<3) RAISE("minimum 3 args");
		int light = (int)argv[0];
		GLenum pname = light_parameter(argv[1]);
		if (light<0 || light>=8) RAISE("$1 must be a number from 0 to 7");
		switch (pname) {
		  case GL_AMBIENT: case GL_DIFFUSE: case GL_SPECULAR: case GL_POSITION:
						if (argc!=5) RAISE("need 4 floats after $1"); break;
		  case GL_SPOT_DIRECTION:	if (argc!=4) RAISE("need 3 floats after $1"); break;
		  case GL_SPOT_CUTOFF: case GL_SPOT_EXPONENT: case GL_CONSTANT_ATTENUATION: case GL_LINEAR_ATTENUATION:
		  case GL_QUADRATIC_ATTENUATION:if (argc!=2) RAISE("need 1 float after $1");  break;
		  default: RAISE("...");
		}
		float fv[argc-2];
		for (int i=2; i<argc; i++) fv[i-2] = argv[i];
		glLightfv(GL_LIGHT0+light,pname,fv);
	}
	\decl 0 light_model (...) {
		if (argc<2) RAISE("minimum 2 args");
		GLenum pname = light_model_parameter(argv[0]);
		switch (pname) {
		  case GL_LIGHT_MODEL_AMBIENT: if (argc!=5) RAISE("need 4 floats after $1"); break;
		  case GL_LIGHT_MODEL_LOCAL_VIEWER:
		  case GL_LIGHT_MODEL_TWO_SIDE:if (argc!=2) RAISE("need 1 float after $1");  break;
		  case GL_LIGHT_MODEL_COLOR_CONTROL:
		    glLightModelf(pname,light_model_color_control(argv[1]));
		    return;
		  default: RAISE("...");
		}
		float fv[argc-1];
		for (int i=1; i<argc; i++) fv[i-1] = argv[i];
		glLightModelfv(pname,fv);
	}
	\decl 0 line_stipple (int factor, uint16 pattern) {glLineStipple(factor,pattern);}
	\decl 0 line_width (float width) {glLineWidth(width);}
	\decl 0 list_base (uint32 base) {glListBase(base);} // not in GEM
	\decl 0 load_identity () {glLoadIdentity();}
	\decl 0 load_matrix (...) {
		if (argc!=16) RAISE("need 16 args");
		float fv[16]; for (int i=0; i<16; i++) fv[i]=argv[i];
		glLoadMatrixf(fv);
	}
	\decl 0 load_name (uint32 name) {glLoadName(name);}
	\decl 0 load_transpose_matrix (...) {
		if (!glLoadTransposeMatrixf) RAISE("need OpenGL 1.3");
		if (argc!=16) RAISE("need 16 args");
		float fv[16]; for (int i=0; i<16; i++) fv[i]=argv[i];
		glLoadTransposeMatrixf(fv);
	}
	\decl 0 logic_op (t_atom opcode) {glLogicOp(logic_op(opcode));}
	\decl 0 map1 (t_atom target, float u1, float u2, int stride, int order, void *points) {
		glMap1f(map_eval_type(target),u1,u2,stride,order,(const float *)points);
	}
	\decl 0 map2 (t_atom target, float u1, float u2, int ustride, int uorder, float v1, float v2, int vstride, int vorder, void *points) {
		glMap2f(map_eval_type(target),u1,u2,ustride,uorder,v1,v2,vstride,vorder,(const float *)points);
	}
	\decl 0 map_grid (...) {switch (argc) {
		case 3: glMapGrid1f(argv[0],argv[1],argv[2]); break;
		case 6: glMapGrid2f(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]); break;
		default: RAISE("need 3 or 6 args");
	}}
	\decl 0 material (...) {
		if (argc<3) RAISE("need 3 or more args");
		GLenum face = which_side(argv[0]);
		GLenum pname = material_parameter(argv[1]);
		switch(pname) {
		  case GL_AMBIENT: case GL_DIFFUSE: case GL_SPECULAR: case GL_EMISSION: case GL_AMBIENT_AND_DIFFUSE:
					if (argc!=5) RAISE("this $1 needs to be followed by 4 float args"); break;
		  case GL_SHININESS:	if (argc!=2) RAISE("this $1 needs to be followed by 1 float arg");  break;
		  case GL_COLOR_INDEXES:if (argc!=4) RAISE("this $1 needs to be followed by 3 float args"); break;
		  default: RAISE("...");
                }
                float fv[4];
		for (int i=1; i<argc; i++) fv[i-1] = argv[i];
		glMaterialfv(face,pname,fv);
	}
	\decl 0 matrix_mode (t_atom mode) {glMatrixMode(matrix_mode(mode));}
	\decl 0 mult_matrix (...) {
		if (argc!=16) RAISE("need 16 args");
		float fv[16]; for (int i=0; i<16; i++) fv[i]=argv[i];
		glMultMatrixf(fv);
	}
	\decl 0 mult_transpose_matrix (...) {
		if (!glMultTransposeMatrixf) RAISE("need OpenGL 1.3");
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
	//\decl 0 prioritize_textures (...)
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
		glStencilFunc(depth_func(func),ref,mask);}
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
		if (target==GL_TEXTURE_FILTER_CONTROL) {
		 switch (pname) {
		  case GL_TEXTURE_LOD_BIAS:
		    break;
		  default: RAISE("...");
		 }
		} else if (target==GL_TEXTURE_ENV) {
		 switch (pname) {
		  case GL_TEXTURE_ENV_MODE: 
		  case GL_COMBINE_RGB:
		  case GL_COMBINE_ALPHA:
		  case GL_SRC0_RGB  : case GL_SRC1_RGB  : case GL_SRC2_RGB  :
		  case GL_SRC0_ALPHA: case GL_SRC1_ALPHA: case GL_SRC2_ALPHA:
		  case GL_OPERAND0_RGB  : case GL_OPERAND1_RGB  : case GL_OPERAND2_RGB  :
		  case GL_OPERAND0_ALPHA: case GL_OPERAND1_ALPHA: case GL_OPERAND2_ALPHA:
		  case GL_RGB_SCALE:
		  case GL_ALPHA_SCALE:
		  default: RAISE("...");
		 }
		} else if (target==GL_POINT_SPRITE) {
		 switch (pname) {
		  case GL_COORD_REPLACE:
		  default: RAISE("...");
		 }
		}
		//glTexEnvfv(target,pname,params);
		//glTexEnviv(target,pname,params);
	}
	// glTexGenfv(t_atom coord,  t_atom pname, const float *params);
	\decl 0 tex_gen (...) {
		if (argc<3) RAISE("minimum 3 args");
		GLenum coord = tex_gen_coord(argv[0]);
		GLenum pname = tex_gen_parameter(argv[1]);
		switch (pname) {
		  default: RAISE("...");
		}
		//glTexGenfv(target,pname,params);
	}

	\decl 0 tex_image_1D(t_atom target, int level, t_atom iformat, int width,                        int border, t_atom format, t_atom type, void *pixels) {
		if (tex_target(target)!=GL_TEXTURE_1D) RAISE("must be texture_1d");
		glTexImage1D(tex_target(target),level,tex_iformat(iformat),width,             border,tex_format(format),tex_type(type),pixels);}
	\decl 0 tex_image_2D(t_atom target, int level, t_atom iformat, int width, int height,            int border, t_atom format, t_atom type, void *pixels) {
		glTexImage2D(tex_target(target),level,tex_iformat(iformat),width,height,      border,tex_format(format),tex_type(type),pixels);}
	\decl 0 glTexImage3D(t_atom target, int level, t_atom iformat, int width, int height, int depth, int border, t_atom format, t_atom type, void *pixels) {
		if (!glTexImage3D) RAISE("need OpenGL 1.2");
		if (tex_target(target)!=GL_TEXTURE_3D) RAISE("must be texture_3d");
		glTexImage3D(tex_target(target),level,tex_iformat(iformat),width,height,depth,border,tex_format(format),tex_type(type),pixels);} // not in GEM

	\decl 0 tex_parameter (...) {
		if (argc<3) RAISE("minimum 3 args");
		GLenum target = texture_target(argv[0]);
		GLenum pname = texture_parameter(argv[1]);
		switch (pname) {
		  case GL_TEXTURE_MIN_FILTER:   glTexParameteri(target,pname,texture_min_filter(  argv[2])); return;
		  case GL_TEXTURE_MAG_FILTER:   glTexParameteri(target,pname,texture_mag_filter(  argv[2])); return;
		  case GL_TEXTURE_COMPARE_MODE: glTexParameteri(target,pname,texture_compare_mode(argv[2])); break;
		  case GL_TEXTURE_COMPARE_FUNC: glTexParameteri(target,pname,depth_func(          argv[2])); break;
		  case GL_DEPTH_TEXTURE_MODE:   glTexParameteri(target,pname,tex_depth_mode(      argv[2])); break;
		  case GL_TEXTURE_MIN_LOD: case GL_TEXTURE_MAX_LOD: case GL_TEXTURE_PRIORITY: case GL_GENERATE_MIPMAP:
		  case GL_TEXTURE_BASE_LEVEL: case GL_TEXTURE_MAX_LEVEL: glTexParameterf(target,pname,argv[2]); return;
		  case GL_TEXTURE_WRAP_S: case GL_TEXTURE_WRAP_T: case GL_TEXTURE_WRAP_R:
			glTexParameterf(target,pname,texture_wrap(argv[2]));
		  case GL_TEXTURE_BORDER_COLOR: if (argc!=5) RAISE("need 4 args after $2");
			{float fv[4]={argv[2],argv[3],argv[4],argv[5]}; glTexParameterfv(target,pname,fv);} break;
		  default: RAISE("...");
		}
	}

	\decl 0 tex_sub_image_1D(t_atom target, int level, int xoffset,                           int width,                        t_atom format, t_atom type, const GLvoid *pixels) {
		if (tex_target(target)!=GL_TEXTURE_1D) RAISE("must be texture_1d");
		glTexSubImage1D(tex_target(target),level,xoffset,                width,             tex_format(format),tex_type(type),pixels);}
	\decl 0 tex_sub_image_2D(t_atom target, int level, int xoffset, int yoffset,              int width, int height,            t_atom format, t_atom type, const GLvoid *pixels) {
		glTexSubImage2D(tex_target(target),level,xoffset,yoffset,        width,height,      tex_format(format),tex_type(type),pixels);}
	\decl 0 tex_sub_image_3D(t_atom target, int level, int xoffset, int yoffset, int zoffset, int width, int height, int depth, t_atom format, t_atom type, const GLvoid *pixels) {
		if (!glTexSubImage3D) RAISE("need OpenGL 1.2");
		if (tex_target(target)!=GL_TEXTURE_3D) RAISE("must be texture_3d");
		glTexSubImage3D(tex_target(target),level,xoffset,yoffset,zoffset,width,height,depth,tex_format(format),tex_type(type),pixels);} // not in GEM

	\decl 0 translate       (float x, float y, float z) {glTranslatef(x,y,z);}
	\decl 0 vertex (...) {switch (argc) {
		case 2: glVertex2f(argv[0],argv[1]); break;
		case 3: glVertex3f(argv[0],argv[1],argv[2]); break;
		case 4: glVertex4f(argv[0],argv[1],argv[2],argv[3]); break;
		default: RAISE("need 2, 3 or 4 args");
	}}
	\decl 0 viewport (int x, int y, int width, int height) {glViewport(x,y,width,height);}

//////////////////////////////// GLU section

	\decl 0 look_at (float eyex, float eyey, float eyez, float cx, float cy, float cz, float upx, float upy, float upz) {
		gluLookAt(     eyex,       eyey,       eyez,       cx,       cy,       cz,       upx,       upy,       upz);}
	\decl 0 perspective (float fovy, float aspect, float zNear, float zFar) {
		gluPerspective(    fovy,       aspect,       zNear,       zFar);}

//////////////////////////////// ARB section

	// ActiveTextureARB
	// BindProgramARB
	\decl 0 blend_equation (t_atom mode) {
		if (!glBlendEquation) RAISE("need OpenGL ARB_imaging extension");
		glBlendEquation(blend_equation(mode));}
	// GenProgramsARB
	// MultiTexCoord2fARB
	// ProgramEnvParameter4dARB
	// ProgramEnvParameter4fvARB
	// ProgramLocalParameter4fvARB
	// ProgramStringARB
	// Uniform1fARB
	// UseProgramObjectARB
};
\end class {install("gf/gl",1,1);}

void startup_opengl () {
	init_enums();
	\startall
}
