package com.demo.tmediademo.utils

import android.content.res.AssetManager
import android.os.Environment
import java.io.BufferedOutputStream
import java.io.File
import java.io.FileOutputStream

object AssetsUtil {
    fun copyAssetsTo(assetManager: AssetManager, path: String) {
        val list = assetManager.list("media")
        list?.forEach {
            copyTo(assetManager, "media/", it, path)
        }
    }

    fun copyTo(assetManager: AssetManager, relativePath: String, fileName: String, path: String) {
        if (fileName.isEmpty() || path.isEmpty()) return
        val dir = File(path)
        if (dir.exists() && !dir.isDirectory) return
        if (!dir.exists()) dir.mkdir()
        val file = File(path, fileName)
        if (file.exists()) return
        val bis = assetManager.open(relativePath + fileName)
        val bos = BufferedOutputStream(FileOutputStream(file))
        val array = ByteArray(1024)
        while (bis.read(array) != -1) {
            bos.write(array)
        }
        bos.flush()
        bos.close()
        bis.close()
    }
}

object PathUtil {
    fun getAppExternalPath() : String {
        return Environment.getExternalStorageDirectory().path + "/tmedia/"
    }
}