//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#include "VideoDecoder.h"
#include "logger.h"

extern "C" {
#include "libavutil/imgutils.h"
}

VideoDecoder::VideoDecoder() : pFormatCtx(nullptr), mVideoStreamIndex(0), pVideoStream(nullptr),
                               pVideoDecoder(nullptr), pDecodeCtx(nullptr), pFrame(nullptr), pOutFrame(nullptr),
                               pPacket(nullptr), pSwsCtx(nullptr), width(0), height(0) {

}

VideoDecoder::~VideoDecoder() {

}

int VideoDecoder::prepare(const char *inputPath, AVPixelFormat format) {
    if (!inputPath) {
        LOGE("video decoder failed, path invalid. %s", inputPath);
        return -1;
    }
    LOGI("video decoder inputPath = %s\n", inputPath);
    int ret = 0;
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, inputPath, nullptr, nullptr) < 0) {
        LOGE("video decoder avformat_open_input failed.");
        releaseDecoder();
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        LOGE("video decoder avformat_find_stream_info failed.");
        releaseDecoder();
        return -1;
    }
    ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        LOGE("video decoder decode av_find_best_stream failed.");
        releaseDecoder();
        return -1;
    }
    mVideoStreamIndex = ret;
    pVideoStream = pFormatCtx->streams[mVideoStreamIndex];
    pVideoDecoder = avcodec_find_decoder(pVideoStream->codecpar->codec_id);
    if (!pVideoDecoder) {
        LOGE("video decoder avcodec_find_decoder failed.");
        releaseDecoder();
        return -1;
    }
    pDecodeCtx = avcodec_alloc_context3(pVideoDecoder);
    if (!pDecodeCtx) {
        LOGE("video decoder avcodec_alloc_context3 failed.");
        releaseDecoder();
        return -1;
    }
    ret = avcodec_parameters_to_context(pDecodeCtx, pVideoStream->codecpar);
    if (ret < 0) {
        LOGE("video decoder avcodec_parameters_to_context failed.");
        releaseDecoder();
        return -1;
    }
    ret = avcodec_open2(pDecodeCtx, pVideoDecoder, nullptr);
    if (ret < 0) {
        LOGE("video decoder avcodec_open2 failed.");
        releaseDecoder();
        return -1;
    }
    pFrame = av_frame_alloc();
    pOutFrame = av_frame_alloc();
    width = pDecodeCtx->width;
    height = pDecodeCtx->height;
    if (width <= 0 || height <= 0) {
        LOGE("video decoder width <= 0 || height <= 0.");
        releaseDecoder();
        return -1;
    }
    LOGI("video width =%d, height = %d", width, height);
    ret = av_image_alloc(pFrame->data, pFrame->linesize, width, height, pDecodeCtx->pix_fmt, 1);
    if (ret < 0) {
        LOGE("video decoder av_image_alloc pFrame failed.");
        releaseDecoder();
        return -1;
    }
    ret = av_image_alloc(pOutFrame->data, pOutFrame->linesize, width, height, format, 1);
    if (ret < 0) {
        LOGE("video decoder av_image_alloc pOutFrame failed.");
        releaseDecoder();
        return -1;
    }
    pPacket = av_packet_alloc();
    pSwsCtx = sws_getContext(width, height, pDecodeCtx->pix_fmt,
                             width, height, format,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);
    return 0;
}

int VideoDecoder::decodeFrame(AVFrame **frame) {
    int ret = 0;
    int readResult = 0;
    readResult = av_read_frame(pFormatCtx, pPacket);
    if (pPacket->stream_index != mVideoStreamIndex) return 0;
    if (readResult >= 0) {
        ret = avcodec_send_packet(pDecodeCtx, pPacket);
    } else {
        ret = avcodec_send_packet(pDecodeCtx, nullptr);
    }
    if (ret < 0 && (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)) {
        LOGI("video avcodec_send_packet, %s", av_err2str(ret));
        return readResult;
    }
    ret = avcodec_receive_frame(pDecodeCtx, pFrame);
    if (ret < 0) {
        LOGI("video avcodec_receive_frame, %s", av_err2str(ret));
        return readResult;
    }
    int h = sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pDecodeCtx->height, pOutFrame->data,
                      pOutFrame->linesize);
    *frame = pOutFrame;
    LOGI("Decode a video frame to yuv, width = %d, height = %d", width, h);
    return readResult;
}

void VideoDecoder::releaseDecoder() {
    if (pFrame) {
        av_frame_free(&pFrame);
    }
    if (pOutFrame) {
        av_frame_free(&pOutFrame);
    }
    if (pSwsCtx) {
        sws_freeContext(pSwsCtx);
        pSwsCtx = nullptr;
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

int VideoDecoder::getFrameRate() {
    if (pVideoStream) {
        return pVideoStream->avg_frame_rate.num / pVideoStream->avg_frame_rate.den;
    }
    return 0;
}

int VideoDecoder::getWidth() {
    return width;
}

int VideoDecoder::getHeight() {
    return height;
}
