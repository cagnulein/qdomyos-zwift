package com.che.zap.utils

import java.util.Collections
import java.util.concurrent.ConcurrentHashMap

/**
 * Base class for observable entities in the application
 * @param <LISTENER_CLASS>the class of the listeners</LISTENER_CLASS>
 */
@Suppress("UnnecessaryAbstractClass")
abstract class BaseObservable<LISTENER_CLASS> {
    // thread-safe set of listeners
    private val mListeners =
        Collections.newSetFromMap(
            ConcurrentHashMap<LISTENER_CLASS, Boolean>(1)
        )

    fun registerListener(listener: LISTENER_CLASS) {
        mListeners.add(listener)
        onListenerRegistered()
    }

    open fun onListenerRegistered() { } // the implementation can choose to give the current values

    fun unregisterListener(listener: LISTENER_CLASS) {
        mListeners.remove(listener)
    }

    /**
     * Get a reference to the unmodifiable set containing all the registered listeners.
     */
    protected val listeners: Set<LISTENER_CLASS>
        get() = Collections.unmodifiableSet(mListeners)
}