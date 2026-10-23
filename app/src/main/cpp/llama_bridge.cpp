#include <jni.h>
#include <string>
#include <android/log.h>

#define LOG_TAG "LocalMindJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_localmind_app_LlmEngine_getHelloFromJni(JNIEnv* env, jobject /* this */) {
    std::string hello = "مرحباً من C++! مكتبة الذكاء الاصطناعي جاهزة.";
    LOGI("JNI Called successfully");
    return env->NewStringUTF(hello.c_str());
}
