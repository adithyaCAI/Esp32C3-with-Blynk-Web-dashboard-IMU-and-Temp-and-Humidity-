/* Vendor-neutral Blynk telemetry publisher contract. */
#ifndef TELEMETRY_SERVICE_H
#define TELEMETRY_SERVICE_H

#include "telemetry_if.h"

#include <stdbool.h>

/**
 * @brief Initialize the Blynk datastream publisher.
 * @return true when the platform adapter initializes, otherwise false.
 */
bool telemetry_service_init(void);

/**
 * @brief Publish one sensor snapshot when network connectivity is available.
 * @param[in] snapshot Sensor readings and orientation for this publishing interval.
 * @return true when the snapshot was sent successfully; false when skipped or failed.
 */
bool telemetry_service_publish(const telemetry_if_snapshot_t *snapshot);

#endif /* TELEMETRY_SERVICE_H */
