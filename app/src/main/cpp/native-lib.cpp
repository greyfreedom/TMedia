#include <jni.h>
#include <string>
#include <logger.h>
#include "FFmpegEngine.h"

#ifdef __cplusplus
extern "C" {
#endif

FFmpegEngine *ffmpeg;

void logInfo(JNIEnv *env, jobject) {
    if (ffmpeg) {
        ffmpeg->printCodecInfo();
    }
}

void setUpNative(JNIEnv *env, jobject) {
    if (!ffmpeg) {
        ffmpeg = new FFmpegEngine();
    }
}

void releaseNative(JNIEnv *env, jobject) {
    delete ffmpeg;
    ffmpeg = nullptr;
}

void decodeVideoToYUV(JNIEnv *env, jobject, jstring input, jstring output) {
    if (ffmpeg) {
        const char *inputPath = env->GetStringUTFChars(input, nullptr);
        const char *outputPath = env->GetStringUTFChars(output, nullptr);
        ffmpeg->decodeVideoToYUV(inputPath, outputPath);
    }
}

static const JNINativeMethod gMethods[] = {
        {"logInfo", "()V", (void *) logInfo},
        {"setUpNative", "()V", (void *) setUpNative},
        {"releaseNative", "()V", (void *) releaseNative},
        {"decodeVideoToYUV", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) decodeVideoToYUV}
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

#ifdef __cplusplus
};
#endif