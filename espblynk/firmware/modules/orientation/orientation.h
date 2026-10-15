/* Portable complementary orientation filter for MPU6050 angular rate and acceleration. */
#ifndef ORIENTATION_H
#define ORIENTATION_H

#include <stdbool.h>
#include <stdint.h>

/** Orientation state in degrees; yaw intentionally has no absolute reference. */
typedef struct
{
    float roll_deg;
    float pitch_deg;
    float yaw_deg;
    bool is_initialized;
} orientation_state_t;

/**
 * @brief Initialize a relative orientation state with yaw at zero.
 * @param[out] state Filter state to initialize.
 * @return true when state is non-null and initialized.
 */
bool orientation_init(orientation_state_t *state);

/**
 * @brief Integrate gyro rates and correct roll/pitch from gravity acceleration.
 * @param[in,out] state Filter state updated in place.
 * @param[in] acceleration_x_mps2 X acceleration in m/s^2.
 * @param[in] acceleration_y_mps2 Y acceleration in m/s^2.
 * @param[in] acceleration_z_mps2 Z acceleration in m/s^2.
 * @param[in] gyroscope_x_dps X angular rate in degrees/s.
 * @param[in] gyroscope_y_dps Y angular rate in degrees/s.
 * @param[in] gyroscope_z_dps Z angular rate in degrees/s.
 * @param[in] elapsed_ms Time since the preceding sample, limited to a safe range.
 * @return true when the update was applied; false for invalid state or timing.
 */
bool orientation_update(orientation_state_t *state,
                        float acceleration_x_mps2,
                        float acceleration_y_mps2,
                        float acceleration_z_mps2,
                        float gyroscope_x_dps,
                        float gyroscope_y_dps,
                        float gyroscope_z_dps,
                        uint32_t elapsed_ms);

#endif /* ORIENTATION_H */
