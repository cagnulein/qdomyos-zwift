package com.nettarion.hyperborea.hardware.fitpro.v2

enum class V2FeatureId(val code: Int) {
    SYSTEM_MODE(102),
    IDLE_SYSTEM_MODE_LOCK(103),
    HEART_BEAT_INTERVAL(161),
    CURRENT_CALORIES(202),
    PULSE(222),
    DISTANCE(252),
    TARGET_KPH(301),
    CURRENT_KPH(302),
    RPM(322),
    TARGET_GRADE(401),
    CURRENT_GRADE(402),
    TARGET_RESISTANCE(503),
    MAX_RESISTANCE(504),
    WATTS(522),
    GOAL_WATTS(523),
    /** The console workout state — its value is a [V2WorkoutMode] ordinal. This is the one to drive for start/pause/resume/stop. */
    WORKOUT_STATE(602),
    RUNNING_TIME(604),
    ;

    val wireLo: Byte get() = (code and 0xFF).toByte()
    val wireHi: Byte get() = ((code shr 8) and 0xFF).toByte()

    companion object {
        private val byCode = entries.associateBy { it.code }

        fun fromCode(code: Int): V2FeatureId? = byCode[code]

        fun fromWireBytes(lo: Byte, hi: Byte): V2FeatureId? {
            val code = (lo.toInt() and 0xFF) or ((hi.toInt() and 0xFF) shl 8)
            return fromCode(code)
        }

        val subscribable: List<V2FeatureId> = listOf(
            SYSTEM_MODE, WORKOUT_STATE, CURRENT_CALORIES, PULSE, DISTANCE, CURRENT_KPH, RPM,
            CURRENT_GRADE, TARGET_RESISTANCE, MAX_RESISTANCE, WATTS, RUNNING_TIME,
        )
    }
}
