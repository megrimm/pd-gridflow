#include "../gridflow.h.fcs"
extern "C" {
void mmx_uint8_map_or(long,uint8*,uint8);
void mmx_uint8_zip_or(long,uint8*,uint8*);
void mmx_int16_map_or(long,int16*,int16);
void mmx_int16_zip_or(long,int16*,int16*);
void mmx_uint8_map_and(long,uint8*,uint8);
void mmx_uint8_zip_and(long,uint8*,uint8*);
void mmx_int16_map_and(long,int16*,int16);
void mmx_int16_zip_and(long,int16*,int16*);
void mmx_uint8_map_sub(long,uint8*,uint8);
void mmx_uint8_zip_sub(long,uint8*,uint8*);
void mmx_int16_map_sub(long,int16*,int16);
void mmx_int16_zip_sub(long,int16*,int16*);
void mmx_uint8_map_xor(long,uint8*,uint8);
void mmx_uint8_zip_xor(long,uint8*,uint8*);
void mmx_int16_map_xor(long,int16*,int16);
void mmx_int16_zip_xor(long,int16*,int16*);
void mmx_uint8_map_add(long,uint8*,uint8);
void mmx_uint8_zip_add(long,uint8*,uint8*);
void mmx_int16_map_add(long,int16*,int16);
void mmx_int16_zip_add(long,int16*,int16*);

}; /* extern */
#include <stdlib.h>
void startup_mmx_loader () {/*bogus*/}
void startup_mmx () {
	if (getenv("NO_MMX")) return;
	post("startup_cpu: using MMX optimisations");
	op_dict[string("|")]->on_uint8.map = mmx_uint8_map_or;
op_dict[string("|")]->on_uint8.zip = mmx_uint8_zip_or;
op_dict[string("|")]->on_int16.map = mmx_int16_map_or;
op_dict[string("|")]->on_int16.zip = mmx_int16_zip_or;
op_dict[string("&")]->on_uint8.map = mmx_uint8_map_and;
op_dict[string("&")]->on_uint8.zip = mmx_uint8_zip_and;
op_dict[string("&")]->on_int16.map = mmx_int16_map_and;
op_dict[string("&")]->on_int16.zip = mmx_int16_zip_and;
op_dict[string("-")]->on_uint8.map = mmx_uint8_map_sub;
op_dict[string("-")]->on_uint8.zip = mmx_uint8_zip_sub;
op_dict[string("-")]->on_int16.map = mmx_int16_map_sub;
op_dict[string("-")]->on_int16.zip = mmx_int16_zip_sub;
op_dict[string("^")]->on_uint8.map = mmx_uint8_map_xor;
op_dict[string("^")]->on_uint8.zip = mmx_uint8_zip_xor;
op_dict[string("^")]->on_int16.map = mmx_int16_map_xor;
op_dict[string("^")]->on_int16.zip = mmx_int16_zip_xor;
op_dict[string("+")]->on_uint8.map = mmx_uint8_map_add;
op_dict[string("+")]->on_uint8.zip = mmx_uint8_zip_add;
op_dict[string("+")]->on_int16.map = mmx_int16_map_add;
op_dict[string("+")]->on_int16.zip = mmx_int16_zip_add;

}
