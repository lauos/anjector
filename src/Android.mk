LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH), arm)
	ARCH_PATH := $(LOCAL_PATH)/arch/arm
else ifeq ($(TARGET_ARCH), arm64)
	ARCH_PATH := $(LOCAL_PATH)/arch/arm64
else ifeq ($(TARGET_ARCH), x86)
	ARCH_PATH := $(LOCAL_PATH)/arch/x86
else ifeq ($(TARGET_ARCH), x86_64)
	ARCH_PATH := $(LOCAL_PATH)/arch/x64
endif

LOCAL_MODULE := anjector_static

LOCAL_C_INCLUDES += $(ARCH_PATH) \
					$(ROOT_PATH)/include

LOCAL_EXPORT_C_INCLUDES += $(ROOT_PATH)/include

LOCAL_SRC_FILES += 	$(LOCAL_PATH)/anjector.c     \
					$(LOCAL_PATH)/opt_utils.c    \
					$(LOCAL_PATH)/proc_utils.c   \
					$(LOCAL_PATH)/ptrace_utils.c \
					$(ARCH_PATH)/ptrace_arch.c

LOCAL_CFLAGS += $(EXTRA_CFLAGS)

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := anjector
LOCAL_C_INCLUDES += $(ROOT_PATH)/include
LOCAL_SRC_FILES += 	$(LOCAL_PATH)/main.c 
LOCAL_CFLAGS += $(EXTRA_CFLAGS)
LOCAL_LDFLAGS += -ldl $(EXTRA_LDFLAGS)
LOCAL_STATIC_LIBRARIES := anjector_static
include $(BUILD_EXECUTABLE)
