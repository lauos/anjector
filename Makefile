TOOLCHAIN := 4.9
PLATFORM := android-24
ABI := armeabi-v7a
PIE := true

all: build

build:
	ndk-build NDK_PROJECT_PATH=$(CURDIR) APP_BUILD_SCRIPT=$(CURDIR)/Android.mk APP_PLATFORM=$(PLATFORM) APP_ABI=$(ABI) APP_PIE=$(PIE) NDK_TOOLCHAIN_VERSION=$(TOOLCHAIN)

build_all:
	ndk-build NDK_PROJECT_PATH=$(CURDIR) APP_BUILD_SCRIPT=$(CURDIR)/Android.mk APP_PLATFORM=$(PLATFORM) APP_ABI=armeabi\ armeabi-v7a\ arm64-v8a\ x86\ x86_64 APP_PIE=$(PIE) NDK_TOOLCHAIN_VERSION=$(TOOLCHAIN)

build_debug:
	ndk-build NDK_DEBUG=1 NDK_LOG=1 V=1 NDK_PROJECT_PATH=$(CURDIR) APP_BUILD_SCRIPT=$(CURDIR)/Android.mk APP_PLATFORM=$(PLATFORM) APP_ABI=$(ABI) APP_PIE=$(PIE) NDK_TOOLCHAIN_VERSION=$(TOOLCHAIN) ANJECTOR_DEBUG=true

push: build
	adb push $(CURDIR)/libs/$(ABI)/anjector /data/local/tmp/
	adb push $(CURDIR)/libs/$(ABI)/libbridge.so /data/local/tmp/

push_debug: build_debug
	adb push $(CURDIR)/libs/$(ABI)/gdbserver /data/local/tmp/
	adb push $(CURDIR)/obj/local/$(ABI)/anjector /data/local/tmp/
	adb push $(CURDIR)/obj/local/$(ABI)/libbridge.so /data/local/tmp/

test:
	ndk-build NDK_DEBUG=1 NDK_PROJECT_PATH=$(CURDIR) APP_BUILD_SCRIPT=$(CURDIR)/Android.mk APP_PLATFORM=$(PLATFORM) APP_ABI=$(ABI) APP_PIE=$(PIE) APP_STL=gnustl_static NDK_TOOLCHAIN_VERSION=$(TOOLCHAIN) ANJECTOR_DEBUG=true ANJECTOR_TEST=true 
	adb push $(CURDIR)/libs/$(ABI)/anjector_test /data/local/tmp
	adb push $(CURDIR)/libs/$(ABI)/libbridge.so /data/local/tmp/
	adb push $(CURDIR)/libs/$(ABI)/tracer /data/local/tmp
	adb push $(CURDIR)/libs/$(ABI)/tracee /data/local/tmp
	adb push $(CURDIR)/libs/$(ABI)/tracee_sleep /data/local/tmp
	adb push $(CURDIR)/libs/$(ABI)/tracee_select /data/local/tmp
	adb push $(CURDIR)/libs/$(ABI)/tracee_epoll /data/local/tmp
	adb push $(CURDIR)/libs/$(ABI)/tracee_select_timeout /data/local/tmp
	adb push $(CURDIR)/libs/$(ABI)/tracee_epoll_timeout /data/local/tmp
	adb shell /data/local/tmp/anjector_test 
	
clean:
	rm -rf libs obj

.PHONY: all build build_all build_debug push push_debug test clean
