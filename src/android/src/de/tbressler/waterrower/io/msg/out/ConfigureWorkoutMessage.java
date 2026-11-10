package de.tbressler.waterrower.io.msg.out;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.workout.WorkoutUnit;

import static com.google.common.base.MoreObjects.toStringHelper;
import static de.tbressler.waterrower.io.msg.out.ConfigureWorkoutMessage.MessageType.END_INTERVAL_WORKOUT;
import static java.util.Objects.requireNonNull;

/**
 * This message will configure distance / duration workouts or interval workouts.
 *
 * Workouts are configured with at least 1 packet for single distance and duration workouts while interval workouts
 * require multiple messages to define the total number of intervals. Should any message be incorrectly formatted
 * then an ERROR will be issued, failing to complete an interval workout will result in it being scrapped at the
 * next PING packet.
 *
 * This means the application will have a second to download and confirm the whole of a interval workout, the PING
 * transmit timer will be set to 0 at the start of the interval workout programming. Because of the need to use the
 * PING and rowing is NOT recommended during workout programming the application must warn the user to stop all rowing
 * and wait for the PING messages before attempting to load a workout program.
 *
 * It is also recommended that the rowing computer is RESET prior to downloading any workout, a PING after a reset
 * will indicate the rowing computer is ready again for data.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ConfigureWorkoutMessage extends AbstractMessage {

    /**
     * Type of workout message.
     */
    public enum MessageType {

        /* Configure a single workout. */
        SINGLE_WORKOUT,

        /* Start a interval distance/duration workout. */
        START_INTERVAL_WORKOUT,

        /* Add an interval to a workout. */
        ADD_INTERVAL_WORKOUT,

        /* End configuration of an interval workout. */
        END_INTERVAL_WORKOUT

    }

    /* The type of workout message. */
    private final MessageType messageType;

    /* The distance / duration of the workout or the workout interval. */
    private final int distance;

    /* The unit of the workout or the workout interval. */
    private final WorkoutUnit unit;

    /* The rest interval. */
    private final int restInterval;


    /**
     * This message will configure distance / duration workouts or interval workouts.
     *
     * @param messageType The type of message (single workout or start interval workout). Do not use this constructor
     *                    for add/end interval workout messages.
     * @param distance The distance (in meters/strokes) or duration (in seconds) of the workout. When unit = METERS,
     *                 MILES or KMS: this value is in Meters, the display value for miles is a conversion and valid
     *                 values are 0x0001 to 0xFA00. When unit = STROKES this value is the number of strokes and valid
     *                 values are 0x0001 to 0x1388. When unit = SECONDS this value is in seconds. Valid values are 0x0001
     *                 to 0x4650. This value is limited to 5 Hours, which is 18,000 seconds.
     * @param unit The unit of the workout, must not be null.
     */
    public ConfigureWorkoutMessage(MessageType messageType, int distance, WorkoutUnit unit) {
        this.messageType = requireNonNull(messageType);
        this.distance = checkDistance(messageType, distance, unit);
        this.unit = requireNonNull(unit);
        this.restInterval = checkRestInterval(messageType, -1);
    }

    /**
     * This message will configure distance / duration workouts or interval workouts.
     *
     * @param messageType The type of message (add or end interval workout). Do not use this constructor
     *                    for single workout or start interval workout messages.
     * @param distance The distance (in meters/strokes) or duration (in seconds) of the workout. When unit = METERS,
     *                 MILES or KMS: this value is in Meters, the display value for miles is a conversion and valid
     *                 values are 0x0001 to 0xFA00. When unit = STROKES this value is the number of strokes and valid
     *                 values are 0x0001 to 0x1388. When unit = SECONDS this value is in seconds. Valid values are 0x0001
     *                 to 0x4650. This value is limited to 5 Hours, which is 18,000 seconds.
     * @param unit The unit of the workout, must not be null.
     * @param restInterval The rest interval (in seconds), which must be set for add or end interval workout messages.
     *                     Valid values are 0x0001 to 0x0E10 (and 0xFFFF for end interval workout).
     */
    public ConfigureWorkoutMessage(MessageType messageType, int distance, WorkoutUnit unit, int restInterval) {
        this.messageType = requireNonNull(messageType);
        this.distance = checkDistance(messageType, distance, unit);
        this.unit = requireNonNull(unit);
        this.restInterval = checkRestInterval(messageType, restInterval);
    }

    private int checkRestInterval(MessageType messageType, int restInterval) {
        switch(messageType) {
            case SINGLE_WORKOUT:
            case START_INTERVAL_WORKOUT:
                // Ignore rest interval, because SINGLE_WORKOUT or START_INTERVAL_WORKOUT does
                // not have a rest interval.
                break;
            case ADD_INTERVAL_WORKOUT:
                if ((restInterval < 0x0001) || (restInterval > 0x0E10))
                    throw new IllegalArgumentException("The rest interval must be between 0x0001 and 0x0E10!");
                break;
            case END_INTERVAL_WORKOUT:
                if (restInterval != 0xFFFF)
                    throw new IllegalArgumentException("The rest interval for an END_INTERVAL_WORKOUT message must be 0xFFFF!");
                break;
        }
        return restInterval;
    }

    /* Check if distance is in range. */
    private int checkDistance(MessageType messageType, int distance, WorkoutUnit unit) {
        if ((messageType == END_INTERVAL_WORKOUT) && (distance == 0xFFFF))
            return distance;
        switch(unit) {
            case METERS:
            case MILES:
            case KMS:
                // When unit = METERS, MILES or KMS: this value is in Meters, the display value for
                // miles is a conversion and valid values are 0x0001 to 0xFA00.
                if ((distance < 0x0001) || (distance > 0xFA00))
                    throw new IllegalArgumentException("The distance of the workout must be between 0x0001 and 0xFA00!");
                break;
            case STROKES:
                // When unit = STROKES this value is the number of strokes and valid values are
                // 0x0001 to 0x1388.
                if ((distance < 0x0001) || (distance > 0x1388))
                    throw new IllegalArgumentException("The distance of the workout must be between 0x0001 and 0x1388!");
                break;
            case SECONDS:
                // When unit = SECONDS this value is in seconds. Valid values are 0x0001 to 0x4650. This value is limited
                // to 5 Hours, which is 18,000 seconds.
                if ((distance < 0x0001) || (distance > 0x4650))
                    throw new IllegalArgumentException("The duration of the workout must be between 0x0001 and 0x4650!");
                break;
        }
        return distance;
    }


    /**
     * Returns the message type.
     *
     * @return The message type.
     */
    public MessageType getMessageType() {
        return messageType;
    }


    /**
     * Returns the distance (in meters/strokes) or duration (in seconds) of the workout or the workout interval. When
     * unit = METERS, MILES or KMS: this value is in Meters, the display value for miles is a conversion
     * and valid values are 0x0001 to 0xFA00. When unit = STROKES this value is the number of strokes and valid values are
     * 0x0001 to 0x1388. When unit = SECONDS this value is in seconds. Valid values are 0x0001 to 0x4650. This value is
     * limited to 5 Hours, which is 18,000 seconds.
     *
     * @return The distance (in meters/strokes) or duration (in seconds).
     */
    public int getDistance() {
        return distance;
    }


    /**
     * The unit of the workout or workout interval (e.g. meters, seconds).
     *
     * @return The unit of the workout.
     */
    public WorkoutUnit getWorkoutUnit() {
        return unit;
    }


    /**
     * Returns the rest interval (in seconds). The rest interval is only used for add or end interval workout
     * messages. Valid values are 0x0001 to 0x0E10 (and 0xFFFF for end interval workout).
     *
     * @return The rest interval (in seconds).
     */
    public int getRestInterval() {
        return restInterval;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("messageType", messageType)
                .add("distance", distance)
                .add("unit", unit)
                .add("restInterval", restInterval)
                .toString();
    }

}
