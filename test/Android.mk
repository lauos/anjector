LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)	

ifeq ($(ANJECTOR_DEBUG), true)
	EXTRA_CFLAGS := -DANJECTOR_DEBUG -DSTDOUT_LOG=1
endif

ifeq ($(TARGET_ARCH), arm)
	ARCH_PATH := $(ROOT_PATH)/src/arch/arm
else ifeq ($(TARGET_ARCH), arm64)
	ARCH_PATH := $(ROOT_PATH)/src/arch/arm64
else ifeq ($(TARGET_ARCH), x86)
	ARCH_PATH := $(ROOT_PATH)/src/arch/x86
else ifeq ($(TARGET_ARCH), x86_64)
	ARCH_PATH := $(ROOT_PATH)/src/arch/x64
endif

LOCAL_MODULE := anjector_test

LOCAL_C_INCLUDES += $(ARCH_PATH) \
					$(ROOT_PATH)/include

LOCAL_SRC_FILES += 	$(ROOT_PATH)/src/anjector.c     \
					$(ROOT_PATH)/src/opt_utils.c    \
					$(ROOT_PATH)/src/proc_utils.c   \
					$(ROOT_PATH)/src/ptrace_utils.c \
					$(ARCH_PATH)/ptrace_arch.c

LOCAL_SRC_FILES += $(LOCAL_PATH)/anjector_test.cpp

LOCAL_CFLAGS += $(EXTRA_CFLAGS)
LOCAL_LDFLAGS += -ldl

LOCAL_STATIC_LIBRARIES := googletest_main
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)	

ifeq ($(ANJECTOR_DEBUG), true)
	EXTRA_CFLAGS := -DANJECTOR_DEBUG -DSTDOUT_LOG=1
endif

ifeq ($(TARGET_ARCH), arm)
	ARCH_PATH := $(ROOT_PATH)/src/arch/arm
else ifeq ($(TARGET_ARCH), arm64)
	ARCH_PATH := $(ROOT_PATH)/src/arch/arm64
else ifeq ($(TARGET_ARCH), x86)
	ARCH_PATH := $(ROOT_PATH)/src/arch/x86
else ifeq ($(TARGET_ARCH), x86_64)
	ARCH_PATH := $(ROOT_PATH)/src/arch/x64
endif

LOCAL_MODULE := tracer

LOCAL_C_INCLUDES += $(ARCH_PATH) \
					$(ROOT_PATH)/include

LOCAL_SRC_FILES += 	$(ROOT_PATH)/src/anjector.c     \
					$(ROOT_PATH)/src/opt_utils.c    \
					$(ROOT_PATH)/src/proc_utils.c   \
					$(ROOT_PATH)/src/ptrace_utils.c \
					$(ARCH_PATH)/ptrace_arch.c

LOCAL_SRC_FILES += $(LOCAL_PATH)/tracer.c

LOCAL_CFLAGS += $(EXTRA_CFLAGS)
LOCAL_LDFLAGS += -ldl

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := tracee
LOCAL_SRC_FILES := tracee.c

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := tracee_sleep
LOCAL_SRC_FILES := tracee_sleep.c

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := tracee_select
LOCAL_SRC_FILES := tracee_select.c

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := tracee_select_timeout
LOCAL_SRC_FILES := tracee_select.c
LOCAL_CFLAGS += -DTIME_OUT

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := tracee_epoll
LOCAL_SRC_FILES := tracee_epoll.c

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := tracee_epoll_timeout
LOCAL_SRC_FILES := tracee_epoll.c
LOCAL_CFLAGS += -DTIME_OUT

include $(BUILD_EXECUTABLE)

$(call import-module,third_party/googletest)
