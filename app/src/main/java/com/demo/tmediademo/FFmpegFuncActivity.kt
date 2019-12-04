package com.demo.tmediademo

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.demo.tmediademo.adapter.FFmpegFuncAdapter
import com.demo.tmediademo.domain.FFmpegFuncItem
import com.demo.tmediademo.utils.PathUtil.getAppExternalPath
import kotlinx.android.synthetic.main.activity_ffmpeg_func.*

class FFmpegFuncActivity : AppCompatActivity() {

    var mExtralPath: String = ""

    companion object {
        fun start(activity: Activity?) {
            activity?.startActivity(Intent(activity, FFmpegFuncActivity::class.java))
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_ffmpeg_func)
        mExtralPath = getAppExternalPath()
        FFmpeg.setUpNative()
        ffmpegFuncRecyclerView.adapter = FFmpegFuncAdapter(prepareData())
        ffmpegFuncRecyclerView.layoutManager = GridLayoutManager(this, 2, RecyclerView.VERTICAL, false)

    }

    private fun prepareData(): List<FFmpegFuncItem> {
        var dataList = ArrayList<FFmpegFuncItem>()
        dataList.add(FFmpegFuncItem(
            "Play",
            View.OnClickListener {
                val inputStr = mExtralPath + "ffmpeg.mp4"
                PlayerActivity.start(this)
            }
        ))
        dataList.add(FFmpegFuncItem(
            "LogInfo",
            View.OnClickListener { FFmpeg.logInfo() }
        ))
        dataList.add(FFmpegFuncItem(
            "DecodeVideoToYUV",
            View.OnClickListener {
                val inputStr = mExtralPath + "ffmpeg.mp4"
                val outputStr = mExtralPath + "ffmpeg.yuv"
                FFmpeg.decodeVideoToYUV(inputStr, outputStr)
            }
        ))
        dataList.add(FFmpegFuncItem(
            "DecodeAudioToPCM",
            View.OnClickListener {
                val inputStr = mExtralPath + "bgm.wav"
                val outputStr = mExtralPath + "bgm.pcm"
                FFmpeg.decodeAudioToPCM(inputStr, outputStr)
            }
        ))
        return dataList
    }

    override fun onDestroy() {
        super.onDestroy()
        FFmpeg.releaseNative()
    }
}