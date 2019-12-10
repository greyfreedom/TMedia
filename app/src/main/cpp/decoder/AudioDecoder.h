//
// Created by Wang,Peng(Int'l PM01) on 2019-12-04.
//

#ifndef TMEDIADEMO_AUDIODECODER_H
#define TMEDIADEMO_AUDIODECODER_H

#include <string>

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};

class AudioDecoder {
public:
    AudioDecoder();

    ~AudioDecoder();

    int prepare(const char *inputPath, AVSampleFormat outFormat);

    int decodeFrame(std::function<void(AVFrame *frame)> callback);

    void setLoop(bool loop);

    void releaseDecoder();

    int getOutputSampleRate();

    int getOutputSampleFmtBit();

    int getOutputChannels();

private:
    AVFormatContext *pFormatCtx;
    int mAudioStreamIndex;
    AVStream *pAudioStream;
    AVCodec *pAudioDecoder;
    AVCodecContext *pDecodeCtx;
    AVFrame *pFrame;
    AVFrame *pOutFrame;
    AVPacket *pPacket;
    SwrContext *pSwrCtx;
    int sampleRate;
    AVSampleFormat sampleFormat;
    AVSampleFormat outSampleFmt;
    int nbChannels;
    bool isLoop;
};


#endif //TMEDIADEMO_AUDIODECODER_H
