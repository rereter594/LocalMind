#include <jni.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
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
        llama_model_free(g_model);
        if (g_ctx) llama_free(g_ctx);
        g_model = nullptr;
        g_ctx = nullptr;
    }
    const char* path = env->GetStringUTFChars(model_path, 0);
    llama_backend_init();
    llama_model_params params = llama_model_default_params();
    params.n_gpu_layers = 0;
    
    g_model = llama_model_load_from_file(path, params);
    env->ReleaseStringUTFChars(model_path, path);
    if (!g_model) return JNI_FALSE;

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = 6;
    g_ctx = llama_new_context_with_model(g_model, ctx_params);
    return g_ctx != nullptr ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_localmind_app_LlmEngine_generate(JNIEnv* env, jobject, jstring full_prompt) {
    if (!g_model || !g_ctx) return env->NewStringUTF("الموديل غير محمل.");

    const char* prompt_str = env->GetStringUTFChars(full_prompt, 0);
    std::string formatted_prompt = std::string(prompt_str);
    env->ReleaseStringUTFChars(full_prompt, prompt_str);

    // تصفير الذاكرة المؤقتة قبل كل محادثة جديدة
    llama_kv_cache_clear(g_ctx);

    std::vector<llama_token> tokens_list;
    tokens_list.resize(formatted_prompt.length() + 2);
    int n_tokens = llama_tokenize(g_model, formatted_prompt.c_str(), formatted_prompt.length(), tokens_list.data(), tokens_list.size(), true, true);
    tokens_list.resize(n_tokens);

    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens_list[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = false;
    }
    batch.logits[n_tokens - 1] = true;
    batch.n_tokens = n_tokens;

    if (llama_decode(g_ctx, batch) != 0) {
        llama_batch_free(batch);
        return env->NewStringUTF("خطأ في معالجة النص.");
    }
    llama_batch_free(batch);

    std::string response = "";
    const int n_predict = 256;
    int n_cur = n_tokens;
    llama_batch single_batch = llama_batch_init(1, 0, 1);

    for (int i = 0; i < n_predict; i++) {
        auto* logits = llama_get_logits_ith(g_ctx, -1);
        int n_vocab = llama_n_vocab(g_model);
        
        // خوارزمية Top-K Sampling لجعل الردود طبيعية
        std::vector<llama_token_data> candidates;
        candidates.reserve(n_vocab);
        for (int j = 0; j < n_vocab; j++) {
            candidates.emplace_back(llama_token_data{j, logits[j], 0.0f});
        }
        
        // ترتيب الكلمات حسب الاحتمالية
        std::sort(candidates.begin(), candidates.end(), [](const llama_token_data & a, const llama_token_data & b) {
            return a.logit > b.logit;
        });
        
        // اختيار من أفضل 40 كلمة فقط
        int top_k = 40;
        if (n_vocab < top_k) top_k = n_vocab;
        
        // تطبيق Temperature = 0.7
        float sum = 0;
        float max_logit = candidates[0].logit;
        for (int j = 0; j < top_k; j++) {
            candidates[j].logit = expf((candidates[j].logit - max_logit) / 0.7f);
            sum += candidates[j].logit;
        }
        
        // سحب عشوائي بناءً على الاحتمالات
        float r = (float)rand() / (float)RAND_MAX * sum;
        int selected_id = candidates[0].id;
        for (int j = 0; j < top_k; j++) {
            r -= candidates[j].logit;
            if (r <= 0) {
                selected_id = candidates[j].id;
                break;
            }
        }

        if (selected_id == llama_token_eos(g_model) || selected_id == 151645) break;

        char buf[128];
        int length = llama_token_to_piece(g_model, selected_id, buf, sizeof(buf), 0, true);
        if (length > 0) response += std::string(buf, length);

        single_batch.token[0] = selected_id;
        single_batch.pos[0] = n_cur++;
        single_batch.n_seq_id[0] = 1;
        single_batch.seq_id[0][0] = 0;
        single_batch.logits[0] = true;
        single_batch.n_tokens = 1;
        llama_decode(g_ctx, single_batch);
    }

    llama_batch_free(single_batch);
    return env->NewStringUTF(response.c_str());
}
