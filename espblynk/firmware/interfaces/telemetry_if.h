/* Vendor-neutral Blynk datastream publishing contract. */
#ifndef TELEMETRY_IF_H
#define TELEMETRY_IF_H

#include <stdbool.h>
#include <stdint.h>

/** Sensor values and orientation for one Blynk datastream publishing cycle. */
typedef struct
{
    float temperature_c;
    float humidity_percent;
    float acceleration_x_mps2;
    float acceleration_y_mps2;
    float acceleration_z_mps2;
    float orientation_x_deg;
    float orientation_y_deg;
    float orientation_z_deg;
} telemetry_if_snapshot_t;

/**
 * @brief Initialize Blynk HTTPS publishing using the configured device token.
 * @return true when the publishing adapter is ready to perform updates.
 */
bool telemetry_if_init(void);

/**
 * @brief Submit all eight existing Blynk datastream values in one request.
 * @param[in] snapshot Current sensor snapshot and orientation values.
 * @return true when Blynk accepts the update; false on unavailable or failed request.
 */
bool telemetry_if_publish(const telemetry_if_snapshot_t *snapshot);

#endif /* TELEMETRY_IF_H */
