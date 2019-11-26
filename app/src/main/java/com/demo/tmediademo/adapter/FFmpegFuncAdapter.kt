package com.demo.tmediademo.adapter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.demo.tmediademo.R
import com.demo.tmediademo.domain.FFmpegFuncItem
import kotlinx.android.synthetic.main.recycler_item_ffmpeg_func_recycler_view.view.*

class FFmpegFuncAdapter(private val data: List<FFmpegFuncItem>) : RecyclerView.Adapter<FFmpegFuncViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): FFmpegFuncViewHolder {
        val view =
            LayoutInflater.from(parent.context).inflate(R.layout.recycler_item_ffmpeg_func_recycler_view, parent, false)
        return FFmpegFuncViewHolder(view)
    }

    override fun getItemCount(): Int {
        return data.size
    }

    override fun onBindViewHolder(holder: FFmpegFuncViewHolder, position: Int) {
        holder.bind(data[position])
    }

}

class FFmpegFuncViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
    fun bind(itemData: FFmpegFuncItem) {
        itemView.LogInfo.text = itemData.content
        itemView.LogInfo.setOnClickListener(itemData.clickListener)
    }

}