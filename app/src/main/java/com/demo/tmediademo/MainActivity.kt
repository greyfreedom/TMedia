package com.demo.tmediademo

import android.Manifest
import android.app.Activity
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.demo.tmediademo.utils.AssetsUtil
import com.demo.tmediademo.utils.PathUtil.getAppExternalPath
import kotlinx.android.synthetic.main.activity_main.*
import org.jetbrains.anko.alert
import org.jetbrains.anko.noButton
import org.jetbrains.anko.toast
import org.jetbrains.anko.yesButton

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        ffmpegButton.setOnClickListener { FFmpegFuncActivity.start(this) }
        handlePermission(this)
    }

    private fun copyAsset() {
        AssetsUtil.copyAssetsTo(assets, getAppExternalPath())
    }


    private fun handlePermission(context: Activity) {
        val permission = Manifest.permission.WRITE_EXTERNAL_STORAGE
        val checkPermission = context.let { ActivityCompat.checkSelfPermission(it, permission) }
        if (checkPermission != PackageManager.PERMISSION_GRANTED) {
            val showDialog = context.let { ActivityCompat.shouldShowRequestPermissionRationale(it, permission) }
            if (showDialog) {
                alert("给个存储权限啊") {
                    yesButton {
                        requestPermission()
                    }
                    noButton { finish() }
                }.show()
            } else {
                requestPermission()
            }
        } else {
            copyAsset()
        }
    }

    private fun requestPermission() {
        this.let {
            ActivityCompat.requestPermissions(it, arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), 123)
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 123) {
            if (grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                toast("给个存储权限啊兄dei")
            } else {
                copyAsset()
            }
        }
    }
}
