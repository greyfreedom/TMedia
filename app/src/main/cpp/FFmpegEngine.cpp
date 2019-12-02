//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#include "FFmpegEngine.h"
#include "VideoDecoder.h"
#include <thread>

using namespace std;

FFmpegEngine::FFmpegEngine() {
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
        decoder->releaseDecoder();
        if (decoder->prepare(input) < 0) {
            decoder->releaseDecoder();
        }
        int width = 0, height = 0;
        AVFrame *frame = nullptr;
        int ret;
        do {
            ret = decoder->decodeFrame(&frame, &width, &height);
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
