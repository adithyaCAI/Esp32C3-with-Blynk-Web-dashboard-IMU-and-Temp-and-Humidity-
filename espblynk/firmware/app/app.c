/* Application startup initializes portable sensor processing and launches it via OSAL. */
#include "app.h"

#include "logger.h"
#include "network_service.h"
#include "osal.h"
#include "sensor_service.h"

#include <stddef.h>

#define APP_SENSOR_TASK_STACK_BYTES (4096U)
#define APP_SENSOR_TASK_PRIORITY (5U)

static const char *TAG = "app";

/**
 * @brief Start sensor acquisition as an independent task after application boot.
 *
 * @pre ESP-IDF has initialized the FreeRTOS scheduler.
 * @post A sensor task runs until reset; task creation failure is logged and leaves
 *       no false indication that measurements are active.
 */
void app_start(void)
{
    bool is_network_ready = network_service_start();
    osal_task_t sensor_task;

    if (!is_network_ready)
    {
        ESP_LOGW(TAG, "network service unavailable; sensor acquisition will continue offline");
    }

    sensor_task = osal_task_create(sensor_service_task,
                                               "sensor_service",
                                               APP_SENSOR_TASK_STACK_BYTES,
                                               NULL,
                                               APP_SENSOR_TASK_PRIORITY);

    if (sensor_task == NULL)
    {
        ESP_LOGE(TAG, "sensor task creation failed");
    }
    else
    {
        ESP_LOGI(TAG, "sensor acquisition task started");
    }
}
