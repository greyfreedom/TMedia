//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#ifndef TMEDIADEMO_VIDEODECODER_H
#define TMEDIADEMO_VIDEODECODER_H

#include <string>

extern "C" {
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};

class VideoDecoder {
public:
    VideoDecoder();

    ~VideoDecoder();

    int prepare(const char *inputPath);

    int decodeFrame(AVFrame **frame, int *width, int *height);

    void releaseDecoder();

private:
    AVFormatContext *pFormatCtx;
    int mVideoStreamIndex;
    AVStream *pVideoStream;
    AVCodec *pVideoDecoder;
    AVCodecContext *pDecodeCtx;
    AVFrame *pFrame;
    AVFrame *pOutFrame;
    AVPacket *pPacket;
    SwsContext *pSwsCtx;
    int width;
    int height;
};


#endif //TMEDIADEMO_VIDEODECODER_H
