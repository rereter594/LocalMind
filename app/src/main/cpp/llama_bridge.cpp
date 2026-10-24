#include <jni.h>
#include <string>
#include <vector>
#include <android/log.h>
#include "llama.h"

#define LOG_TAG "LocalMindJNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static llama_model* g_model = nullptr;
static llama_context* g_ctx = nullptr;

extern "C" JNIEXPORT jstring JNICALL
Java_com_localmind_app_LlmEngine_getHelloFromJni(JNIEnv* env, jobject) {
    std::string sys_info = llama_print_system_info();
    return env->NewStringUTF(sys_info.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_localmind_app_LlmEngine_loadModel(JNIEnv* env, jobject, jstring model_path) {
    if (g_model) {
        llama_free_model(g_model);
        if (g_ctx) llama_free(g_ctx);
        g_model = nullptr;
        g_ctx = nullptr;
    }

    const char* path = env->GetStringUTFChars(model_path, 0);
    llama_backend_init();
    llama_model_params params = llama_model_default_params();
    params.n_gpu_layers = 0;
    
    g_model = llama_load_model_from_file(path, params);
    env->ReleaseStringUTFChars(model_path, path);

    if (!g_model) return JNI_FALSE;

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = 6;
    g_ctx = llama_new_context_with_model(g_model, ctx_params);

    return g_ctx != nullptr ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_localmind_app_LlmEngine_generate(JNIEnv* env, jobject, jstring prompt) {
    if (!g_model || !g_ctx) return env->NewStringUTF("الموديل غير محمل.");

    const char* prompt_str = env->GetStringUTFChars(prompt, 0);
    
    // تنسيق رسالة Qwen (ChatML format)
    std::string formatted_prompt = "<|im_start|>user\n" + std::string(prompt_str) + "<|im_end|>\n<|im_start|>assistant\n";
    env->ReleaseStringUTFChars(prompt, prompt_str);

    // تحويل النص إلى توكنز
    std::vector<llama_token> tokens_list;
    tokens_list.resize(formatted_prompt.length() + 2);
    int n_tokens = llama_tokenize(g_model, formatted_prompt.c_str(), formatted_prompt.length(), tokens_list.data(), tokens_list.size(), true, true);
    tokens_list.resize(n_tokens);

    // إدخال النص للموديل
    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    for (int i = 0; i < n_tokens; i++) {
        llama_batch_add(batch, tokens_list[i], i, {0}, false);
    }
    llama_decode(g_ctx, batch);
    llama_batch_free(batch);

    // توليد الرد
    std::string response = "";
    const int n_predict = 256;
    int n_cur = n_tokens;

    llama_batch single_batch = llama_batch_init(1, 0, 1);
    
    for (int i = 0; i < n_predict; i++) {
        // اختيار التوكن التالي
        auto* logits = llama_get_logits_ith(g_ctx, -1);
        int n_vocab = llama_n_vocab(g_model);
        
        // اختيار بسيط (Greedy Sampling)
        int max_index = 0;
        float max_logit = logits[0];
        for (int j = 1; j < n_vocab; j++) {
            if (logits[j] > max_logit) {
                max_logit = logits[j];
                max_index = j;
            }
        }

        if (max_index == llama_token_eos(g_model) || max_index == 151645) break; // نهاية النص

        char buf[128];
        int length = llama_token_to_piece(g_model, max_index, buf, sizeof(buf), 0, true);
        if (length > 0) {
            response += std::string(buf, length);
        }

        llama_batch_add(single_batch, max_index, n_cur++, {0}, true);
        llama_decode(g_ctx, single_batch);
        single_batch.n_tokens = 0;
    }

    llama_batch_free(single_batch);
    return env->NewStringUTF(response.c_str());
}
