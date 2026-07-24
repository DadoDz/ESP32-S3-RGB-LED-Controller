/*
 * Provides functions to initialize and control WS2812 RGB LEDs
 * using the ESP32-S3 RMT peripheral
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "led_strip.h"
#include "esp_err.h"

/**
 * @brief LED data structure mirroring Firebase JSON schema
 */
typedef struct {
    bool power;         // < LED on/off state
    int red;            // < Red color intensity (0-255)
    int green;          // < Green color intensity (0-255)
    int blue;           // < Blue color intensity (0-255)
    int brightness;     // < Global brightness, 0-100 (%)
} led_data_t;

/**
 * @brief Initialize the LED strip hardware
 * 
 * @param gpio_num GPIO pin for data line
 * @param num_leds Number of LEDs in the strip
 * @return ESP_OK on success, error code on failure
 */
esp_err_t led_init(int gpio_num, int num_leds);

/**
 * @brief Update LED color based on current led_data state
 * 
 * @param data Pointer to led_data_t structure
 */
void led_update(const led_data_t *data);

/**
 * @brief Set LED to specific RGB color with brightness
 * 
 * @param r Red value (0-255)
 * @param g Green value (0-255)
 * @param b Blue value (0-255)
 * @param brightness Multiplier (0.0-1.0)
 */
void led_set_color(int r, int g, int b, float brightness);

/**
 * @brief Turn LED off
 */
void led_off(void);

/**
 * @brief Get handle to LED strip (for direct access)
 * 
 * @return led_strip_handle_t Handle or NULL if not initialized
 */
led_strip_handle_t led_get_handle(void);

#endif /* LED_CONTROLLER_H */