#include <jni.h>
#include <string>
#include <android/log.h>
#include "llama.h"

#define LOG_TAG "LocalMindJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_localmind_app_LlmEngine_getHelloFromJni(JNIEnv* env, jobject /* this */) {
    std::string sys_info = llama_print_system_info();
    std::string hello = "مرحباً! تم ربط llama.cpp بنجاح.\nمعلومات النظام:\n" + sys_info;
    LOGI("JNI Called. Llama system info: %s", sys_info.c_str());
    return env->NewStringUTF(hello.c_str());
}
