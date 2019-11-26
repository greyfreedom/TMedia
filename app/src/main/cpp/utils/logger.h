//
// Created by Wang,Peng(Int'l PM01) on 2019-11-26.
//

#ifndef TMEDIADEMO_LOGGER_H
#define TMEDIADEMO_LOGGER_H

#include <android/log.h>

#define LOGI(format, ...) __android_log_print(ANDROID_LOG_INFO, "native", format, ##__VA_ARGS__)
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)

#endif //TMEDIADEMO_LOGGER_H
