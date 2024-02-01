package com.che.zap.utils

import timber.log.Timber
import java.lang.Exception

object Logger : BaseObservable<Logger.LogCallback>() {

    interface LogCallback {
        fun newLogLine(line: String)
    }

    fun d(line: String) {
        Timber.d(line)
        callback(line)
    }

    fun i(line: String) {
        Timber.i(line)
        callback(line)
    }

    fun e(line: String) {
        Timber.e(line)
        callback(line)
    }

    fun e(line: String, exception: Exception) {
        Timber.e(exception, line)
        callback(line)
    }

    private fun callback(line: String) {
        for (listener in listeners) {
            listener.newLogLine(line)
        }
    }
}