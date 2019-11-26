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
import kotlinx.android.synthetic.main.activity_ffmpeg_func.*

class FFmpegFuncActivity : AppCompatActivity() {

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private external fun logInfo()

    private external fun releaseNative()

    private external fun setUpNative()

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }

        fun start(activity: Activity?) {
            activity?.startActivity(Intent(activity, FFmpegFuncActivity::class.java))
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_ffmpeg_func)
        setUpNative()
        ffmpegFuncRecyclerView.adapter = FFmpegFuncAdapter(prepareData())
        ffmpegFuncRecyclerView.layoutManager = GridLayoutManager(this, 2, RecyclerView.VERTICAL, false)

    }

    private fun prepareData(): List<FFmpegFuncItem> {
        var dataList = ArrayList<FFmpegFuncItem>()
        dataList.add(FFmpegFuncItem(
            "LogInfo",
            View.OnClickListener { logInfo() }
        ))
        return dataList
    }

    override fun onDestroy() {
        super.onDestroy()
        releaseNative()
    }
}
