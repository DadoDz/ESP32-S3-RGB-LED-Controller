/*
 * ESP32-S3 RGB LED Controller - Main Entry Point
 */

#include <stdio.h>
#include <string.h>

/* FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* ESP-IDF Core */
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

/* Project Modules */
#include "project_config.h"
#include "led_controller.h"
#include "wifi_manager.h"
#include "firebase_client.h"

/* ============================================================================
 * STATIC VARIABLES
 * ============================================================================ */

static const char *TAG = "MAIN";
static led_data_t led_data = {0};  // Current LED state
static EventGroupHandle_t wifi_event_group = NULL;

/* ============================================================================
 * FREERTOS TASKS
 * ============================================================================ */

/**
 * @brief Firebase polling task
 * 
 * Periodically fetches LED state from Firebase and updates led_data structure
 * 
 * @param pvParameters Unused task parameter
 */
static void firebase_task(void *pvParameters)
{
    ESP_LOGI(FIREBASE_TAG, "Firebase task started on Core %d", xPortGetCoreID());

    while (1) 
    {
        // Fetch latest LED state from Firebase
        esp_err_t err = firebase_fetch_led_data(&led_data);
        if (err != ESP_OK) 
            ESP_LOGW(FIREBASE_TAG, "Firebase request failed, will retry...");

        // Wait before next poll
        vTaskDelay(pdMS_TO_TICKS(FIREBASE_POLL_INTERVAL));
    }
}

/**
 * @brief LED control task
 * 
 * Reads the led_data structure and updates the physical LED.
 * Runs at 60Hz for smooth visual updates.
 * 
 * @param pvParameters Unused task parameter
 */
static void led_task(void *pvParameters)
{
    ESP_LOGI(TAG, "LED task started on Core %d", xPortGetCoreID());

    while (1) 
    {
        // Update LED based on current data
        led_update(&led_data);

        // 60Hz refresh rate (16.67ms) for smooth updates
        vTaskDelay(pdMS_TO_TICKS(LED_REFRESH_RATE_MS));
    }
}

/* ============================================================================
 * MAIN APPLICATION
 * ============================================================================ */

/**
 * @brief Main application entry point
 * 
 * Application flow:
 * 1. Initialize NVS (non-volatile storage)
 * 2. Initialize LED hardware
 * 3. Initialize and connect to Wi-Fi
 * 4. Start Firebase and LED tasks
 */
void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32-S3 RGB LED Controller Starting ===");

    /* ======================================================================
     * STEP 1: Initialize Non-Volatile Storage (NVS)
     * ====================================================================== */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_LOGW(TAG, "NVS partition corrupted - erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized successfully");

    /* ======================================================================
     * STEP 2: Initialize LED Hardware
     * ====================================================================== */
    ret = led_init(LED_GPIO, LED_NUMBER);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "LED initialization failed!");
        return;
    }
    ESP_LOGI(TAG, "LED initialized on GPIO %d", LED_GPIO);

    /* ======================================================================
     * STEP 3: Initialize and Start Wi-Fi
     * ====================================================================== */
    wifi_event_group = wifi_init_sta();
    if (wifi_event_group == NULL) 
    {
        ESP_LOGE(TAG, "Wi-Fi initialization failed!");
        return;
    }

    /* ======================================================================
     * STEP 4: Wait for Wi-Fi Connection
     * ====================================================================== */
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    bool connected = wifi_wait_connection(wifi_event_group, portMAX_DELAY);
    
    if (!connected) 
    {
        ESP_LOGE(TAG, "Wi-Fi connection failed - restarting in 5 seconds");
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
        return;
    }

    ESP_LOGI(TAG, "Wi-Fi connected successfully!");

    /* ======================================================================
     * STEP 5: Initialize Firebase Client
     * ====================================================================== */
    ret = firebase_init();
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "Firebase initialization failed!");
        return;
    }

    /* ======================================================================
     * STEP 6: Create Application Tasks
     * ====================================================================== */
    ESP_LOGI(TAG, "Creating application tasks...");

    /* Firebase polling task */
    xTaskCreate(firebase_task, "firebase_task", TASK_FIREBASE_STACK, NULL, TASK_FIREBASE_PRIORITY, NULL);

    /* LED control task */
    xTaskCreate(led_task, "led_task", TASK_LED_STACK, NULL, TASK_LED_PRIORITY, NULL);

    ESP_LOGI(TAG, "All tasks started successfully!");
    ESP_LOGI(TAG, "=== System Ready ===");

    /* ======================================================================
     * STEP 7: Idle Loop (Should never reach here)
     * ====================================================================== */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}