#include <jni.h>
#include <string>
#include <android/log.h>
#include "llama.h"

#define LOG_TAG "LocalMindJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static llama_model* g_model = nullptr;

extern "C" JNIEXPORT jstring JNICALL
Java_com_localmind_app_LlmEngine_getHelloFromJni(JNIEnv* env, jobject) {
    std::string sys_info = llama_print_system_info();
    return env->NewStringUTF(sys_info.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_localmind_app_LlmEngine_loadModel(JNIEnv* env, jobject, jstring model_path) {
    if (g_model != nullptr) {
        llama_free_model(g_model);
        g_model = nullptr;
    }
    
    const char* path = env->GetStringUTFChars(model_path, 0);
    LOGI("Loading model from: %s", path);
    
    llama_backend_init();
    llama_model_params params = llama_model_default_params();
    params.n_gpu_layers = 0; // CPU only for now
    
    g_model = llama_load_model_from_file(path, params);
    env->ReleaseStringUTFChars(model_path, path);
    
    if (g_model) {
        LOGI("Model loaded successfully!");
        return JNI_TRUE;
    } else {
        LOGI("Failed to load model!");
        return JNI_FALSE;
    }
}
