LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ROOT_PATH := $(LOCAL_PATH)
ifeq ($(ANJECTOR_DEBUG), true)
	EXTRA_CFLAGS := -DANJECTOR_DEBUG -DANDROID_LOG=1
	EXTRA_LDFLAGS := -llog
endif

ifeq ($(ANJECTOR_TEST), true)
#For test purpose
include $(ROOT_PATH)/test/Android.mk 
else
include $(ROOT_PATH)/src/Android.mk 
endif

include $(ROOT_PATH)/bridge/Android.mk 
