//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#ifndef TMEDIADEMO_FFMPEGENGINE_H
#define TMEDIADEMO_FFMPEGENGINE_H

#include <logger.h>

extern "C" {
#include "libavformat/avformat.h"
};

class FFmpegEngine {
public:
    FFmpegEngine();

    ~FFmpegEngine();

    void printCodecInfo();
};


#endif //TMEDIADEMO_FFMPEGENGINE_H
