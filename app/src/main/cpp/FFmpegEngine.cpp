//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#include "FFmpegEngine.h"
#include "VideoDecoder.h"
#include <thread>
#include <unistd.h>
#include "AudioDecoder.h"
#include "AudioPlayer.h"

using namespace std;

FFmpegEngine::FFmpegEngine() : audioPlayer(nullptr) {
}

FFmpegEngine::~FFmpegEngine() {
}

void FFmpegEngine::printCodecInfo() {
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

void FFmpegEngine::decodeVideoToYUV(const char *input, const char *output) {
    thread t([input, output] {
        FILE *outFile = fopen(output, "wb+");
        if (!outFile) {
            LOGE("decoder decode fopen %s failed.", output);
            return;
        }
        shared_ptr<VideoDecoder> decoder = std::make_shared<VideoDecoder>();
        if (decoder->prepare(input, AV_PIX_FMT_YUV420P) < 0) {
            decoder->releaseDecoder();
            return;
        }
        int width = decoder->getWidth();
        int height = decoder->getHeight();
        LOGI("play yuv width = %d, height = %d", width, height);
        if (width <= 0 || height <= 0) {
            return;
        }
        AVFrame *frame = nullptr;
        int ret;
        do {
            ret = decoder->decodeFrame(&frame);
            if (frame) {
                fwrite(frame->data[0], 1, width * height, outFile);
                fwrite(frame->data[1], 1, width * height / 4, outFile);
                fwrite(frame->data[2], 1, width * height / 4, outFile);
                LOGI("write a frame to file, w = %d, h = %d", width, height);
            } else {
                LOGI("frame is null");
            }
            frame = nullptr;
        } while (ret >= 0);
        fclose(outFile);
        decoder->releaseDecoder();
        LOGI("decodeVideoToYUV finished. path = %s", output);
    });
    t.detach();
}

void FFmpegEngine::playYUV(const char *input, ANativeWindow *window) {
    LOGI("play yuv path = %s", input);
    if (!window) {
        return;
    }
    thread t([input, window] {
        shared_ptr<VideoDecoder> decoder = make_shared<VideoDecoder>();
        if (decoder->prepare(input, AV_PIX_FMT_RGBA) < 0) {
            decoder->releaseDecoder();
            return;
        }
        int width = decoder->getWidth();
        int height = decoder->getHeight();
        LOGI("play yuv width = %d, height = %d", width, height);
        if (width <= 0 || height <= 0) {
            return;
        }
        int frameRate = decoder->getFrameRate();
        if (frameRate <= 0) {
            frameRate = 25;
        }
        int interval = static_cast<int>(1000.0 / frameRate);
        LOGI("frame rate = %d, interval = %d", frameRate, interval);
        ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBX_8888);
        ANativeWindow_Buffer windowBuffer;
        AVFrame *frame = nullptr;
        int ret;
        do {
            ret = decoder->decodeFrame(&frame);
            if (frame) {
                ANativeWindow_lock(window, &windowBuffer, 0);
                uint8_t *dst = static_cast<uint8_t *>(windowBuffer.bits);
                uint8_t *src = frame->data[0];
                memcpy(dst, src, width * height * 4);
                ANativeWindow_unlockAndPost(window);
                usleep(interval * 1000);
            } else {
                LOGI("frame is null");
            }
            frame = nullptr;
        } while (ret >= 0);
        decoder->releaseDecoder();
    });
    t.detach();
}

void FFmpegEngine::decodeAudioToPCM(const char *input, const char *output) {
    thread t([input, output] {
        FILE *out = fopen(output, "wb+");
        if (!out) {
            LOGE("decodeAudioToPCM fopen %s failed.", output);
            return;
        }
        shared_ptr<AudioDecoder> decoder = make_shared<AudioDecoder>();
        if (decoder->prepare(input, AV_SAMPLE_FMT_S16) < 0) {
            decoder->releaseDecoder();
            return;
        }
        while (decoder->decodeFrame([out](AVFrame *frame) mutable {
            LOGI("decode a audio frame, linesize = %d", frame->linesize[0]);
            int size = frame->linesize[0] * frame->channels;
            fwrite(frame->data[0], 1, size, out);
        }) >= 0);
        decoder->releaseDecoder();
        LOGI("decode audio to pcm finish.");
    });
    t.detach();
}

void FFmpegEngine::playAudio(const char *input) {
    if (!audioPlayer) {
        audioPlayer = std::make_shared<AudioPlayer>();
        if (audioPlayer->prepare(input) >= 0) {
            audioPlayer->play();
        } else {
            audioPlayer->stop();
            audioPlayer = nullptr;
        }
    } else {
        audioPlayer->play();
    }
}

void FFmpegEngine::pauseAudio() {
    if (audioPlayer) {
        audioPlayer->pause();
    }
}

void FFmpegEngine::stopAudio() {
    if (audioPlayer) {
        audioPlayer->stop();
        audioPlayer = nullptr;
    }
}

