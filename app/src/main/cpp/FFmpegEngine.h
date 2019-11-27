//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#ifndef TMEDIADEMO_FFMPEGENGINE_H
#define TMEDIADEMO_FFMPEGENGINE_H

#include <logger.h>
#include <memory.h>
#include <memory>

extern "C" {
#include "libavformat/avformat.h"
};

class VideoDecoder;

class FFmpegEngine {
public:
    FFmpegEngine();

    ~FFmpegEngine();

    void printCodecInfo();

    void decodeVideoToYUV(const char *input, const char *output);

    void releaseFFmpeg();

private:
    std::shared_ptr<VideoDecoder> mVideoDecoder;
};


#endif //TMEDIADEMO_FFMPEGENGINE_H
