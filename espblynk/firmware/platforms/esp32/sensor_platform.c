/* ESP-IDF implementation of the MPU6050 I2C and DHT22 single-wire sensor contract. */
#include "sensor_if.h"

#include "app_config.h"
#include "logger.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stddef.h>
#include <stdint.h>

#define MPU6050_REG_SAMPLE_RATE_DIVIDER (0x19U)
#define MPU6050_REG_CONFIG (0x1AU)
#define MPU6050_REG_GYRO_CONFIG (0x1BU)
#define MPU6050_REG_ACCEL_CONFIG (0x1CU)
#define MPU6050_REG_ACCEL_XOUT_H (0x3BU)
#define MPU6050_REG_PWR_MGMT_1 (0x6BU)
#define MPU6050_REG_WHO_AM_I (0x75U)
#define MPU6050_WHO_AM_I_VALUE (0x68U)
#define MPU6050_REGISTER_BLOCK_SIZE (14U)
#define MPU6050_ACCEL_LSB_PER_G (16384.0F)
#define MPU6050_GYRO_LSB_PER_DPS (131.0F)
#define MPU6050_GRAVITY_MPS2 (9.80665F)
#define DHT22_RESPONSE_TIMEOUT_US (120U)
#define DHT22_PULSE_TIMEOUT_US (100U)
#define DHT22_START_LOW_US (1100U)
#define DHT22_START_SETTLE_US (30U)
#define DHT22_DATA_BITS (40U)
#define DHT22_ONE_THRESHOLD_US (50)
#define DHT22_MIN_TEMPERATURE_C (-40.0F)
#define DHT22_MAX_TEMPERATURE_C (80.0F)
#define DHT22_MAX_HUMIDITY_PERCENT (100.0F)

typedef enum
{
    DHT22_FRAME_OK = 0,
    DHT22_FRAME_START_DIRECTION_ERROR,
    DHT22_FRAME_START_LEVEL_ERROR,
    DHT22_FRAME_INPUT_DIRECTION_ERROR,
    DHT22_FRAME_RESPONSE_LOW_TIMEOUT,
    DHT22_FRAME_RESPONSE_HIGH_TIMEOUT,
    DHT22_FRAME_RESPONSE_END_TIMEOUT,
    DHT22_FRAME_BIT_RISE_TIMEOUT,
    DHT22_FRAME_BIT_FALL_TIMEOUT
} dht22_frame_status_t;

static const char *TAG = "sensor_platform";
static i2c_master_bus_handle_t sensor_i2c_bus;
static i2c_master_dev_handle_t mpu6050_device;
static bool is_i2c_bus_ready;
static bool is_mpu6050_device_ready;

/* Probe the complete legal 7-bit address range before diagnosing an absent sensor. */
static void sensor_i2c_scan_bus(void)
{
    uint16_t address;
    bool found_acknowledging_device = false;

    for (address = 0x08U; address <= 0x77U; ++address)
    {
        esp_err_t probe_result = i2c_master_probe(sensor_i2c_bus,
                                                  address,
                                                  APP_SENSOR_I2C_SCAN_TIMEOUT_MS);
        if (probe_result == ESP_OK)
        {
            ESP_LOGW(TAG, "I2C scan found an acknowledging device at 0x%02X", address);
            found_acknowledging_device = true;
        }
        else if (probe_result != ESP_ERR_NOT_FOUND)
        {
            ESP_LOGW(TAG, "I2C scan probe 0x%02X failed: %s",
                     address, esp_err_to_name(probe_result));
        }
    }

    if (!found_acknowledging_device)
    {
        ESP_LOGW(TAG, "I2C address scan found no acknowledging devices from 0x08 through 0x77");
    }
}

/* Read a contiguous MPU6050 register range using the ESP-IDF master driver. */
static esp_err_t mpu6050_read_registers(uint8_t register_address,
                                        uint8_t *read_buffer,
                                        size_t read_size)
{
    return i2c_master_transmit_receive(mpu6050_device,
                                       &register_address,
                                       sizeof(register_address),
                                       read_buffer,
                                       read_size,
                                       APP_SENSOR_I2C_TIMEOUT_MS);
}

/* Write one MPU6050 register after its I2C address byte. */
static esp_err_t mpu6050_write_register(uint8_t register_address, uint8_t value)
{
    uint8_t write_buffer[2] = {register_address, value};

    return i2c_master_transmit(mpu6050_device,
                               write_buffer,
                               sizeof(write_buffer),
                               APP_SENSOR_I2C_TIMEOUT_MS);
}

/* Wait for a DHT22 data-line level without exceeding the supplied microsecond bound. */
static bool dht22_wait_for_level(int expected_level, uint32_t timeout_us)
{
    int64_t start_time_us = esp_timer_get_time();
    bool level_reached = false;

    while (((uint64_t)(esp_timer_get_time() - start_time_us) <= timeout_us) && !level_reached)
    {
        level_reached = (gpio_get_level((gpio_num_t)APP_DHT22_DATA_GPIO) == expected_level);
    }

    return level_reached;
}

/* Decode one DHT22 transaction and reject framing, checksum, and range errors. */
static dht22_frame_status_t dht22_read_frame(uint8_t frame[DHT22_DATA_BITS / 8U])
{
    dht22_frame_status_t frame_status = DHT22_FRAME_OK;
    uint8_t bit_index;

    if (gpio_set_direction((gpio_num_t)APP_DHT22_DATA_GPIO, GPIO_MODE_OUTPUT) != ESP_OK)
    {
        frame_status = DHT22_FRAME_START_DIRECTION_ERROR;
    }
    else if (gpio_set_level((gpio_num_t)APP_DHT22_DATA_GPIO, 0U) != ESP_OK)
    {
        frame_status = DHT22_FRAME_START_LEVEL_ERROR;
    }
    else
    {
        esp_rom_delay_us(DHT22_START_LOW_US);
        if (gpio_set_direction((gpio_num_t)APP_DHT22_DATA_GPIO, GPIO_MODE_INPUT) != ESP_OK)
        {
            frame_status = DHT22_FRAME_INPUT_DIRECTION_ERROR;
        }
        else
        {
            esp_rom_delay_us(DHT22_START_SETTLE_US);
            if (!dht22_wait_for_level(0, DHT22_RESPONSE_TIMEOUT_US))
            {
                frame_status = DHT22_FRAME_RESPONSE_LOW_TIMEOUT;
            }
            else if (!dht22_wait_for_level(1, DHT22_RESPONSE_TIMEOUT_US))
            {
                frame_status = DHT22_FRAME_RESPONSE_HIGH_TIMEOUT;
            }
            else if (!dht22_wait_for_level(0, DHT22_RESPONSE_TIMEOUT_US))
            {
                frame_status = DHT22_FRAME_RESPONSE_END_TIMEOUT;
            }
        }
    }

    for (bit_index = 0U; (bit_index < DHT22_DATA_BITS) && (frame_status == DHT22_FRAME_OK); ++bit_index)
    {
        int64_t pulse_start_us;
        int64_t pulse_width_us;
        uint8_t byte_index = (uint8_t)(bit_index / 8U);

        int64_t edge_wait_start_us = esp_timer_get_time();
        if (!dht22_wait_for_level(1, DHT22_PULSE_TIMEOUT_US))
        {
            int64_t edge_wait_elapsed_us = esp_timer_get_time() - edge_wait_start_us;
            int data_line_level = gpio_get_level((gpio_num_t)APP_DHT22_DATA_GPIO);
            frame_status = DHT22_FRAME_BIT_RISE_TIMEOUT;
            ESP_LOGW(TAG,
                     "DHT22 bit rising-edge timeout: bit=%u line=%d elapsed_us=%u limit_us=%u",
                     (unsigned int)bit_index,
                     data_line_level,
                     (unsigned int)edge_wait_elapsed_us,
                     (unsigned int)DHT22_PULSE_TIMEOUT_US);
        }
        else
        {
            pulse_start_us = esp_timer_get_time();
            if (!dht22_wait_for_level(0, DHT22_PULSE_TIMEOUT_US))
            {
                frame_status = DHT22_FRAME_BIT_FALL_TIMEOUT;
            }
            pulse_width_us = esp_timer_get_time() - pulse_start_us;

            if (frame_status == DHT22_FRAME_OK)
            {
                frame[byte_index] = (uint8_t)(frame[byte_index] << 1U);
                if (pulse_width_us > DHT22_ONE_THRESHOLD_US)
                {
                    frame[byte_index] = (uint8_t)(frame[byte_index] | 1U);
                }
            }
        }
    }

    return frame_status;
}

bool sensor_if_mpu6050_init(void)
{
    esp_err_t result = ESP_OK;
    uint8_t identity = 0U;

    if (!is_i2c_bus_ready)
    {
        const i2c_master_bus_config_t bus_config = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = (gpio_num_t)APP_MPU6050_SDA_GPIO,
            .scl_io_num = (gpio_num_t)APP_MPU6050_SCL_GPIO,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7U,
            .flags.enable_internal_pullup = true,
        };

        result = i2c_new_master_bus(&bus_config, &sensor_i2c_bus);
        is_i2c_bus_ready = (result == ESP_OK);
    }

    if ((result == ESP_OK) && !is_mpu6050_device_ready)
    {
        const i2c_device_config_t device_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = APP_MPU6050_I2C_ADDRESS,
            .scl_speed_hz = APP_SENSOR_I2C_FREQUENCY_HZ,
        };

        result = i2c_master_bus_add_device(sensor_i2c_bus, &device_config, &mpu6050_device);
        is_mpu6050_device_ready = (result == ESP_OK);
    }

    if ((result == ESP_OK) && is_mpu6050_device_ready)
    {
        result = i2c_master_probe(sensor_i2c_bus,
                                  APP_MPU6050_I2C_ADDRESS,
                                  APP_SENSOR_I2C_SCAN_TIMEOUT_MS);
    }

    if ((result == ESP_OK) && is_mpu6050_device_ready)
    {
        result = mpu6050_read_registers(MPU6050_REG_WHO_AM_I, &identity, sizeof(identity));
        if ((result == ESP_OK) && (identity != MPU6050_WHO_AM_I_VALUE))
        {
            result = ESP_ERR_NOT_FOUND;
        }
    }

    if (result == ESP_OK)
    {
        result = mpu6050_write_register(MPU6050_REG_PWR_MGMT_1, 0x00U);
    }
    if (result == ESP_OK)
    {
        result = mpu6050_write_register(MPU6050_REG_CONFIG, 0x03U);
    }
    if (result == ESP_OK)
    {
        result = mpu6050_write_register(MPU6050_REG_SAMPLE_RATE_DIVIDER, 0x00U);
    }
    if (result == ESP_OK)
    {
        result = mpu6050_write_register(MPU6050_REG_GYRO_CONFIG, 0x00U);
    }
    if (result == ESP_OK)
    {
        result = mpu6050_write_register(MPU6050_REG_ACCEL_CONFIG, 0x00U);
    }

    if ((result == ESP_ERR_NOT_FOUND) && is_i2c_bus_ready)
    {
        sensor_i2c_scan_bus();
    }

    if (result != ESP_OK)
    {
        ESP_LOGW(TAG, "MPU6050 initialization failed: %s", esp_err_to_name(result));
    }
    else
    {
        ESP_LOGI(TAG, "MPU6050 ready at 7-bit address 0x%02X", APP_MPU6050_I2C_ADDRESS);
    }

    return result == ESP_OK;
}

bool sensor_if_dht22_init(void)
{
    const gpio_config_t pin_config = {
        .pin_bit_mask = (1ULL << APP_DHT22_DATA_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t result = gpio_config(&pin_config);

    if (result != ESP_OK)
    {
        ESP_LOGW(TAG, "DHT22 GPIO initialization failed: %s", esp_err_to_name(result));
    }
    else
    {
        ESP_LOGI(TAG, "DHT22 data input ready on GPIO%d", APP_DHT22_DATA_GPIO);
    }

    return result == ESP_OK;
}

bool sensor_if_mpu6050_read(sensor_if_sample_t *sample)
{
    uint8_t register_data[MPU6050_REGISTER_BLOCK_SIZE] = {0U};
    esp_err_t result;
    bool is_sample_valid = (sample != NULL) && is_mpu6050_device_ready;

    if (is_sample_valid)
    {
        result = mpu6050_read_registers(MPU6050_REG_ACCEL_XOUT_H,
                                        register_data,
                                        sizeof(register_data));
        is_sample_valid = (result == ESP_OK);

        if (is_sample_valid)
        {
            int16_t acceleration_x_raw = (int16_t)(((uint16_t)register_data[0] << 8U) | register_data[1]);
            int16_t acceleration_y_raw = (int16_t)(((uint16_t)register_data[2] << 8U) | register_data[3]);
            int16_t acceleration_z_raw = (int16_t)(((uint16_t)register_data[4] << 8U) | register_data[5]);
            int16_t gyroscope_x_raw = (int16_t)(((uint16_t)register_data[8] << 8U) | register_data[9]);
            int16_t gyroscope_y_raw = (int16_t)(((uint16_t)register_data[10] << 8U) | register_data[11]);
            int16_t gyroscope_z_raw = (int16_t)(((uint16_t)register_data[12] << 8U) | register_data[13]);

            sample->acceleration_x_mps2 = ((float)acceleration_x_raw / MPU6050_ACCEL_LSB_PER_G) * MPU6050_GRAVITY_MPS2;
            sample->acceleration_y_mps2 = ((float)acceleration_y_raw / MPU6050_ACCEL_LSB_PER_G) * MPU6050_GRAVITY_MPS2;
            sample->acceleration_z_mps2 = ((float)acceleration_z_raw / MPU6050_ACCEL_LSB_PER_G) * MPU6050_GRAVITY_MPS2;
            sample->gyroscope_x_dps = (float)gyroscope_x_raw / MPU6050_GYRO_LSB_PER_DPS;
            sample->gyroscope_y_dps = (float)gyroscope_y_raw / MPU6050_GYRO_LSB_PER_DPS;
            sample->gyroscope_z_dps = (float)gyroscope_z_raw / MPU6050_GYRO_LSB_PER_DPS;
            sample->is_valid = true;
        }
    }

    return is_sample_valid;
}

bool sensor_if_dht22_read(float *temperature_c, float *humidity_percent)
{
    uint8_t frame[DHT22_DATA_BITS / 8U] = {0U};
    bool is_sample_valid = (temperature_c != NULL) && (humidity_percent != NULL);
    dht22_frame_status_t frame_status = DHT22_FRAME_OK;

    if (is_sample_valid)
    {
        frame_status = dht22_read_frame(frame);
        is_sample_valid = (frame_status == DHT22_FRAME_OK) &&
                          ((uint8_t)(frame[0] + frame[1] + frame[2] + frame[3]) == frame[4]);
        if (frame_status != DHT22_FRAME_OK)
        {
            ESP_LOGW(TAG, "DHT22 frame read failed at stage %u", (unsigned int)frame_status);
        }
        else if (!is_sample_valid)
        {
            ESP_LOGW(TAG, "DHT22 frame checksum mismatch");
        }
    }

    if (is_sample_valid)
    {
        uint16_t humidity_raw = (uint16_t)(((uint16_t)frame[0] << 8U) | frame[1]);
        uint16_t temperature_magnitude_raw = (uint16_t)(((uint16_t)(frame[2] & 0x7FU) << 8U) | frame[3]);
        float temperature_value_c = (float)temperature_magnitude_raw / 10.0F;

        if ((frame[2] & 0x80U) != 0U)
        {
            temperature_value_c = -temperature_value_c;
        }

        *temperature_c = temperature_value_c;
        *humidity_percent = (float)humidity_raw / 10.0F;
        is_sample_valid = (*temperature_c >= DHT22_MIN_TEMPERATURE_C) &&
                          (*temperature_c <= DHT22_MAX_TEMPERATURE_C) &&
                          (*humidity_percent >= 0.0F) &&
                          (*humidity_percent <= DHT22_MAX_HUMIDITY_PERCENT);
    }

    return is_sample_valid;
}

void sensor_if_report(const sensor_if_sample_t *sample,
                      float orientation_x_deg,
                      float orientation_y_deg,
                      float orientation_z_deg,
                      bool dht_valid)
{
    if (sample != NULL)
    {
        if (sample->is_valid && dht_valid)
        {
            ESP_LOGI(TAG,
                     "DHT22 temperature=%.1f C humidity=%.1f %%; accel=(%.3f, %.3f, %.3f) m/s^2; orientation=(%.2f, %.2f, %.2f) deg",
                     sample->temperature_c,
                     sample->humidity_percent,
                     sample->acceleration_x_mps2,
                     sample->acceleration_y_mps2,
                     sample->acceleration_z_mps2,
                     orientation_x_deg,
                     orientation_y_deg,
                     orientation_z_deg);
        }
        else
        {
            ESP_LOGW(TAG,
                     "sensor snapshot unavailable: MPU6050_valid=%u DHT22_valid=%u",
                     (unsigned int)sample->is_valid,
                     (unsigned int)dht_valid);
        }
    }
}
