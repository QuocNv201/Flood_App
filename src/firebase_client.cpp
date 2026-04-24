#include "firebase_client.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// ---------------------------------------------------------------------------
// Compile-time credentials (override via platformio.ini build_flags)
// ---------------------------------------------------------------------------
#ifndef WIFI_SSID
  #define WIFI_SSID "YourWiFiSSID"
#endif
#ifndef WIFI_PASS
  #define WIFI_PASS "YourWiFiPassword"
#endif
#ifndef FIREBASE_HOST
  #define FIREBASE_HOST "https://your-project-default-rtdb.firebaseio.com/"
#endif
#ifndef FIREBASE_KEY
  #define FIREBASE_KEY "your_database_secret_or_legacy_token"
#endif

// ---------------------------------------------------------------------------
// Firebase DB paths
// ---------------------------------------------------------------------------
#define DB_SENSORS "/floodapp/sensors"
#define DB_CONTROL "/floodapp/control"

// ---------------------------------------------------------------------------
// Firebase objects
// ---------------------------------------------------------------------------
static FirebaseData fbdoWrite;   // used for sensor pushes
static FirebaseData fbdoStream;  // used for command stream
static FirebaseAuth auth;
static FirebaseConfig config;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
static bool     fbReady            = false;
static unsigned long lastSensorMs  = 0;
static const unsigned long SENSOR_INTERVAL_MS = 2000UL;

// Pending commands from the Android app (updated via stream callback)
static volatile int  pendingServoTarget = -1;
static volatile bool pendingResetAlert  = false;

// ---------------------------------------------------------------------------
// Stream callbacks (called from Firebase background task)
// ---------------------------------------------------------------------------
static void streamCallback(FirebaseStream data) {
    if (!data.streamAvailable()) return;

    String path = data.dataPath();

    if (path == "/servo_target") {
        if (data.dataType() == "int" || data.dataType() == "number") {
            int v = data.intData();
            if (v >= 0 && v <= 180) {
                pendingServoTarget = v;
                Serial.printf("[Firebase] Servo command received: %d°\n", v);
            }
        }
    } else if (path == "/reset_alert") {
        if (data.dataType() == "boolean" && data.boolData()) {
            pendingResetAlert = true;
            Serial.println("[Firebase] Reset alert command received");
        }
    }
}

static void streamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("[Firebase] Stream timeout – will reconnect");
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void firebase_init() {
    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("[Firebase] Connecting to WiFi: %s", WIFI_SSID);

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000UL) {
        delay(500);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Firebase] WiFi connection failed – Firebase disabled");
        return;
    }
    Serial.printf("[Firebase] WiFi connected: %s\n",
                  WiFi.localIP().toString().c_str());

    // Configure Firebase
    config.database_url = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_KEY;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Start streaming the control node for real-time commands
    if (!Firebase.RTDB.beginStream(&fbdoStream, DB_CONTROL)) {
        Serial.printf("[Firebase] Stream begin failed: %s\n",
                      fbdoStream.errorReason().c_str());
        // Non-fatal: sensor writes still work
    }
    Firebase.RTDB.setStreamCallback(&fbdoStream, streamCallback,
                                    streamTimeoutCallback);

    fbReady = true;
    Serial.println("[Firebase] Ready");
}

bool firebase_is_ready() {
    return fbReady && Firebase.ready();
}

void firebase_update_sensors(float dist_cm, int light, bool alert,
                              int servo_angle, bool servo_paused) {
    if (!firebase_is_ready()) return;

    unsigned long now = millis();
    if (now - lastSensorMs < SENSOR_INTERVAL_MS) return;
    lastSensorMs = now;

    FirebaseJson json;
    json.set("distance_cm",  dist_cm);
    json.set("light_level",  light);
    json.set("alert",        alert);
    json.set("servo_angle",  servo_angle);
    json.set("servo_paused", servo_paused);
    json.set("last_updated/.sv", "timestamp");  // server-side timestamp

    if (!Firebase.RTDB.setJSON(&fbdoWrite, DB_SENSORS, &json)) {
        Serial.printf("[Firebase] Sensor write error: %s\n",
                      fbdoWrite.errorReason().c_str());
    }
}

int firebase_get_servo_target() {
    return pendingServoTarget;
}

bool firebase_get_reset_command() {
    return pendingResetAlert;
}

void firebase_clear_servo_command() {
    pendingServoTarget = -1;
    if (firebase_is_ready()) {
        Firebase.RTDB.setInt(&fbdoWrite, DB_CONTROL "/servo_target", -1);
    }
}

void firebase_clear_reset_command() {
    pendingResetAlert = false;
    if (firebase_is_ready()) {
        Firebase.RTDB.setBool(&fbdoWrite, DB_CONTROL "/reset_alert", false);
    }
}
