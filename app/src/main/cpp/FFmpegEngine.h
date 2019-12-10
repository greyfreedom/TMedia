//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#ifndef TMEDIADEMO_FFMPEGENGINE_H
#define TMEDIADEMO_FFMPEGENGINE_H

#include <logger.h>
#include <memory.h>
#include <memory>
#include <android/native_window.h>

extern "C" {
#include "libavformat/avformat.h"
};

class VideoDecoder;
class AudioPlayer;

class FFmpegEngine {
public:
    FFmpegEngine();

    ~FFmpegEngine();

    void printCodecInfo();

    void decodeVideoToYUV(const char *input, const char *output);

    void playYUV(const char *input, ANativeWindow *window);

    void decodeAudioToPCM(const char *input, const char *output);

    void playAudio(const char *input);

    void pauseAudio();

    void stopAudio();

private:
    std::shared_ptr<AudioPlayer> audioPlayer;
};


#endif //TMEDIADEMO_FFMPEGENGINE_H
