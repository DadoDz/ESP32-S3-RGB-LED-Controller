/*
 * Handles HTTPS requests to Firebase REST API and JSON parsing
 */

#ifndef FIREBASE_CLIENT_H
#define FIREBASE_CLIENT_H

#include "led_controller.h"
#include "esp_err.h"

/**
 * @brief Initialize Firebase client
 * 
 * @return ESP_OK on success
 */
esp_err_t firebase_init(void);

/**
 * @brief Fetch LED data from Firebase
 * 
 * Makes a GET request to Firebase and updates led_data
 * 
 * @param data Pointer to led_data_t to update
 * @return ESP_OK on success, error code on failure
 */
esp_err_t firebase_fetch_led_data(led_data_t *data);

#endif /* FIREBASE_CLIENT_H */