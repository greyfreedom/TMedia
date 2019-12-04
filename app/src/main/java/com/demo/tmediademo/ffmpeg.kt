package com.demo.tmediademo

import android.view.Surface

class FFmpeg {

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }

        external fun logInfo()

        external fun releaseNative()

        external fun setUpNative()

        external fun decodeVideoToYUV(inputPath: String, outputPath: String)

        external fun playYuv(inputPath: String, surface: Surface)

        external fun decodeAudioToPCM(inputPath: String, outputPath: String)
    }
}