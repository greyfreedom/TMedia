#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
#include "libavformat/avformat.h"
}

#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "native", format, ##__VA_ARGS__)
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)

extern "C" JNIEXPORT void JNICALL
logInfo() {
    char info[100] = {0};
    void *opaque = nullptr;
    const AVCodec *codec = nullptr;
    LOGI("FFmpeg version : %s\n", av_version_info());
    while ((codec = av_codec_iterate(&opaque)) != nullptr) {
        if (codec->decode != nullptr) {
            sprintf(info, "[Dec]");
        } else {
            sprintf(info, "[Enc]");
        }
        switch (codec->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, codec->name);
        LOGI("%s\n", info);
        memset(info, 0, sizeof(info));
    }
}

static const JNINativeMethod gMethods[] = {
        "logInfo", "()V", (void *) logInfo
};

#define kClassName "com/demo/tmediademo/FFmpegFuncActivity"

JNIEXPORT int
JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGI("JNI_OnLoad...");
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    jclass clazz = env->FindClass(kClassName);
    if (!clazz) {
        LOGI("JNI_OnLoad can not find class %s\n", kClassName);
        return -1;
    }
    if (env->RegisterNatives(clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
        LOGI("JNI_OnLoad RegisterNatives failed\n");
        return -1;
    }
    LOGI("JNI_OnLoad success\n");
    return JNI_VERSION_1_4;
}
