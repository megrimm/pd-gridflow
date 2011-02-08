LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION = .fcs
LOCAL_MODULE := gridflow
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../src
LOCAL_CFLAGS := -g -xc++ -DPD -DANDROID -Wall -Wno-unused -Wunused-variable -Wno-trigraphs -DPDSUF=\".so\" -DPASS1 -DPASS2 -DPASS3 -DPASS4
LOCAL_SHARED_LIBRARIES := pdnative
LOCAL_SRC_FILES := src/gridflow.cxx.fcs \
	src/grid.cxx.fcs \
	src/classes1.cxx.fcs \
	src/classes2.cxx.fcs \
	src/classes3.cxx.fcs \
	src/classes_gui.cxx.fcs \
	src/expr.cxx.fcs \
        src/numop1.cxx.fcs \
        src/numop2.cxx.fcs \
	src/formats.cxx.fcs

#	src/android.cxx.fcs

LOCAL_LDLIBS := -lstdc++




include $(BUILD_SHARED_LIBRARY)


# PDSUF is going to be troublesome later in gridflow.cxx

