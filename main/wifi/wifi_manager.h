/*
 * Handles WiFi station mode connection, reconnection, and event management
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_event.h"
#include "freertos/event_groups.h"

/**
 * @brief Initialize WiFi in station mode
 * 
 * @return EventGroupHandle_t Event group for monitoring WiFi status
 */
EventGroupHandle_t wifi_init_sta(void);

/**
 * @brief Get WiFi connection status
 * 
 * @param event_group Event group from wifi_init_sta()
 * @param timeout_ms Timeout in milliseconds (portMAX_DELAY for infinite)
 * @return true if connected successfully, false otherwise
 */
bool wifi_wait_connection(EventGroupHandle_t event_group, int timeout_ms);

/**
 * @brief Get WiFi event handler (for registration)
 * 
 * @return esp_event_handler_t Event handler function
 */
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

#endif /* WIFI_MANAGER_H */