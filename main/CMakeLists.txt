idf_component_register(
    SRCS
        "main.c"
        "wifi/wifi_manager.c"
        "firebase/firebase_client.c"
        "led/led_controller.c"

    INCLUDE_DIRS
        "."
        "wifi"
        "firebase"
        "led"
        "config"

    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        esp_http_client
        esp-tls
        led_strip
        json
)