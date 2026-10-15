/* ESP-IDF Wi-Fi station adapter with event-driven reconnection. */
#include "network_if.h"

#include "app_config.h"
#include "logger.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "nvs_flash.h"

#include <stdbool.h>
#include <string.h>

#define NETWORK_CONNECTED_BIT (1U << 0)

static const char *TAG = "network_platform";
static EventGroupHandle_t connection_events;
static bool is_initialized;
static bool is_sntp_initialized;
static esp_event_handler_instance_t wifi_event_handler_instance;
static esp_event_handler_instance_t ip_event_handler_instance;

/* Start a driver-managed reconnect whenever the station loses association. */
static void network_event_handler(void *argument,
                                  esp_event_base_t event_base,
                                  int32_t event_id,
                                  void *event_data)
{
    (void)argument;
    (void)event_data;

    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        (void)xEventGroupClearBits(connection_events, NETWORK_CONNECTED_BIT);
        esp_err_t result = esp_wifi_connect();
        if ((result != ESP_OK) && (result != ESP_ERR_WIFI_CONN))
        {
            ESP_LOGW(TAG, "Wi-Fi reconnect request failed: %s", esp_err_to_name(result));
        }
        ESP_LOGW(TAG, "Wi-Fi disconnected; sensor acquisition continues");
    }
    else if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP))
    {
        (void)xEventGroupSetBits(connection_events, NETWORK_CONNECTED_BIT);
        ESP_LOGI(TAG, "Wi-Fi station acquired an IP address");
        if (!is_sntp_initialized)
        {
            esp_sntp_config_t time_config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
            esp_err_t time_init_result = esp_netif_sntp_init(&time_config);
            is_sntp_initialized = (time_init_result == ESP_OK);
            if (!is_sntp_initialized)
            {
                ESP_LOGW(TAG, "SNTP startup failed: %s", esp_err_to_name(time_init_result));
            }
        }
    }
    else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    {
        esp_err_t result = esp_wifi_connect();
        if ((result != ESP_OK) && (result != ESP_ERR_WIFI_CONN))
        {
            ESP_LOGW(TAG, "Initial Wi-Fi connection request failed: %s", esp_err_to_name(result));
        }
    }
    else
    {
        /* Unrelated radio and IP events do not change connection state. */
    }
}

bool network_if_start(void)
{
    esp_err_t result;

    if (is_initialized)
    {
        return true;
    }

    if ((strcmp(APP_WIFI_SSID, "SET_WIFI_SSID") == 0) ||
        (strcmp(APP_WIFI_PASSWORD, "SET_WIFI_PASSWORD") == 0))
    {
        ESP_LOGW(TAG, "Wi-Fi credentials are placeholders; continuing offline");
        return false;
    }

    connection_events = xEventGroupCreate();
    if (connection_events == NULL)
    {
        ESP_LOGE(TAG, "Wi-Fi connection event group creation failed");
        return false;
    }

    result = nvs_flash_init();
    if ((result == ESP_ERR_NVS_NO_FREE_PAGES) || (result == ESP_ERR_NVS_NEW_VERSION_FOUND))
    {
        result = nvs_flash_erase();
        if (result == ESP_OK)
        {
            result = nvs_flash_init();
        }
    }
    if (result == ESP_OK)
    {
        result = esp_netif_init();
    }
    if (result == ESP_OK)
    {
        result = esp_event_loop_create_default();
        if (result == ESP_ERR_INVALID_STATE)
        {
            result = ESP_OK;
        }
    }
    if (result == ESP_OK)
    {
        if (esp_netif_create_default_wifi_sta() == NULL)
        {
            result = ESP_FAIL;
        }
    }
    if (result == ESP_OK)
    {
        wifi_init_config_t wifi_init_configuration = WIFI_INIT_CONFIG_DEFAULT();
        result = esp_wifi_init(&wifi_init_configuration);
    }
    if (result == ESP_OK)
    {
        result = esp_event_handler_instance_register(WIFI_EVENT,
                                                    ESP_EVENT_ANY_ID,
                                                    network_event_handler,
                                                    NULL,
                                                    &wifi_event_handler_instance);
    }
    if (result == ESP_OK)
    {
        result = esp_event_handler_instance_register(IP_EVENT,
                                                    IP_EVENT_STA_GOT_IP,
                                                    network_event_handler,
                                                    NULL,
                                                    &ip_event_handler_instance);
    }
    if (result == ESP_OK)
    {
        wifi_config_t wifi_configuration = {0};
        (void)strlcpy((char *)wifi_configuration.sta.ssid,
                      APP_WIFI_SSID,
                      sizeof(wifi_configuration.sta.ssid));
        (void)strlcpy((char *)wifi_configuration.sta.password,
                      APP_WIFI_PASSWORD,
                      sizeof(wifi_configuration.sta.password));
        wifi_configuration.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        result = esp_wifi_set_mode(WIFI_MODE_STA);
        if (result == ESP_OK)
        {
            result = esp_wifi_set_config(WIFI_IF_STA, &wifi_configuration);
        }
    }
    if (result == ESP_OK)
    {
        result = esp_wifi_start();
    }

    if (result == ESP_OK)
    {
        is_initialized = true;
        ESP_LOGI(TAG, "Wi-Fi station started; connection attempts run asynchronously");
    }
    else
    {
        ESP_LOGE(TAG, "Wi-Fi station initialization failed: %s", esp_err_to_name(result));
    }

    return result == ESP_OK;
}

bool network_if_is_connected(void)
{
    return is_initialized &&
           ((xEventGroupGetBits(connection_events) & NETWORK_CONNECTED_BIT) != 0U);
}
