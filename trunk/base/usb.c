/*
	$Id$

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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

#include <usb.h>
#include "grid.h.fcs"

static Ruby cUSB;

struct usb_define {
	const char *name;
	int i;
};

usb_define usb_class_choice[] = {
	{"USB_CLASS_PER_INTERFACE",0},
	{"USB_CLASS_AUDIO",1},
	{"USB_CLASS_COMM",2},
	{"USB_CLASS_HID",3},
	{"USB_CLASS_PRINTER",7},
	{"USB_CLASS_MASS_STORAGE",8},
	{"USB_CLASS_HUB",9},
	{"USB_CLASS_DATA",10},
	{"USB_CLASS_VENDOR_SPEC",0xff},
	{0,0},
};

usb_define usb_descriptor_types_choices[] = {
	{"USB_DT_DEVICE",0x01},
	{"USB_DT_CONFIG",0x02},
	{"USB_DT_STRING",0x03},
	{"USB_DT_INTERFACE",0x04},
	{"USB_DT_ENDPOINT",0x05},
	{"USB_DT_HID",0x21},
	{"USB_DT_REPORT",0x22},
	{"USB_DT_PHYSICAL",0x23},
	{"USB_DT_HUB",0x29},
	{0,0},
};

usb_define usb_descriptor_types_sizes[] = {
	{"USB_DT_DEVICE_SIZE",18},
	{"USB_DT_CONFIG_SIZE",9},
	{"USB_DT_INTERFACE_SIZE",9},
	{"USB_DT_ENDPOINT_SIZE",7},
	{"USB_DT_ENDPOINT_AUDIO_SIZE",9},/* Audio extension */
	{"USB_DT_HUB_NONVAR_SIZE",7},
	{0,0},
};

usb_define usb_endpoints_choices[] = {
	{"USB_ENDPOINT_ADDRESS_MASK",0x0f},/* in bEndpointAddress */
	{"USB_ENDPOINT_DIR_MASK",0x80},
	{"USB_ENDPOINT_TYPE_MASK",0x03},/* in bmAttributes */
	{"USB_ENDPOINT_TYPE_CONTROL",0},
	{"USB_ENDPOINT_TYPE_ISOCHRONOUS",1},
	{"USB_ENDPOINT_TYPE_BULK",2},
	{"USB_ENDPOINT_TYPE_INTERRUPT",3},
	{0,0},
};

usb_define usb_requests_choice[] = {
	{"USB_REQ_GET_STATUS",0x00},
	{"USB_REQ_CLEAR_FEATURE",0x01},
	{"USB_REQ_SET_FEATURE",0x03},
	{"USB_REQ_SET_ADDRESS",0x05},
	{"USB_REQ_GET_DESCRIPTOR",0x06},
	{"USB_REQ_SET_DESCRIPTOR",0x07},
	{"USB_REQ_GET_CONFIGURATION",0x08},
	{"USB_REQ_SET_CONFIGURATION",0x09},
	{"USB_REQ_GET_INTERFACE",0x0A},
	{"USB_REQ_SET_INTERFACE",0x0B},
	{"USB_REQ_SYNCH_FRAME",0x0C},
	{0,0},
};

usb_define usb_type_choice[] = {
	{"USB_TYPE_STANDARD",(0x00 << 5)},
	{"USB_TYPE_CLASS",(0x01 << 5)},
	{"USB_TYPE_VENDOR",(0x02 << 5)},
	{"USB_TYPE_RESERVED",(0x03 << 5)},
	{0,0},
};

usb_define usb_recipient_choice[] = {
	{"USB_RECIP_DEVICE",0x00},
	{"USB_RECIP_INTERFACE",0x01},
	{"USB_RECIP_ENDPOINT",0x02},
	{"USB_RECIP_OTHER",0x03},
	{0,0},
};

usb_define usb_misc[] = {
	{"USB_MAXENDPOINTS",32},
	{"USB_MAXINTERFACES",32},
	{"USB_MAXALTSETTING",128},
	{"USB_MAXCONFIG",8},
	{"USB_ENDPOINT_IN",0x80},
	{"USB_ENDPOINT_OUT",0x00},
	{"USB_ERROR_BEGIN",500000},
	{0,0},
};

usb_define* usb_all_defines[] = {
	usb_class_choice,
	usb_descriptor_types_choices,
	usb_descriptor_types_sizes,
	usb_endpoints_choices,
	usb_requests_choice,
	usb_type_choice,
	usb_recipient_choice,
	usb_misc,
};

#define COMMA ,

//14
#define USB_DEVICE_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(bcdUSB)SEP\
	MANGLE(bDeviceClass)SEP\
	MANGLE(bDeviceSubClass)SEP\
	MANGLE(bDeviceProtocol)SEP\
	MANGLE(bMaxPacketSize0)SEP\
	MANGLE(idVendor)SEP\
	MANGLE(idProduct)SEP\
	MANGLE(bcdDevice)SEP\
	MANGLE(iManufacturer)SEP\
	MANGLE(iProduct)SEP\
	MANGLE(iSerialNumber)SEP\
	MANGLE(bNumConfigurations)

//8
#define USB_ENDPOINT_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(bEndpointAddress)SEP\
	MANGLE(bmAttributes)SEP\
	MANGLE(wMaxPacketSize)SEP\
	MANGLE(bInterval)SEP\
	MANGLE(bRefresh)SEP\
	MANGLE(bSynchAddress)
//	  MANGLE(extras)

//9
#define USB_INTERFACE_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(bInterfaceNumber)SEP\
	MANGLE(bAlternateSetting)SEP\
	MANGLE(bNumEndpoints)SEP\
	MANGLE(bInterfaceClass)SEP\
	MANGLE(bInterfaceSubClass)SEP\
	MANGLE(bInterfaceProtocol)SEP\
	MANGLE(iInterface)
//        MANGLE(endpoint)
//        MANGLE(extras)

//8
#define USB_CONFIG_DESCRIPTOR(MANGLE,SEP) \
	MANGLE(bLength)SEP\
	MANGLE(bDescriptorType)SEP\
	MANGLE(wTotalLength)SEP\
	MANGLE(bNumInterfaces)SEP\
	MANGLE(bConfigurationValue)SEP\
	MANGLE(iConfiguration)SEP\
	MANGLE(bmAttributes)SEP\
	MANGLE(MaxPower)
//	  MANGLE(interface)
//	  MANGLE(extras)

static Ruby usb_get_config (struct usb_config_descriptor *dev) {
#define MANGLE(X) INT2NUM(dev->X)
	return rb_funcall(rb_const_get(cUSB,SI(Config)),SI(new),8,
		USB_CONFIG_DESCRIPTOR(MANGLE,COMMA));
#undef MANGLE
}

static Ruby usb_scan_bus (usb_bus *bus) {
	Ruby rbus = rb_ary_new();
	for (struct usb_device *dev=bus->devices; dev; dev=dev->next) {
		struct usb_device_descriptor *devd = &dev->descriptor;
#define MANGLE(X) INT2NUM(devd->X)
		rb_ary_push(rbus, rb_funcall(rb_const_get(cUSB,SI(Device)),SI(new),17,
			USB_DEVICE_DESCRIPTOR(MANGLE,COMMA),
			rb_str_new2(dev->filename),
			PTR2FIX(dev),
			usb_get_config(dev->config)));
#undef MANGLE
	}
	return rbus;
}

void startup_usb () {
	cUSB = rb_define_class_under(mGridFlow, "USB", rb_cObject);
	IEVAL(cUSB, "def self.new(dev) @dev=dev; @ptr=dev.ptr; end");
#define MANGLE(X) SYM(X)
	rb_const_set(cUSB, SI(Device), rb_funcall(EVAL("Struct"),SI(new),17,
		USB_DEVICE_DESCRIPTOR(MANGLE,COMMA), SYM(filename), SYM(ptr), SYM(config)));
	rb_const_set(cUSB, SI(Endpoint), rb_funcall(EVAL("Struct"),SI(new),8,
		USB_ENDPOINT_DESCRIPTOR(MANGLE,COMMA)));
	rb_const_set(cUSB, SI(Interface), rb_funcall(EVAL("Struct"),SI(new),10,
		USB_INTERFACE_DESCRIPTOR(MANGLE,COMMA)));
	rb_const_set(cUSB, SI(Config), rb_funcall(EVAL("Struct"),SI(new),8,
		USB_CONFIG_DESCRIPTOR(MANGLE,COMMA)));
#undef MANGLE
	for(int i=0; i<COUNT(usb_all_defines); i++) {
		usb_define *ud = usb_all_defines[i];
		for(; ud->name; ud++) {
			rb_const_set(cUSB, rb_intern(ud->name), INT2NUM(ud->i));
		}
	}
	usb_init();
	usb_find_busses();
	usb_find_devices();
	Ruby busses = rb_hash_new();
	rb_ivar_set(cUSB, SI(@busses), busses);
	for (usb_bus *bus=usb_get_busses(); bus; bus=bus->next) {
		rb_hash_aset(busses,rb_str_new2(bus->dirname),usb_scan_bus(bus));
	}
	IEVAL(cUSB,"STDERR.print '@busses = '; STDERR.puts @busses.inspect");
}

/*
struct usb_descriptor_header
struct usb_string_descriptor
struct usb_hid_descriptor
struct usb_endpoint_descriptor
struct usb_interface_descriptor
struct usb_interface
struct usb_config_descriptor
struct usb_device_descriptor
struct usb_device
struct usb_bus;
struct usb_dev_handle;
typedef struct usb_dev_handle usb_dev_handle;
extern struct usb_bus *usb_busses;
*/
/*
usb_dev_handle *usb_open(struct usb_device *dev);
int usb_close(usb_dev_handle *dev);
int usb_bulk_write(usb_dev_handle *dev, int ep, char *bytes, int size,
        int timeout);
int usb_bulk_read(usb_dev_handle *dev, int ep, char *bytes, int size,
        int timeout);
int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
        int value, int index, char *bytes, int size, int timeout);
int usb_set_configuration(usb_dev_handle *dev, int configuration);
int usb_claim_interface(usb_dev_handle *dev, int interface);
int usb_release_interface(usb_dev_handle *dev, int interface);
int usb_set_altinterface(usb_dev_handle *dev, int alternate);
int usb_resetep(usb_dev_handle *dev, unsigned int ep);
int usb_clear_halt(usb_dev_handle *dev, unsigned int ep);
int usb_reset(usb_dev_handle *dev);
int usb_get_string(usb_dev_handle *dev, int index, int langid, char *buf,
        size_t buflen);
int usb_get_string_simple(usb_dev_handle *dev, int index, char *buf,
        size_t buflen);
char *usb_strerror(void);
void usb_init(void);
void usb_set_debug(int level);
int usb_find_busses(void);
int usb_find_devices(void);
struct usb_device *usb_device(usb_dev_handle *dev);
struct usb_bus *usb_get_busses(void);
*/
