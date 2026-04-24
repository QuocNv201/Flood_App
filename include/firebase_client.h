#pragma once
#include <Arduino.h>

/**
 * firebase_client.h
 *
 * ESP32 Firebase Realtime Database client for Flood_App.
 *
 * Firebase DB schema:
 *   /floodapp/sensors/
 *     distance_cm  – float  (latest ultrasonic reading in cm)
 *     light_level  – int    (LDR ADC value 0-4095)
 *     alert        – bool   (true = barrier triggered / obstacle near)
 *     servo_angle  – int    (current servo angle 0-180°)
 *     servo_paused – bool   (true = barrier closed, false = open)
 *     last_updated – long   (server-side timestamp ms, set via ".sv":"timestamp")
 *
 *   /floodapp/control/
 *     servo_target – int   (-1 = no command; 0-180 = move servo to this angle)
 *     reset_alert  – bool  (true = acknowledge and clear active alert)
 *
 * Credentials / connectivity are supplied at compile time via build_flags:
 *   -D WIFI_SSID=\"YourSSID\"
 *   -D WIFI_PASS=\"YourPassword\"
 *   -D FIREBASE_HOST=\"https://your-project-default-rtdb.firebaseio.com/\"
 *   -D FIREBASE_KEY=\"your_database_secret_or_legacy_token\"
 */

/**
 * Connect to WiFi and initialise the Firebase client + control stream.
 * Should be called once from setup() after all hardware modules are ready.
 */
void firebase_init();

/**
 * Returns true once WiFi is connected and the Firebase library is ready.
 */
bool firebase_is_ready();

/**
 * Push the latest sensor snapshot to Firebase.
 * Internally rate-limited to one write every 2 s so it can be called every loop.
 *
 * @param dist_cm     Ultrasonic distance in centimetres (> 0).
 * @param light       LDR ADC value (0-4095).
 * @param alert       true when barrier is triggered.
 * @param servo_angle Current servo angle (0-180°).
 * @param servo_paused true = barrier is closed.
 */
void firebase_update_sensors(float dist_cm, int light, bool alert,
                              int servo_angle, bool servo_paused);

/**
 * Returns the pending servo target received from the Android app.
 * @return 0-180 if a command is waiting, -1 if none.
 */
int firebase_get_servo_target();

/**
 * Returns true if the Android app has requested an alert reset.
 */
bool firebase_get_reset_command();

/**
 * Acknowledge the servo command: clears local state and writes -1 back to DB
 * so the app knows the command has been consumed.
 */
void firebase_clear_servo_command();

/**
 * Acknowledge the reset command: clears local state and writes false back to DB.
 */
void firebase_clear_reset_command();
