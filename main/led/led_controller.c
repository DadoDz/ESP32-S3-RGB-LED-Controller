/*
 * WS2812 LED controller implementation
 */

#include "led_controller.h"
#include "led_strip_rmt.h"
#include "esp_log.h"

static const char *TAG = "LED";
static led_strip_handle_t led_strip = NULL;

esp_err_t led_init(int gpio_num, int num_leds)
{
    ESP_LOGI(TAG, "Initializing LED on GPIO %d with %d LED(s)", gpio_num, num_leds);

    /* Configure LED strip */
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_num,
        .max_leds = num_leds,
    };

    /* Configure RMT peripheral */
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  /* 10MHz */
        .flags.with_dma = false,
    };

    /* Create LED strip device */
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to initialize LED strip: %s", esp_err_to_name(err));
        return err;
    }

    /* Start with LED off */
    led_off();
    ESP_LOGI(TAG, "LED initialized successfully");
    
    return ESP_OK;
}

void led_update(const led_data_t *data)
{
    if (led_strip == NULL) 
    {
        ESP_LOGE(TAG, "LED not initialized!");
        return;
    }

    if (data->power) 
    {
        float brightness = (float)data->brightness / 100.0f;
        led_set_color(data->red, data->green, data->blue, brightness);
    } 
    else
        led_off();
}

void led_set_color(int r, int g, int b, float brightness)
{
    if (led_strip == NULL) 
        return;

    r = (int)(r * brightness);
    g = (int)(g * brightness);
    b = (int)(b * brightness);

    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

void led_off(void)
{
    if (led_strip == NULL) 
        return;

    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);
}

led_strip_handle_t led_get_handle(void) {
    return led_strip;
}