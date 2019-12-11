//
// Created by Wang,Peng(Int'l PM01) on 2019-12-04.
//

extern "C" {
#include "libavutil/opt.h"
}

#include "AudioDecoder.h"
#include "logger.h"

AudioDecoder::AudioDecoder() : pFormatCtx(nullptr), mAudioStreamIndex(-1), pAudioStream(nullptr),
                               pAudioDecoder(nullptr), pDecodeCtx(nullptr), pFrame(nullptr), pOutFrame(nullptr),
                               pPacket(nullptr), pSwrCtx(nullptr), sampleRate(0), sampleFormat(AV_SAMPLE_FMT_NONE),
                               outSampleFmt(AV_SAMPLE_FMT_NONE), nbChannels(0), isLoop(false) {

}

AudioDecoder::~AudioDecoder() {

}

int AudioDecoder::prepare(const char *inputPath, AVSampleFormat outFormat) {
    if (!inputPath) {
        LOGE("audio decoder failed, path invalid. %s", inputPath);
        return -1;
    }
    LOGI("audio decoder inputPath = %s\n", inputPath);
    int ret = 0;
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, inputPath, nullptr, nullptr) < 0) {
        LOGE("audio decoder avformat_open_input failed.");
        releaseDecoder();
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        LOGE("audio decoder avformat_find_stream_info failed.");
        releaseDecoder();
        return -1;
    }
    ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (ret < 0) {
        LOGE("audio decoder decode av_find_best_stream failed.");
        releaseDecoder();
        return -1;
    }
    mAudioStreamIndex = ret;
    pAudioStream = pFormatCtx->streams[mAudioStreamIndex];
    pAudioDecoder = avcodec_find_decoder(pAudioStream->codecpar->codec_id);
    if (!pAudioDecoder) {
        LOGE("audio decoder avcodec_find_decoder failed.");
        releaseDecoder();
        return -1;
    }
    pDecodeCtx = avcodec_alloc_context3(pAudioDecoder);
    if (!pDecodeCtx) {
        LOGE("audio decoder avcodec_alloc_context3 failed.");
        releaseDecoder();
        return -1;
    }
    ret = avcodec_parameters_to_context(pDecodeCtx, pAudioStream->codecpar);
    if (ret < 0) {
        LOGE("audio decoder avcodec_parameters_to_context failed.");
        releaseDecoder();
        return -1;
    }
    ret = avcodec_open2(pDecodeCtx, pAudioDecoder, nullptr);
    if (ret < 0) {
        LOGE("audio decoder avcodec_open2 failed.");
        releaseDecoder();
        return -1;
    }
    sampleRate = pAudioStream->codecpar->sample_rate;
    sampleFormat = static_cast<AVSampleFormat>(pAudioStream->codecpar->format);
    nbChannels = pAudioStream->codecpar->channels;
    outSampleFmt = outFormat;

    if (sampleRate <= 0 || sampleFormat < 0 || nbChannels <= 0 || outFormat < 0) {
        LOGE("audio decoder something error. sampleRate %d, sampleFormat = %d, nbChannels = %d, outFormat = %d",
             sampleRate, sampleFormat, nbChannels, outFormat);
    }
    LOGI("audio decoder, sampleRate %d, sampleFormat = %d, nbChannels = %d, outFormat = %d",
         sampleRate, sampleFormat, nbChannels, outFormat);

    pFrame = av_frame_alloc();
    pOutFrame = av_frame_alloc();
    pOutFrame->sample_rate = sampleRate;
    pOutFrame->format = outFormat;
    pOutFrame->channels = nbChannels;
    pOutFrame->channel_layout = AV_CH_LAYOUT_STEREO;
    pPacket = av_packet_alloc();

    pSwrCtx = swr_alloc();
    if (!pSwrCtx) {
        LOGE("swr context alloc failed");
        releaseDecoder();
        return -1;
    }
    av_opt_set_int(pSwrCtx, "in_channel_layout", nbChannels == 2 ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(pSwrCtx, "out_channel_layout", nbChannels == 2 ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(pSwrCtx, "in_sample_rate", sampleRate, 0);
    av_opt_set_int(pSwrCtx, "out_sample_rate", sampleRate, 0);
    av_opt_set_sample_fmt(pSwrCtx, "in_sample_fmt", sampleFormat, 0);
    av_opt_set_sample_fmt(pSwrCtx, "out_sample_fmt", outFormat, 0);
    if (swr_init(pSwrCtx) < 0) {
        LOGE("swr_init failed");
        return -1;
    }
    return 0;
}

int AudioDecoder::decodeFrame(std::function<void(AVFrame *frame)> callback) {
    int ret = 0;
    START_DECODE:
    ret = av_read_frame(pFormatCtx, pPacket);
    if (ret < 0) {
        if (!isLoop) {
            ret = avcodec_send_packet(pDecodeCtx, nullptr);
        } else {
            av_seek_frame(pFormatCtx, mAudioStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
            LOGI("seek to start..");
            goto START_DECODE;
        }
    } else {
        if (pPacket->stream_index == mAudioStreamIndex) {
            ret = avcodec_send_packet(pDecodeCtx, pPacket);
        } else {
            LOGI("audio decoder skip not audio frame");
            goto START_DECODE;
        }
    }
    if (ret == AVERROR(EAGAIN)) {
        ret = 0;
        LOGI("audio decoder avcodec_send_packet error again");
    } else if (ret == AVERROR_EOF) {
        ret = 0;
        LOGI("audio decoder avcodec_send_packet error eof");
    } else if (ret < 0) {
        return -1;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(pDecodeCtx, pFrame);
        if (ret == AVERROR(EAGAIN)) {
            LOGI("audio decoder avcodec_receive_frame error again");
            return 0;
        } else if (ret == AVERROR_EOF) {
            LOGI("audio decoder avcodec_receive_frame error eof");
            if (isLoop) {
                av_seek_frame(pFormatCtx, mAudioStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
                LOGI("seek to start..");
                goto START_DECODE;
            }
            return -1;
        } else if (ret < 0) {
            return -1;
        } else {
            pFrame->channel_layout = av_get_default_channel_layout(nbChannels);
            pOutFrame->nb_samples = pFrame->nb_samples;
            LOGI("input frame samplerate = %d, nb_sample = %d, fmt = %d, channel_layout = %lld",
                 pFrame->sample_rate, pFrame->nb_samples, pFrame->format, pFrame->channel_layout);
            LOGI("out frame samplerate = %d, nb_sample = %d, fmt = %d, channel_layout = %lld",
                 pOutFrame->sample_rate, pOutFrame->nb_samples, pOutFrame->format, pOutFrame->channel_layout);
            int64_t delay = swr_get_delay(pSwrCtx, sampleRate);
            LOGI("resample delay = %lld", delay);
            ret = swr_convert_frame(pSwrCtx, pOutFrame, pFrame);
            if (ret < 0) {
                LOGE("swr_convert_frame error %s", av_err2str(ret));
            }
            if (callback) {
                callback(pOutFrame);
            }
        }
    }

}

void AudioDecoder::releaseDecoder() {
    if (pFrame) {
        av_frame_free(&pFrame);
    }
    if (pOutFrame) {
        av_frame_free(&pOutFrame);
    }
    if (pSwrCtx) {
        swr_free(&pSwrCtx);
    }
    if (pDecodeCtx) {
        avcodec_free_context(&pDecodeCtx);
        pDecodeCtx = nullptr;
    }
    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = nullptr;
    }
}

int AudioDecoder::getOutputSampleRate() {
    return sampleRate;
}

int AudioDecoder::getOutputSampleFmtBit() {
    int bit = 0;
    if (outSampleFmt == AV_SAMPLE_FMT_U8
        || outSampleFmt == AV_SAMPLE_FMT_U8P) {
        bit = 8;
    } else if (outSampleFmt == AV_SAMPLE_FMT_S16
               || outSampleFmt == AV_SAMPLE_FMT_S16P) {
        bit = 16;
    } else if (outSampleFmt == AV_SAMPLE_FMT_S32
               || outSampleFmt == AV_SAMPLE_FMT_FLT
               || outSampleFmt == AV_SAMPLE_FMT_S32) {
        bit = 32;
    } else if (outSampleFmt == AV_SAMPLE_FMT_DBL
               || outSampleFmt == AV_SAMPLE_FMT_DBLP
               || outSampleFmt == AV_SAMPLE_FMT_S64
               || outSampleFmt == AV_SAMPLE_FMT_S64P) {
        bit = 64;
    } else {
        bit = 16;
    }
    return bit;
}

int AudioDecoder::getOutputChannels() {
    return nbChannels;
}

void AudioDecoder::setLoop(bool loop) {
    isLoop = loop;
}
