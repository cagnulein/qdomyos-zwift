package de.tbressler.waterrower.model;

/**
 * Different types of strokes.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public enum StrokeType {

    /**
     * Start of stroke pull to show when the rowing computer determined acceleration occurring in
     * the paddle.
     */
    START_OF_STROKE,

    /**
     * End of stroke pull to show when the rowing computer determined deceleration occurring in
     * the paddle. (Now entered the relax phase).
     */
    END_OF_STROKE

}