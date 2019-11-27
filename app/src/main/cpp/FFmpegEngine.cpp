//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#include "FFmpegEngine.h"
#include "VideoDecoder.h"

FFmpegEngine::FFmpegEngine() : mVideoDecoder(nullptr) {
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
    if (!mVideoDecoder) {
        mVideoDecoder = std::make_shared<VideoDecoder>();
    }
    mVideoDecoder->decode(input, output);
}

void FFmpegEngine::releaseFFmpeg() {
    if (mVideoDecoder) {
        mVideoDecoder->releaseDecoder();
    }
}
