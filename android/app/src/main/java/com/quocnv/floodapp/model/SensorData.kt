package com.quocnv.floodapp.model

/**
 * Snapshot of sensor data stored under `/floodapp/sensors` in Firebase
 * Realtime Database.
 *
 * Firebase deserialisation requires a no-argument constructor; all fields
 * therefore have default values.
 *
 * @property distanceCm  Latest ultrasonic reading in centimetres.
 * @property lightLevel  LDR ADC value (0–4095). Higher = brighter.
 * @property alert       true when the flood barrier has been triggered
 *                       (obstacle within OBSTACLE_NEAR_CM).
 * @property servoAngle  Current servo angle in degrees (0–180).
 * @property servoPaused true = barrier is closed (servo at END_ANGLE);
 *                       false = barrier is open (servo at HOME_ANGLE).
 * @property lastUpdated Server-side timestamp in milliseconds (epoch).
 */
data class SensorData(
    val distanceCm: Double = 0.0,
    val lightLevel: Int = 0,
    val alert: Boolean = false,
    val servoAngle: Int = 0,
    val servoPaused: Boolean = false,
    val lastUpdated: Long = 0L
) {
    /**
     * Convenience: whether the distance indicates a safe condition.
     * Mirrors the firmware OBSTACLE_CLEAR_CM = 30 threshold.
     */
    val isSafe: Boolean get() = !alert && distanceCm > 30.0

    /**
     * Human-readable barrier status string.
     */
    val barrierStatus: String
        get() = when {
            servoPaused -> "CLOSED"
            else        -> "OPEN"
        }
}
