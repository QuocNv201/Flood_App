package com.quocnv.floodapp.repository

import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener
import com.google.firebase.database.ktx.database
import com.google.firebase.ktx.Firebase
import com.quocnv.floodapp.model.SensorData
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.tasks.await

/**
 * Single source of truth for all Firebase Realtime Database operations.
 *
 * DB schema (root node: `/floodapp`):
 * ```
 * floodapp/
 *   sensors/
 *     distance_cm:  Double  – ultrasonic reading in cm
 *     light_level:  Int     – LDR ADC value (0–4095)
 *     alert:        Boolean – true = barrier triggered
 *     servo_angle:  Int     – current servo angle 0–180°
 *     servo_paused: Boolean – true = barrier closed
 *     last_updated: Long    – server-side timestamp (ms)
 *   control/
 *     servo_target: Int     – -1 = no-op; 0–180 = move servo
 *     reset_alert:  Boolean – true = clear alert on firmware
 * ```
 */
class FirebaseRepository {

    private val db: FirebaseDatabase = Firebase.database
    private val sensorsRef = db.getReference("floodapp/sensors")
    private val controlRef = db.getReference("floodapp/control")

    // -------------------------------------------------------------------------
    // Sensor data stream
    // -------------------------------------------------------------------------

    /**
     * Returns a cold [Flow] that emits a new [SensorData] every time the
     * `/floodapp/sensors` node changes.  The flow completes when the
     * collector is cancelled (e.g. when the ViewModel is cleared).
     */
    fun sensorDataFlow(): Flow<SensorData> = callbackFlow {
        val listener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val data = SensorData(
                    distanceCm  = snapshot.child("distance_cm").getValue(Double::class.java) ?: 0.0,
                    lightLevel  = snapshot.child("light_level").getValue(Int::class.java) ?: 0,
                    alert       = snapshot.child("alert").getValue(Boolean::class.java) ?: false,
                    servoAngle  = snapshot.child("servo_angle").getValue(Int::class.java) ?: 0,
                    servoPaused = snapshot.child("servo_paused").getValue(Boolean::class.java) ?: false,
                    lastUpdated = snapshot.child("last_updated").getValue(Long::class.java) ?: 0L
                )
                trySend(data)
            }

            override fun onCancelled(error: DatabaseError) {
                close(error.toException())
            }
        }

        sensorsRef.addValueEventListener(listener)
        // When the flow is cancelled, remove the Firebase listener to avoid leaks
        awaitClose { sensorsRef.removeEventListener(listener) }
    }

    // -------------------------------------------------------------------------
    // Control commands
    // -------------------------------------------------------------------------

    /**
     * Writes a servo target angle to `/floodapp/control/servo_target`.
     * The ESP32 firmware picks this up, moves the servo, then resets the
     * value to -1.
     *
     * @param angle Target angle in degrees (0–180).
     * @throws IllegalArgumentException if [angle] is outside [0, 180].
     */
    suspend fun sendServoCommand(angle: Int) {
        require(angle in 0..180) { "Servo angle must be between 0 and 180 degrees" }
        controlRef.child("servo_target").setValue(angle).await()
    }

    /**
     * Sets `/floodapp/control/reset_alert` to true.
     * The ESP32 firmware will clear the active alert and reset the value to
     * false once acknowledged.
     */
    suspend fun sendResetAlert() {
        controlRef.child("reset_alert").setValue(true).await()
    }

    /**
     * Reads the current control node once (non-streaming).
     * Useful to check whether a previous command is still pending.
     */
    suspend fun isServoCmdPending(): Boolean {
        val snapshot = controlRef.child("servo_target").get().await()
        return (snapshot.getValue(Int::class.java) ?: -1) >= 0
    }
}
