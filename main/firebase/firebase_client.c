/*
 * Firebase client implementation
 */

#include "firebase_client.h"
#include "project_config.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>

/**
 * @brief Parse JSON response from Firebase into a led_data_t
 *
 * @param json_string Raw JSON response
 * @param data Pointer to LED data structure to fill
 * @return ESP_OK on success, ESP_FAIL on parse error
 */
static esp_err_t parse_firebase_response(const char *json_string, led_data_t *data)
{
    cJSON *root = cJSON_Parse(json_string);
    if (root == NULL) 
    {
        ESP_LOGE(FIREBASE_TAG, "Failed to parse JSON: %s", json_string);
        return ESP_FAIL;
    }

    /* Extract fields */
    cJSON *power = cJSON_GetObjectItem(root, "power");
    cJSON *red = cJSON_GetObjectItem(root, "red");
    cJSON *green = cJSON_GetObjectItem(root, "green");
    cJSON *blue = cJSON_GetObjectItem(root, "blue");
    cJSON *brightness = cJSON_GetObjectItem(root, "brightness");

    /* Validate all fields exist and have correct types */
    if (!cJSON_IsBool(power) || !cJSON_IsNumber(red) || !cJSON_IsNumber(green) || !cJSON_IsNumber(blue) || !cJSON_IsNumber(brightness)) 
    {
        ESP_LOGE(FIREBASE_TAG, "Invalid LED data - missing or wrong type fields");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    /* Populate data structure. Red/green/blue are 0-255, brightness is a
     * plain 0-100 percentage (that's what the app's slider sends) - clamp
     * everything defensively in case a bad value ever lands in the DB. */
    data->power = cJSON_IsTrue(power);
    data->red = red->valueint < 0 ? 0 : (red->valueint > 255 ? 255 : red->valueint);
    data->green = green->valueint < 0 ? 0 : (green->valueint > 255 ? 255 : green->valueint);
    data->blue = blue->valueint < 0 ? 0 : (blue->valueint > 255 ? 255 : blue->valueint);
    data->brightness = brightness->valueint < 0 ? 0 : (brightness->valueint > 100 ? 100 : brightness->valueint);

    ESP_LOGI(FIREBASE_TAG, 
        "Power: %s | RGB: (%d, %d, %d) | Brightness: %d%%",
        data->power ? "ON" : "OFF",
        data->red, data->green, data->blue,
        data->brightness
    );

    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t firebase_init(void)
{
    ESP_LOGI(FIREBASE_TAG, "Firebase client initialized");
    return ESP_OK;
}

esp_err_t firebase_fetch_led_data(led_data_t *data)
{
    char buffer[2048] = {0};

    /* Configure HTTP client */
    esp_http_client_config_t config = {
        .url = FIREBASE_URL,
        .method = HTTP_METHOD_GET,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) 
    {
        ESP_LOGE(FIREBASE_TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }

    /* Open connection and send request */
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) 
    {
        ESP_LOGE(FIREBASE_TAG, "Failed to open connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    /* Fetch headers first - content_length and status code */
    int content_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);
    
    ESP_LOGI(FIREBASE_TAG, "HTTP Status: %d, Content-Length: %d", status_code, content_length);

    /* Check for HTTP errors - 2xx = success, 4xx/5xx = client/server error */
    if (status_code < 200 || status_code >= 300) {
        ESP_LOGE(FIREBASE_TAG, "HTTP error: %d", status_code);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    /* Read response body */
    int total_read = 0;
    while (total_read < sizeof(buffer) - 1) 
    {
        int read_len = esp_http_client_read(client, buffer + total_read, sizeof(buffer) - total_read - 1);
        if (read_len <= 0) break;
        total_read += read_len;
    }
    buffer[total_read] = '\0';

    ESP_LOGI(FIREBASE_TAG, "Response: %s", buffer);

    /* Parse and update LED data */
    err = parse_firebase_response(buffer, data);

    /* Cleanup */
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return err;
}