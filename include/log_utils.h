#ifndef _LOG_UTILS_H_
#define _LOG_UTILS_H_


#ifdef ANJECTOR_DEBUG

#if ANDROID_LOG

#include <android/log.h>
    #define TAG "ANJECTOR"
    #define LOGV(format, args...) \
        __android_log_print(ANDROID_LOG_VERBOSE, TAG, format, ##args)
    #define LOGD(format, args...) \
        __android_log_print(ANDROID_LOG_DEBUG, TAG, format, ##args)
    #define LOGI(format, args...) \
        __android_log_print(ANDROID_LOG_INFO, TAG, format, ##args)
    #define LOGW(format, args...) \
        __android_log_print(ANDROID_LOG_WARN, TAG, format, ##args)
    #define LOGE(format, args...) \
        __android_log_print(ANDROID_LOG_ERROR,TAG, format, ##args)

#elif STDOUT_LOG

#include <stdio.h>
    #define LOGV(format, args...) printf(format, ##args)
    #define LOGD(format, args...) printf(format, ##args)
    #define LOGI(format, args...) printf(format, ##args)
    #define LOGW(format, args...) printf(format, ##args)
    #define LOGE(format, args...) printf(format, ##args)

#else

    #define LOGV(format, args...)
    #define LOGD(format, args...)
    #define LOGI(format, args...)
    #define LOGW(format, args...)
    #define LOGE(format, args...)

#endif // How to log

#else

    #define LOGV(format, args...)
    #define LOGD(format, args...)
    #define LOGI(format, args...)
    #define LOGW(format, args...)
    #define LOGE(format, args...)

#endif

#endif    //_LOG_UTILS__H_
