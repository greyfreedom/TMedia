package com.demo.tmediademo

import android.app.Activity
import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.Surface
import android.view.SurfaceHolder
import androidx.core.graphics.PathUtils
import com.demo.tmediademo.utils.PathUtil
import kotlinx.android.synthetic.main.activity_player.*

class PlayerActivity : AppCompatActivity(), SurfaceHolder.Callback {

    companion object {
        fun start(activity:Activity?) {
            activity?.startActivity(Intent(activity, PlayerActivity::class.java))
        }
    }

    var mExtralPath:String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_player)
        mExtralPath = PathUtil.getAppExternalPath()
        playerSurface.holder.addCallback(this)
    }

    override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
    }

    override fun surfaceDestroyed(holder: SurfaceHolder?) {
        holder?.surface?.release()
    }

    override fun surfaceCreated(holder: SurfaceHolder?) {
        if (holder != null) {
            FFmpeg.playYuv(mExtralPath + "ffmpeg.mp4", holder.surface)
        }
    }


}
