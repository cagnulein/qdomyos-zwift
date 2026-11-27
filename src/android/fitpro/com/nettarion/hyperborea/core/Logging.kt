package com.nettarion.hyperborea.core

import kotlinx.coroutines.flow.StateFlow

enum class LogLevel { DEBUG, INFO, WARN, ERROR }

data class LogEntry(
    val timestamp: Long,
    val level: LogLevel,
    val tag: String,
    val message: String,
    val throwable: String? = null,
)

interface AppLogger {
    fun d(tag: String, message: String)
    fun i(tag: String, message: String)
    fun w(tag: String, message: String)
    fun e(tag: String, message: String, throwable: Throwable? = null)
}

interface LogStore {
    val entries: StateFlow<List<LogEntry>>
    val size: StateFlow<Int>
    fun clear()
    fun export(): String
}
