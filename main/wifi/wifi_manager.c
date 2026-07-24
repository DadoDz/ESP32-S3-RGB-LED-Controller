/*
 * WiFi manager implementation
 */

#include "wifi_manager.h"
#include "project_config.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "WIFI";
static int retry_count = 0;
static EventGroupHandle_t wifi_event_group = NULL;

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_LOGI(TAG, "Connecting to Wi-Fi...");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (retry_count < WIFI_MAX_RETRY) 
        {
            retry_count++;
            ESP_LOGI(TAG, "Retry %d/%d connecting to Wi-Fi...", retry_count, WIFI_MAX_RETRY);
            esp_wifi_connect();
        } 
        else 
        {
            ESP_LOGE(TAG, "Wi-Fi connection failed after %d attempts", WIFI_MAX_RETRY);
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Wi-Fi connected! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        
        retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

EventGroupHandle_t wifi_init_sta(void)
{
    /* Create event group for WiFi status */
    wifi_event_group = xEventGroupCreate();

    /* Initialize TCP/IP and event loop */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    /* Initialize WiFi driver */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Register event handlers */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    /* Configure WiFi */
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    /* Apply configuration and start */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "Wi-Fi initialization complete");
    return wifi_event_group;
}

bool wifi_wait_connection(EventGroupHandle_t event_group, int timeout_ms)
{
    EventBits_t bits = xEventGroupWaitBits(event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(timeout_ms));
    return (bits & WIFI_CONNECTED_BIT) != 0;
}