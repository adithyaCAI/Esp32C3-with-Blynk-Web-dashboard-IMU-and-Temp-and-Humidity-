/* Vendor-neutral sensor loop with bounded scheduling and failure-tolerant updates. */
#include "sensor_service.h"

#include "app_config.h"
#include "osal.h"
#include "network_service.h"
#include "orientation.h"
#include "sensor_if.h"
#include "telemetry_service.h"

#include <stdbool.h>
#include <stdint.h>

#define SENSOR_SERVICE_INITIAL_RETRY_MS (APP_SENSOR_RETRY_PERIOD_MS)

/* Continue at a fixed IMU cadence and report snapshots at the configured interval. */
void sensor_service_task(void *argument)
{
    orientation_state_t orientation = {0};
    sensor_if_sample_t sample = {0};
    uint32_t next_sample_ms = osal_uptime_ms();
    uint32_t next_report_ms = next_sample_ms;
    uint32_t last_sample_ms = 0U;
    uint32_t next_dht_read_ms = next_sample_ms;
    bool is_mpu_ready = sensor_if_mpu6050_init();
    bool is_dht_ready = sensor_if_dht22_init();
    bool is_orientation_ready = orientation_init(&orientation);
    bool is_dht_valid = false;
    bool is_telemetry_ready = telemetry_service_init();
    uint32_t next_publish_ms = next_report_ms;

    (void)argument;

    for (;;)
    {
        uint32_t now_ms = osal_uptime_ms();

        if (!is_mpu_ready)
        {
            is_mpu_ready = sensor_if_mpu6050_init();
        }

        if (!is_dht_ready)
        {
            is_dht_ready = sensor_if_dht22_init();
        }

        if (is_mpu_ready && ((int32_t)(now_ms - next_sample_ms) >= 0))
        {
            sensor_if_sample_t current_sample = {0};
            uint32_t elapsed_ms = (last_sample_ms == 0U) ? 1U : now_ms - last_sample_ms;

            next_sample_ms = now_ms + APP_SENSOR_SAMPLE_PERIOD_MS;
            last_sample_ms = now_ms;
            current_sample.is_valid = sensor_if_mpu6050_read(&current_sample);

            if (current_sample.is_valid)
            {
                sample.acceleration_x_mps2 = current_sample.acceleration_x_mps2;
                sample.acceleration_y_mps2 = current_sample.acceleration_y_mps2;
                sample.acceleration_z_mps2 = current_sample.acceleration_z_mps2;
                sample.gyroscope_x_dps = current_sample.gyroscope_x_dps;
                sample.gyroscope_y_dps = current_sample.gyroscope_y_dps;
                sample.gyroscope_z_dps = current_sample.gyroscope_z_dps;
                sample.is_valid = true;

                if (is_orientation_ready)
                {
                    (void)orientation_update(&orientation,
                                             current_sample.acceleration_x_mps2,
                                             current_sample.acceleration_y_mps2,
                                             current_sample.acceleration_z_mps2,
                                             current_sample.gyroscope_x_dps,
                                             current_sample.gyroscope_y_dps,
                                             current_sample.gyroscope_z_dps,
                                             elapsed_ms);
                }
            }
        }

        if (is_dht_ready && ((int32_t)(now_ms - next_dht_read_ms) >= 0))
        {
            float temperature_c = 0.0F;
            float humidity_percent = 0.0F;

            is_dht_valid = sensor_if_dht22_read(&temperature_c, &humidity_percent);
            if (is_dht_valid)
            {
                sample.temperature_c = temperature_c;
                sample.humidity_percent = humidity_percent;
            }
            next_dht_read_ms = now_ms + APP_SENSOR_REPORT_PERIOD_MS;
        }

        (void)network_service_is_connected();

        if ((int32_t)(now_ms - next_report_ms) >= 0)
        {
            sensor_if_report(&sample,
                             orientation.roll_deg,
                             orientation.pitch_deg,
                             orientation.yaw_deg,
                             is_dht_valid);
            next_report_ms = now_ms + APP_SENSOR_REPORT_PERIOD_MS;
        }

        if ((int32_t)(now_ms - next_publish_ms) >= 0)
        {
            if (!is_telemetry_ready)
            {
                is_telemetry_ready = telemetry_service_init();
            }

            if (is_telemetry_ready && sample.is_valid)
            {
                telemetry_if_snapshot_t telemetry_snapshot = {
                    .temperature_c = sample.temperature_c,
                    .humidity_percent = sample.humidity_percent,
                    .acceleration_x_mps2 = sample.acceleration_x_mps2,
                    .acceleration_y_mps2 = sample.acceleration_y_mps2,
                    .acceleration_z_mps2 = sample.acceleration_z_mps2,
                    .orientation_x_deg = orientation.roll_deg,
                    .orientation_y_deg = orientation.pitch_deg,
                    .orientation_z_deg = orientation.yaw_deg,
                };
                (void)telemetry_service_publish(&telemetry_snapshot);
            }
            next_publish_ms = now_ms + APP_BLYNK_PUBLISH_PERIOD_MS;
        }

        if (!is_mpu_ready || !is_dht_ready)
        {
            osal_delay_ms(SENSOR_SERVICE_INITIAL_RETRY_MS);
        }
        else
        {
            osal_delay_ms(1U);
        }
    }
}
