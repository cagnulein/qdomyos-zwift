package de.tbressler.waterrower.model;

/**
 * Memory locations of the WaterRower S4, version 2.00.
 * See PDF file 'docs/Water_Rower_S4_S5_USB_Protocol_Iss_1.04.pdf' for more details.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public enum MemoryLocation {

    /*
     * Flags:
     *
     * These registers can be read to determine the current state of the display; they are formed of
     * 8 separate true or false flags. These flags are defined at the end of the memory map.
     */

    FEXTENDED(0x03e),           // working and workout control flags
                                // bits for extended zones and workout modes:
                                //   0 = fzone_hr fextended; working in heartrate zone
                                //   1 = fzone_int fextended; working in intensity zone
                                //   2 = fzone_sr fextended; working in strokerate zone
                                //   3 = fprognostics fextended; prognostics active.
                                //   4 = fworkout_dis fextended; workout distance mode
                                //   5 = fworkout_dur fextended; workout duration mode
                                //   6 = fworkout_dis_i fextended; workout distance interval mode
                                //   7 = fworkout_dur_i fextended; workout duration interval mode

    FMISC_FLAGS(0x049),         // zone words and misc windows flags
                                //   0 = fzone_fg_work fmisc_flags; set when to turn on or if flashed is clear and flash is set
                                //   1 = fzone_fg_rest fmisc_flags; set when to turn on or if flashed is clear and flash is set
                                //   2 = fmisc_fg_lowbat fmisc_flags; set when to turn on or if flashed is clear and flash is set
                                //   3 = fmisc_fg_pc fmisc_flags; set when to turn on or if flashed is clear and flash is set
                                //   4 = fmisc_fg_line fmisc_flags; set when to turn on or if flashed is clear and flash is set
                                //   5 = fmisc_fg_mmc_cd fmisc_flags; set when to turn on or if flashed is clear and flash is set
                                //   6 = fmisc_fg_mmc_up fmisc_flags; set when to turn on or if flashed is clear and flash is set
                                //   7 = fmisc_fg_mmc_dn fmisc_flags; set when to turn on or if flashed is clear and flash is set

    /*
     * Variables:
     *
     * The following memory locations are available to the user for reading. Other locations not specified are
     * unavailable for reading. A lot of the timers count 1bit per 25mS of actual time, remember this for ALL maths,
     * otherwise things can be confusing of how much time say a stroke was done IN.
     */

    /* Distance variables: */

    MS_DISTANCE_DEC(0x054),     // 0.1m count (only counts up from 0-9
    MS_DISTANCE_LOW(0x055),     // low byte of meters
    MS_DISTANCE_HI(0x056),      // hi byte of meters and km (65535meters max)

    /* This is the displayed distance: */

    DISTANCE_LOW(0x057),        // low byte of meters
    DISTANCE_HI(0x058),         // hi byte of meters and km (65535meters max)

    /* Clock count down, this is 16bit value: */

    CLOCK_DOWN_DEC(0x05a),      // seconds 0.9-0.0
    CLOCK_DOWN_LOW(0x05b),      // low byte clock count down
    CLOCK_DOWN_HI(0x05c),       // hi byte clock count down


    KCAL_WATTS_LOW(0x088),
    KCAL_WATTS_HI(0x089),
    TOTAL_KCAL_LOW(0x08a),
    TOTAL_KCAL_HI(0x08b),
    TOTAL_KCAL_UP(0x08c),


    /* Total distance meter counter - this is stored at switch off: */

    TOTAL_DIS_DEC(0x080),       // dec byte of meters
    TOTAL_DIS_LOW(0x081),       // low byte of meters
    TOTAL_DIS_HI(0x082),        // hi byte of meters and km (65535meters max)

    /* Tank volume in liters: */

    TANK_VOLUME(0x0a9),         // volume of water in tank

    /* Stroke counter: */

    STROKES_CNT_LOW(0x140),     // low byte count
    STROKES_CNT_HI(0x141),      // high byte count
    STROKE_AVERAGE(0x142),      // average time for a whole stroke
    STROKE_PULL(0x143),         // average time for a pull (acc to dec)

    // Stroke_pull is first subtracted from stroke_average then a modifier of 1.25 multiplied
    // by the result to generate the ratio value for display.

    /* Meters per second registers: */

    M_S_LOW_TOTAL(0x148),       // total distance per second in cm low byte
    M_S_HI_TOTAL(0x149),        // total distance per second in cm hi byte
    M_S_LOW_AVERAGE(0x14a),     // instant average distance in cm low byte
    M_S_HI_AVERAGE(0x14b),      // instant average distance in cm hi byte
    M_S_STORED(0x14c),          // no. of the stored values.
    M_S_PROJL_AVG(0x14d),       // all average for projected distance/duration maths
    M_S_PROJH_AVG(0x14e),       // all average for projected distance/duration maths


    /* stored values for the zone maths (these are pre display values): */

    ZONE_HR_VAL(0x1a0),         // heart rate stored value
    // ...

    /* Used to generate the display clock: */

    DISPLAY_SEC_DEC(0x1e0),     // seconds 0.0-0.9
    DISPLAY_SEC(0x1e1),         // seconds 0-59 (send as ACH not byte value)
    DISPLAY_MIN(0x1e2),         // minutes 0-59 (send as ACH not byte value)
    DISPLAY_HR(0x1e3),          // hours 0-9 only (send as ACH not byte value)

    /* Workout total times/distances/limits: */

    WORKOUT_TIMEL(0x1e8),       // total workout time
    WORKOUT_TIMEH(0x1e9),
    WORKOUT_MS_L(0x1ea),        // total workout m/s
    WORKOUT_MS_H(0x1eb),
    WORKOUT_STROKEL(0x1ec),     // total workout strokes
    WORKOUT_STROKEH(0x1ed),
    WORKOUT_LIMIT_L(0x1ee),     // this is the limit value for workouts
    WORKOUT_LIMIT_H(0x1ef),

    /*
     * Interval's:
     *
     * These are the interval timing's in use or being programmed.
     */

    WORKOUT_WORK1_L(0x1b0),
    WORKOUT_WORK1_H(0x1b1),
    WORKOUT_REST1_L(0x1b2),
    WORKOUT_REST1_H(0x1b3),

    WORKOUT_WORK2_L(0x1b4),
    WORKOUT_WORK2_H(0x1b5),
    WORKOUT_REST2_L(0x1b6),
    WORKOUT_REST2_H(0x1b7),

    WORKOUT_WORK3_L(0x1b8),
    WORKOUT_WORK3_H(0x1b9),
    WORKOUT_REST3_L(0x1ba),
    WORKOUT_REST3_H(0x1bb),

    WORKOUT_WORK4_L(0x1bc),
    WORKOUT_WORK4_H(0x1bd),
    WORKOUT_REST4_L(0x1be),
    WORKOUT_REST4_H(0x1bf),

    WORKOUT_WORK5_L(0x1c0),
    WORKOUT_WORK5_H(0x1c1),
    WORKOUT_REST5_L(0x1c2),
    WORKOUT_REST5_H(0x1c3),

    WORKOUT_WORK6_L(0x1c4),
    WORKOUT_WORK6_H(0x1c5),
    WORKOUT_REST6_L(0x1c6),
    WORKOUT_REST6_H(0x1c7),

    WORKOUT_WORK7_L(0x1c8),
    WORKOUT_WORK7_H(0x1c9),
    WORKOUT_REST7_L(0x1ca),
    WORKOUT_REST7_H(0x1cb),

    WORKOUT_WORK8_L(0x1cc),
    WORKOUT_WORK8_H(0x1cd),
    WORKOUT_REST8_L(0x1ce),
    WORKOUT_REST8_H(0x1cf),

    WORKOUT_WORK9_L(0x1d0),
    WORKOUT_WORK9_H(0x1d1),

    WORKOUT_INTER(0x1d9);       // No work workout intervals


    /* The memory location as decimal. */
    private final int location;

    /**
     * Constructor of the enum.
     *
     * @param location The memory location as decimal (0x000 .. 0xFFF).
     */
    MemoryLocation(int location) {
        if ((location < 0x000) || (location > 0xFFF))
            throw new IllegalArgumentException("Invalid memory location! Location must be between 0x000 and 0xFFF.");
        this.location = location;
    }

    /**
     * Returns the memory location as decimal.
     *
     * @return The memory location.
     */
    public int getLocation() {
        return location;
    }

}
