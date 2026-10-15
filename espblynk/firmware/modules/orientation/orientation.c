/* Portable complementary filter; gravity corrects tilt only and never absolute yaw. */
#include "orientation.h"

#include <math.h>
#include <stddef.h>

#define ORIENTATION_GRAVITY_MPS2 (9.80665F)
#define ORIENTATION_TILT_CORRECTION_WEIGHT (0.02F)
#define ORIENTATION_MIN_ACCELERATION_MPS2 (7.0F)
#define ORIENTATION_MAX_ACCELERATION_MPS2 (12.5F)
#define ORIENTATION_MAX_ELAPSED_MS (2500U)
#define ORIENTATION_PI (3.14159265358979323846F)

/* Convert gravity-vector geometry into roll and pitch, in radians. */
static void orientation_acceleration_angles(float acceleration_x_mps2,
                                            float acceleration_y_mps2,
                                            float acceleration_z_mps2,
                                            float *roll_radians,
                                            float *pitch_radians)
{
    *roll_radians = atan2f(acceleration_y_mps2, acceleration_z_mps2);
    *pitch_radians = atan2f(-acceleration_x_mps2,
                            sqrtf((acceleration_y_mps2 * acceleration_y_mps2) +
                                  (acceleration_z_mps2 * acceleration_z_mps2)));
}

bool orientation_init(orientation_state_t *state)
{
    bool is_valid = (state != NULL);

    if (is_valid)
    {
        state->roll_deg = 0.0F;
        state->pitch_deg = 0.0F;
        state->yaw_deg = 0.0F;
        state->is_initialized = true;
    }

    return is_valid;
}

bool orientation_update(orientation_state_t *state,
                        float acceleration_x_mps2,
                        float acceleration_y_mps2,
                        float acceleration_z_mps2,
                        float gyroscope_x_dps,
                        float gyroscope_y_dps,
                        float gyroscope_z_dps,
                        uint32_t elapsed_ms)
{
    bool is_valid = (state != NULL) && state->is_initialized &&
                    (elapsed_ms > 0U) && (elapsed_ms <= ORIENTATION_MAX_ELAPSED_MS);

    if (is_valid)
    {
        float elapsed_seconds = (float)elapsed_ms / 1000.0F;
        float acceleration_magnitude = sqrtf((acceleration_x_mps2 * acceleration_x_mps2) +
                                             (acceleration_y_mps2 * acceleration_y_mps2) +
                                             (acceleration_z_mps2 * acceleration_z_mps2));
        float roll_radians = 0.0F;
        float pitch_radians = 0.0F;
        bool gravity_is_plausible = (acceleration_magnitude >= ORIENTATION_MIN_ACCELERATION_MPS2) &&
                                    (acceleration_magnitude <= ORIENTATION_MAX_ACCELERATION_MPS2);

        state->roll_deg += gyroscope_x_dps * elapsed_seconds;
        state->pitch_deg += gyroscope_y_dps * elapsed_seconds;
        state->yaw_deg += gyroscope_z_dps * elapsed_seconds;

        if (gravity_is_plausible)
        {
            orientation_acceleration_angles(acceleration_x_mps2,
                                            acceleration_y_mps2,
                                            acceleration_z_mps2,
                                            &roll_radians,
                                            &pitch_radians);
            state->roll_deg = ((1.0F - ORIENTATION_TILT_CORRECTION_WEIGHT) * state->roll_deg) +
                              (ORIENTATION_TILT_CORRECTION_WEIGHT * roll_radians *
                               (180.0F / ORIENTATION_PI));
            state->pitch_deg = ((1.0F - ORIENTATION_TILT_CORRECTION_WEIGHT) * state->pitch_deg) +
                               (ORIENTATION_TILT_CORRECTION_WEIGHT * pitch_radians *
                                (180.0F / ORIENTATION_PI));
        }
    }

    return is_valid;
}
