package de.tbressler.waterrower.model;

import static de.tbressler.waterrower.utils.MessageUtils.getBooleanFromByte;

/**
 * Working and workout control flags.
 * 0 = fzone_hr: working in heartrate zone
 * 1 = fzone_int: working in intensity zone
 * 2 = fzone_sr: working in strokerate zone
 * 3 = fprognostics: prognostics active
 * 4 = fworkout_dis: workout distance mode
 * 5 = fworkout_dur: workout duration mode
 * 6 = fworkout_dis_i: workout distance interval mode
 * 7 = fworkout_dur_i: workout duration interval mode
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class WorkoutFlags {

    /* The flag values as byte. */
    private final int value;


    /**
     * Working and workout control flags.
     *
     * @param value The flags as byte.
     */
    public WorkoutFlags(int value) {
        if ((value < 0x00) || (value > 0xFF))
            throw new IllegalArgumentException("Value must be in range 0x00 to 0xFF!");
        this.value = value;
    }


    /**
     * True if working in heartrate zone.
     *
     * @return True if working in heartrate zone.
     */
    public boolean isWorkingInHeartRateZone() {
        return getBooleanFromByte(value, 0);
    }

    /**
     * True if working in intensity zone.
     *
     * @return True if working in intensity zone.
     */
    public boolean isWorkingInIntensityZone() {
        return getBooleanFromByte(value, 1);
    }

    /**
     * True if working in strokerate zone.
     *
     * @return True if working in strokerate zone.
     */
    public boolean isWorkingInStrokeRateZone() {
        return getBooleanFromByte(value, 2);
    }

    /**
     * True if prognostics active.
     *
     * @return True if prognostics active.
     */
    public boolean isPrognosticsActive() {
        return getBooleanFromByte(value, 3);
    }

    /**
     * True if in workout distance mode.
     *
      * @return True if in workout distance mode.
     */
    public boolean isWorkoutDistanceMode() {
        return getBooleanFromByte(value, 4);
    }

    /**
     * True if in workout duration mode.
     *
     * @return True if in workout duration mode.
     */
    public boolean isWorkoutDurationMode() {
        return getBooleanFromByte(value, 5);
    }

    /**
     * True if in workout distance interval mode.
     *
     * @return True if in workout distance interval mode.
     */
    public boolean isWorkoutDistanceIntervalMode() {
        return getBooleanFromByte(value, 6);
    }

    /**
     * True if in workout duration interval mode.
     *
     * @return True if in workout duration interval mode.
     */
    public boolean isWorkoutDurationIntervalMode() {
        return getBooleanFromByte(value, 7);
    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        WorkoutFlags that = (WorkoutFlags) o;

        return value == that.value;
    }

    @Override
    public int hashCode() {
        return value;
    }

}
