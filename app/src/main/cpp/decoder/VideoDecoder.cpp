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
                               pPacket(nullptr), pSwsCtx(nullptr), yuvFile(nullptr) {

}

VideoDecoder::~VideoDecoder() {

}

int VideoDecoder::decode(const char *inputPath, const char *outputPath) {
    if (!inputPath || !outputPath) {
        LOGE("decoder decode failed, path invalid.");
        return -1;
    }
    LOGI("decoder inputPath = %s\noutputPath = %s", inputPath, outputPath);
    int ret = 0;
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, inputPath, nullptr, nullptr) < 0) {
        LOGE("decoder decode avformat_open_input failed.");
        releaseDecoder();
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        LOGE("decoder decode avformat_find_stream_info failed.");
        releaseDecoder();
        return -1;
    }
    ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (ret < 0) {
        LOGE("decoder decode av_find_best_stream failed.");
        releaseDecoder();
        return -1;
    }
    mVideoStreamIndex = ret;
    pVideoStream = pFormatCtx->streams[mVideoStreamIndex];
    pVideoDecoder = avcodec_find_decoder(pVideoStream->codecpar->codec_id);
    if (!pVideoDecoder) {
        LOGE("decoder decode avcodec_find_decoder failed.");
        releaseDecoder();
        return -1;
    }
    pDecodeCtx = avcodec_alloc_context3(pVideoDecoder);
    if (!pDecodeCtx) {
        LOGE("decoder decode avcodec_alloc_context3 failed.");
        releaseDecoder();
        return -1;
    }
    ret = avcodec_parameters_to_context(pDecodeCtx, pVideoStream->codecpar);
    if (ret < 0) {
        LOGE("decoder decode avcodec_parameters_to_context failed.");
        releaseDecoder();
        return -1;
    }
    ret = avcodec_open2(pDecodeCtx, pVideoDecoder, nullptr);
    if (ret < 0) {
        LOGE("decoder decode avcodec_open2 failed.");
        releaseDecoder();
        return -1;
    }
    pFrame = av_frame_alloc();
    pOutFrame = av_frame_alloc();
    int width = pDecodeCtx->width;
    int height = pDecodeCtx->height;
    if (width <= 0 || height <= 0) {
        LOGE("decoder decode width <= 0 || height <= 0.");
        releaseDecoder();
        return -1;
    }
    LOGI("video width =%d, height = %d", width, height);
    ret = av_image_alloc(pFrame->data, pFrame->linesize, width, height, pDecodeCtx->pix_fmt, 1);
    if (ret < 0) {
        LOGE("decoder decode av_image_alloc pFrame failed.");
        releaseDecoder();
        return -1;
    }
    ret = av_image_alloc(pOutFrame->data, pOutFrame->linesize, width, height, AV_PIX_FMT_YUV420P, 1);
    if (ret < 0) {
        LOGE("decoder decode av_image_alloc pOutFrame failed.");
        releaseDecoder();
        return -1;
    }
    pPacket = av_packet_alloc();
    pSwsCtx = sws_getContext(width, height, pDecodeCtx->pix_fmt,
                             width, height, AV_PIX_FMT_YUV420P,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);
    yuvFile = fopen(outputPath, "wb+");
    if (!yuvFile) {
        LOGE("decoder decode fopen %s failed.", outputPath);
        releaseDecoder();
        return -1;
    }
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        decodeInner(pPacket, width, height);
    }
    decodeInner(nullptr, width, height);
    fclose(yuvFile);
    releaseDecoder();
    LOGI("Decode video to yuv finished.");
    return 0;
}

int VideoDecoder::decodeInner(AVPacket *packet, int width, int height) {
    int ret = 0;
    ret = avcodec_send_packet(pDecodeCtx, packet);
    if (ret < 0 && (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)) {
        return -1;
    }
    ret = avcodec_receive_frame(pDecodeCtx, pFrame);
    if (ret < 0) {
        return -1;
    }
    int h = sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pDecodeCtx->height, pOutFrame->data,
                      pOutFrame->linesize);
    fwrite(pOutFrame->data[0], 1, width * h, yuvFile);
    fwrite(pOutFrame->data[1], 1, width * h / 4, yuvFile);
    fwrite(pOutFrame->data[2], 1, width * h / 4, yuvFile);
    return 0;
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
