#include <jni.h>
#include <string>
#include <logger.h>
#include "FFmpegEngine.h"
#include <android/native_window_jni.h>

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

void playYuv(JNIEnv *env, jobject, jstring input, jobject surface) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        LOGI("jni create window from surface failed.");
        return;
    }
    if (ffmpeg) {
        const char *filePath = env->GetStringUTFChars(input, nullptr);
        ffmpeg->playYUV(filePath, window);
    }
}

void decodeAudioToPCM(JNIEnv *env, jobject, jstring input, jstring output) {
    if (ffmpeg) {
        const char *inputPath = env->GetStringUTFChars(input, nullptr);
        const char *outputPath = env->GetStringUTFChars(output, nullptr);
        ffmpeg->decodeAudioToPCM(inputPath, outputPath);
    }
}

void playAudio(JNIEnv *env, jobject, jstring input) {
    if (ffmpeg) {
        const char *inputPath = env->GetStringUTFChars(input, nullptr);
        ffmpeg->playAudio(inputPath);
    }
}

void pauseAudio(JNIEnv *env, jobject) {
    if (ffmpeg) {
        ffmpeg->pauseAudio();
    }
}

void stopAudio(JNIEnv *env, jobject) {
    if (ffmpeg) {
        ffmpeg->stopAudio();
    }
}

static const JNINativeMethod gMethods[] = {
        {"logInfo",          "()V",                                         (void *) logInfo},
        {"setUpNative",      "()V",                                         (void *) setUpNative},
        {"releaseNative",    "()V",                                         (void *) releaseNative},
        {"decodeVideoToYUV", "(Ljava/lang/String;Ljava/lang/String;)V",     (void *) decodeVideoToYUV},
        {"playYuv",          "(Ljava/lang/String;Landroid/view/Surface;)V", (void *) playYuv},
        {"decodeAudioToPCM", "(Ljava/lang/String;Ljava/lang/String;)V",     (void *) decodeAudioToPCM},
        {"playAudio",        "(Ljava/lang/String;)V",                       (void *) playAudio},
        {"pauseAudio",       "()V",                                         (void *) pauseAudio},
        {"stopAudio",        "()V",                                         (void *) stopAudio},
};

#define kClassName "com/demo/tmediademo/FFmpeg$Companion"

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