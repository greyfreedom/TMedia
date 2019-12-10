//
// Created by Wang,Peng(Int'l PM01) on 2019-12-09.
//

#ifndef TMEDIADEMO_AUDIOPLAYER_H
#define TMEDIADEMO_AUDIOPLAYER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <memory>
#include "AudioDecoder.h"


class AudioPlayer {
public:
    AudioPlayer();

    ~AudioPlayer();

    int prepare(const char *inputPath);

    void feedPcm();

    void play();

    void pause();

    void stop();

private:
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb;
    SLEnvironmentalReverbSettings reverbSettings;

    SLObjectItf pcmPlayerObject;
    SLPlayItf pcmPlayerPlay;
    SLVolumeItf pcmPlayerVolume;

    SLAndroidSimpleBufferQueueItf pcmBufferQueue;

    std::shared_ptr<AudioDecoder> audioDecoder;

    void clean();

    SLuint32 getSampleFormat(int fmtBit);

    SLuint32 getSampleRate(int sampleRate);

    SLuint32 getChannelLayout(int channels);

    SLuint32 getChannels(int channels);
};


#endif //TMEDIADEMO_AUDIOPLAYER_H
