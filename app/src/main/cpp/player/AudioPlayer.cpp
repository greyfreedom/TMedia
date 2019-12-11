//
// Created by Wang,Peng(Int'l PM01) on 2019-12-09.
//

#include "AudioPlayer.h"
#include "logger.h"

AudioPlayer::AudioPlayer() : engineObject(nullptr), engineEngine(nullptr),
                             outputMixObject(nullptr), outputMixEnvironmentalReverb(nullptr),
                             reverbSettings(SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR),
                             pcmPlayerObject(nullptr), pcmPlayerPlay(nullptr), audioDecoder(nullptr),
                             pcmPlayerVolume(nullptr), pcmBufferQueue(nullptr) {

}

AudioPlayer::~AudioPlayer() {

}

static void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    LOGI("pcmBufferCallBack, need more data.");
    auto *audioPlayer = (AudioPlayer *) context;
    if (audioPlayer) {
        audioPlayer->feedPcm();
    }
}

int AudioPlayer::prepare(const char *inputPath) {
    audioDecoder = std::make_shared<AudioDecoder>();
    if (audioDecoder->prepare(inputPath, PLAYER_TARGET_FMT) < 0) {
        LOGE("AudioPlayer decoder prepare error.");
        clean();
        return -1;
    }
    audioDecoder->setLoop(true);
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("slCreateEngine error..");
        clean();
        return -1;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("slCreateEngine realize error..");
        clean();
        return -1;
    }
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("slCreateEngine get interface error..");
        clean();
        return -1;
    }

    const SLInterfaceID mixIds[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mixReq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mixIds, mixReq);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateOutputMix error..");
        clean();
        return -1;
    }
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateOutputMix realize error..");
        clean();
        return -1;
    }
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,
                                                                          &reverbSettings);
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, nullptr};

    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 6};
    int channels = audioDecoder->getOutputChannels();
    int sampleRate = audioDecoder->getOutputSampleRate();
    int sampleFmtBit = audioDecoder->getOutputSampleFmtBit();
    SLuint32 channelsInfo = getChannels(channels);
    SLuint32 channelLayoutInfo = getChannelLayout(channels);
    SLuint32 sampleRateInfo = getSampleRate(sampleRate);
    SLuint32 sampleFormatInfo = getSampleFormat(sampleFmtBit);
    LOGI("init pcm info, channels = %d, channelLayout = %d, sampleRate = %d, sampleFormat = %d",
         channelsInfo, channelLayoutInfo, sampleRateInfo, sampleFormatInfo);
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            channelsInfo,
            sampleRateInfo,
            sampleFormatInfo,
            sampleFormatInfo,
            channelLayoutInfo,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource slDataSource = {&android_queue, &pcm};

    const SLInterfaceID playerIds[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean playerReq[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 3, playerIds,
                                                playerReq);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer error..");
        clean();
        return -1;
    }

    result = (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer realize error..");
        clean();
        return -1;
    }

    result = (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateOutputMix get interface error..");
        clean();
        return -1;
    }


    result = (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("get buffer interface error..");
        clean();
        return -1;
    }

    result = (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("RegisterCallback error..");
        clean();
        return -1;
    }


    result = (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmPlayerVolume);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("volume interface error..");
        clean();
        return -1;
    }
    return 0;
}

SLuint32 AudioPlayer::getChannels(int channels) {
    return channels == 2 ? 2 : 1;
}

SLuint32 AudioPlayer::getChannelLayout(int channels) {
    return channels == 2 ? (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT) : SL_SPEAKER_FRONT_CENTER;
}

SLuint32 AudioPlayer::getSampleRate(int sampleRate) {
    int rate = sampleRate;
    SLuint32 result;
    switch (rate) {
        case 8000:
            result = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            result = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            result = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            result = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            result = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            result = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            result = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            result = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            result = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            result = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            result = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            result = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            result = SL_SAMPLINGRATE_192;
            break;
        default:
            result = SL_SAMPLINGRATE_44_1;
    }
    return result;
}

SLuint32 AudioPlayer::getSampleFormat(int fmtBit) {
    SLuint32 result;
    switch (fmtBit) {
        case 8:
            result = SL_PCMSAMPLEFORMAT_FIXED_8;
            break;
        case 16:
            result = SL_PCMSAMPLEFORMAT_FIXED_16;
            break;
        case 20:
            result = SL_PCMSAMPLEFORMAT_FIXED_20;
            break;
        case 24:
            result = SL_PCMSAMPLEFORMAT_FIXED_24;
            break;
        case 28:
            result = SL_PCMSAMPLEFORMAT_FIXED_28;
            break;
        case 32:
            result = SL_PCMSAMPLEFORMAT_FIXED_32;
            break;
        default:
            result = SL_PCMSAMPLEFORMAT_FIXED_16;
    }
    return result;
}

void AudioPlayer::clean() {
    if (audioDecoder) {
        audioDecoder->releaseDecoder();
        audioDecoder = nullptr;
    }
    if (pcmPlayerObject != nullptr) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = nullptr;
        pcmPlayerPlay = nullptr;
        pcmBufferQueue = nullptr;
    }

    if (outputMixObject != nullptr) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = nullptr;
        outputMixEnvironmentalReverb = nullptr;
    }

    if (engineObject != nullptr) {
        (*engineObject)->Destroy(engineObject);
        engineObject = nullptr;
        engineEngine = nullptr;
    }
    LOGI("audio player release..");
}

void AudioPlayer::feedPcm() {
    if (audioDecoder) {
        audioDecoder->decodeFrame([this](AVFrame *frame) mutable {
            LOGI("decode frame");
            int size = frame->nb_samples * frame->channels * av_get_bytes_per_sample(PLAYER_TARGET_FMT);
            LOGI("decode a frame, size = %d, linesize = %d", size, frame->linesize[0]);
            (*pcmBufferQueue)->Enqueue(pcmBufferQueue, frame->data[0], size);
        });
    }
}

void AudioPlayer::play() {
    if (pcmPlayerPlay) {
        SLuint32 state;
        (*pcmPlayerPlay)->GetPlayState(pcmPlayerPlay, &state);
        LOGI("audio player play, state = %d", state);
        if (state == SL_PLAYSTATE_PAUSED) {
            (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
        } else if (state == SL_PLAYSTATE_STOPPED) {
            (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
            feedPcm();
        }
    }
}

void AudioPlayer::pause() {
    if (pcmPlayerPlay) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

void AudioPlayer::stop() {
    if (pcmPlayerPlay) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
    clean();
}
