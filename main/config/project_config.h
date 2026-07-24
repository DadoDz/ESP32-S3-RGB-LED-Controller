/*
 * All configurable parameters live here for easy tweaking.
 * WiFi/Firebase secrets come from `idf.py menuconfig` -> "LED Controller Configuration" 
 * so nothing sensitive ends up committed to git (seesdkconfig.defaults / .gitignore).
 */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include "sdkconfig.h"

#define FIREBASE_TAG "FIREBASE"

/* ============================================================================
 * WI-FI CONFIGURATION
 * ============================================================================ */
#define WIFI_SSID               CONFIG_WIFI_SSID
#define WIFI_PASSWORD           CONFIG_WIFI_PASSWORD
#define WIFI_MAX_RETRY          5           // max reconnect attempts before giving up
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

/* ============================================================================
 * LED CONFIGURATION
 * ============================================================================ */
#define LED_GPIO                48          // onboard WS2812 data pin on most ESP32-S3 devkits
#define LED_NUMBER              1
#define LED_REFRESH_RATE_MS     17          // ~60Hz

/* ============================================================================
 * FIREBASE CONFIGURATION
 * ============================================================================ */
#define FIREBASE_URL            CONFIG_FIREBASE_DATABASE_URL
#define FIREBASE_POLL_INTERVAL  5000        // ms between polls

/* ============================================================================
 * TASK CONFIGURATION
 * ============================================================================ */
#define TASK_FIREBASE_STACK     16384       // TLS handshake needs a decent chunk of stack
#define TASK_LED_STACK          4096
#define TASK_FIREBASE_PRIORITY  5
#define TASK_LED_PRIORITY       5

#endif /* PROJECT_CONFIG_H */
