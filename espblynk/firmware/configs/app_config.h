#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* ESP32-C3-DevKitM-1 sensor, Blynk template, and timing configuration. */
#define APP_BLYNK_TEMPLATE_ID         "TMPL3VjCwAzY1"
#define APP_BLYNK_TEMPLATE_NAME       "ESPInterface"
#define APP_BLYNK_BATCH_UPDATE_URL    "https://blynk.cloud/external/api/batch/update"

#include "app_secrets.h"

/* User-confirmed off-board MPU6050 wiring; differs from frozen PRD pin assignments. */
#define APP_MPU6050_SCL_GPIO          (3)
#define APP_MPU6050_SDA_GPIO          (2)
#define APP_MPU6050_I2C_ADDRESS       (0x68U)
#define APP_DHT22_DATA_GPIO           (1)
#define APP_DHT22_BOARD_PIN           (10U)
#define APP_SENSOR_I2C_FREQUENCY_HZ   (100000U)
#define APP_SENSOR_I2C_TIMEOUT_MS     (100U)
#define APP_SENSOR_I2C_SCAN_TIMEOUT_MS (10U)
#define APP_SENSOR_SAMPLE_PERIOD_MS   (2000U)
#define APP_SENSOR_REPORT_PERIOD_MS   (2000U)
#define APP_BLYNK_PUBLISH_PERIOD_MS   (2000U)
#define APP_SENSOR_RETRY_PERIOD_MS    (1000U)

#endif /* APP_CONFIG_H */
