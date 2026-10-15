/* ESP-IDF Blynk Cloud HTTPS adapter for atomic updates to existing datastreams. */
#include "telemetry_if.h"

#include "app_config.h"
#include "logger.h"
#include "telemetry_mapping.h"

#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define TELEMETRY_URL_CAPACITY (512U)
#define TELEMETRY_REQUEST_TIMEOUT_MS (5000)
#define TELEMETRY_SUCCESS_STATUS_CODE (200)
#define TELEMETRY_WORKER_STACK_BYTES (4096U)
#define TELEMETRY_WORKER_PRIORITY (4U)
#define TELEMETRY_QUEUE_LENGTH (1U)

static const char *TAG = "telemetry_platform";
static esp_http_client_handle_t blynk_http_client;
static QueueHandle_t telemetry_queue;
static bool is_initialized;

/* Refuse to activate the publisher while the local secret template is unchanged. */
static bool telemetry_credentials_are_configured(void)
{
    return (strcmp(APP_BLYNK_AUTH_TOKEN, "SET_BLYNK_AUTH_TOKEN") != 0) &&
           (strcmp(APP_WIFI_SSID, "SET_WIFI_SSID") != 0) &&
           (strcmp(APP_WIFI_PASSWORD, "SET_WIFI_PASSWORD") != 0);
}

/* Build Blynk's documented batch-update query using the V0-V7 pin mapping. */
static bool telemetry_build_request_url(const telemetry_if_snapshot_t *snapshot,
                                        char request_url[TELEMETRY_URL_CAPACITY])
{
    telemetry_mapping_batch_t batch;
    size_t url_length = 0U;
    int prefix_length;
    bool is_valid = telemetry_mapping_build_batch(snapshot, &batch);

    if (is_valid)
    {
        prefix_length = snprintf(request_url,
                                 TELEMETRY_URL_CAPACITY,
                                 "%s?token=%s",
                                 APP_BLYNK_BATCH_UPDATE_URL,
                                 APP_BLYNK_AUTH_TOKEN);
        is_valid = (prefix_length > 0) &&
                   ((size_t)prefix_length < TELEMETRY_URL_CAPACITY);
        if (is_valid)
        {
            url_length = (size_t)prefix_length;
        }
    }

    for (uint8_t index = 0U; (index < TELEMETRY_VIRTUAL_PIN_COUNT) && is_valid; ++index)
    {
        int parameter_length = snprintf(request_url + url_length,
                                        TELEMETRY_URL_CAPACITY - url_length,
                                        "&v%u=%.5f",
                                        (unsigned int)batch.virtual_pins[index],
                                        (double)batch.values[index]);
        is_valid = (parameter_length > 0) &&
                   ((size_t)parameter_length < (TELEMETRY_URL_CAPACITY - url_length));
        if (is_valid)
        {
            url_length += (size_t)parameter_length;
        }
    }

    return is_valid;
}

/* Send requests from a dedicated worker so TLS/network waits cannot stall sampling. */
static void telemetry_worker_task(void *argument)
{
    telemetry_if_snapshot_t snapshot;
    char request_url[TELEMETRY_URL_CAPACITY];

    (void)argument;

    for (;;)
    {
        if (xQueueReceive(telemetry_queue, &snapshot, portMAX_DELAY) == pdTRUE)
        {
            if (!telemetry_build_request_url(&snapshot, request_url))
            {
                ESP_LOGE(TAG, "Blynk request URL could not be formatted within its buffer");
            }
            else
            {
                esp_err_t result = esp_http_client_set_url(blynk_http_client, request_url);
                if (result == ESP_OK)
                {
                    result = esp_http_client_set_method(blynk_http_client, HTTP_METHOD_GET);
                }
                if (result == ESP_OK)
                {
                    result = esp_http_client_perform(blynk_http_client);
                }

                int http_status = esp_http_client_get_status_code(blynk_http_client);
                if ((result == ESP_OK) && (http_status == TELEMETRY_SUCCESS_STATUS_CODE))
                {
                    ESP_LOGI(TAG, "Blynk batch update accepted by existing datastreams (HTTP %d)",
                             http_status);
                }
                else
                {
                    ESP_LOGW(TAG, "Blynk HTTPS batch update failed: err=%s HTTP=%d",
                             esp_err_to_name(result),
                             http_status);
                }
            }
        }
    }
}

bool telemetry_if_init(void)
{
    bool is_ready = false;

    if (is_initialized)
    {
        return true;
    }

    if (!telemetry_credentials_are_configured())
    {
        ESP_LOGW(TAG, "Wi-Fi/Blynk credentials are placeholders; Blynk publishing disabled");
    }
    else
    {
        const esp_http_client_config_t client_configuration = {
            .url = APP_BLYNK_BATCH_UPDATE_URL,
            .method = HTTP_METHOD_GET,
            .timeout_ms = TELEMETRY_REQUEST_TIMEOUT_MS,
            .crt_bundle_attach = esp_crt_bundle_attach,
            .keep_alive_enable = true,
        };

        blynk_http_client = esp_http_client_init(&client_configuration);
        if (blynk_http_client != NULL)
        {
            telemetry_queue = xQueueCreate(TELEMETRY_QUEUE_LENGTH,
                                           sizeof(telemetry_if_snapshot_t));
            if (telemetry_queue != NULL)
            {
                BaseType_t task_result = xTaskCreate(telemetry_worker_task,
                                                     "blynk_publish",
                                                     TELEMETRY_WORKER_STACK_BYTES,
                                                     NULL,
                                                     TELEMETRY_WORKER_PRIORITY,
                                                     NULL);
                is_initialized = (task_result == pdPASS);
            }

            if (!is_initialized)
            {
                if (telemetry_queue != NULL)
                {
                    vQueueDelete(telemetry_queue);
                    telemetry_queue = NULL;
                }
                (void)esp_http_client_cleanup(blynk_http_client);
                blynk_http_client = NULL;
            }
        }

        is_ready = is_initialized;
        if (!is_ready)
        {
            ESP_LOGE(TAG, "Blynk HTTPS publisher initialization failed");
        }
        else
        {
            ESP_LOGI(TAG, "Blynk HTTPS publisher configured for template %s (%s)",
                     APP_BLYNK_TEMPLATE_NAME,
                     APP_BLYNK_TEMPLATE_ID);
        }
    }

    return is_ready;
}

bool telemetry_if_publish(const telemetry_if_snapshot_t *snapshot)
{
    bool is_queued = (snapshot != NULL) && is_initialized &&
                     (xQueueOverwrite(telemetry_queue, snapshot) == pdPASS);

    return is_queued;
}
