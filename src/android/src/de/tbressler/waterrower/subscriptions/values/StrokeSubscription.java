package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.in.StrokeMessage;
import de.tbressler.waterrower.model.StrokeType;
import de.tbressler.waterrower.subscriptions.ISubscription;
import de.tbressler.waterrower.subscriptions.Priority;

/**
 * A subscription for stroke events.
 *
 * Start of stroke pull to show when the rowing computer determined acceleration occurring in
 * the paddle. End of stroke pull to show when the rowing computer determined deceleration occurring in
 * the paddle. (Now entered the relax phase).
 *
 * This packet is auto transmitted by the rowing computer.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class StrokeSubscription implements ISubscription {

    @Override
    public final Priority getPriority() {
        return Priority.NO_POLLING;
    }

    @Override
    public final AbstractMessage poll() {
        // No poll necessary! Strokes will be send automatically by
        // the WaterRower Performance Monitor.
        return null;
    }

    @Override
    public final void handle(AbstractMessage msg) {
        if (!(msg instanceof StrokeMessage))
            return;
        onStroke(((StrokeMessage) msg).getStrokeType());
    }


    /**
     * Will be called, when the rowing computer determined acceleration (start of stroke) or
     * deceleration (end of stroke) occurring in the paddle.
     *
     * @param strokeType The type of stroke (e.g. start or end), never null.
     */
    abstract protected void onStroke(StrokeType strokeType);

}
