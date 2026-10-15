/* Vendor-neutral sensor contract implemented by the ESP32 platform adapter. */
#ifndef SENSOR_IF_H
#define SENSOR_IF_H

#include <stdbool.h>
#include <stdint.h>

/** Sensor measurement payload with independent validity flags. */
typedef struct
{
    bool is_valid;
    float temperature_c;
    float humidity_percent;
    float acceleration_x_mps2;
    float acceleration_y_mps2;
    float acceleration_z_mps2;
    float gyroscope_x_dps;
    float gyroscope_y_dps;
    float gyroscope_z_dps;
} sensor_if_sample_t;

/**
 * @brief Configure and wake the MPU6050, validating its identity before use.
 * @return true when the bus, device identity and measurement registers are ready.
 */
bool sensor_if_mpu6050_init(void);

/**
 * @brief Initialize the DHT22 data line for single-wire transactions.
 * @return true when the configured GPIO can be used for DHT22 communication.
 */
bool sensor_if_dht22_init(void);

/**
 * @brief Acquire one MPU6050 acceleration and angular-rate sample.
 * @param[out] sample Destination for acceleration in m/s^2 and rates in degrees/s.
 * @return true when all six raw sensor channels were read and converted.
 */
bool sensor_if_mpu6050_read(sensor_if_sample_t *sample);

/**
 * @brief Acquire one DHT22 temperature and relative-humidity sample.
 * @param[out] temperature_c Destination for temperature in degrees Celsius.
 * @param[out] humidity_percent Destination for relative humidity in percent.
 * @return true when the response, checksum and physical ranges are valid.
 */
bool sensor_if_dht22_read(float *temperature_c, float *humidity_percent);

/**
 * @brief Report the current readings and each sensor's validity to the platform logger.
 * @param[in] sample Latest combined sample; fields are meaningful only when is_valid is true.
 * @param[in] orientation_x_deg Gyroscope-derived roll estimate in degrees.
 * @param[in] orientation_y_deg Gyroscope-derived pitch estimate in degrees.
 * @param[in] orientation_z_deg Gyroscope-derived yaw estimate in degrees.
 * @param[in] dht_valid True only when the current DHT22 read succeeded.
 */
void sensor_if_report(const sensor_if_sample_t *sample,
                      float orientation_x_deg,
                      float orientation_y_deg,
                      float orientation_z_deg,
                      bool dht_valid);

#endif /* SENSOR_IF_H */
