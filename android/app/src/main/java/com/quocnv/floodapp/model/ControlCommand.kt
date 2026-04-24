package com.quocnv.floodapp.model

/**
 * A control command written to `/floodapp/control` in Firebase Realtime
 * Database.  The ESP32 firmware polls this node and executes the command,
 * then clears it back to the default "no-op" values.
 *
 * @property servoTarget Target servo angle (0–180°).
 *                       The firmware ignores values outside [0, 180].
 *                       Set to -1 to represent "no command pending".
 * @property resetAlert  When true the ESP32 acknowledges and clears the
 *                       active alert (even if water is still nearby).
 */
data class ControlCommand(
    val servoTarget: Int = -1,
    val resetAlert: Boolean = false
)
