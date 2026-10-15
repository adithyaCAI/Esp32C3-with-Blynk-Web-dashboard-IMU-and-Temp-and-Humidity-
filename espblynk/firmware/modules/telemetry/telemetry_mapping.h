/* Portable mapping from sensor snapshots to the existing Blynk virtual pins. */
#ifndef TELEMETRY_MAPPING_H
#define TELEMETRY_MAPPING_H

#include "telemetry_if.h"

#include <stdbool.h>
#include <stdint.h>

#define TELEMETRY_VIRTUAL_PIN_COUNT (8U)

typedef struct
{
    uint8_t virtual_pins[TELEMETRY_VIRTUAL_PIN_COUNT];
    float values[TELEMETRY_VIRTUAL_PIN_COUNT];
} telemetry_mapping_batch_t;

/**
 * @brief Map all eight sensor fields into their frozen Blynk V0-V7 datastreams.
 * @param[in] snapshot Source measurements and orientation.
 * @param[out] batch Destination virtual-pin identifiers and values.
 * @return true when both pointers are valid and the batch is populated.
 */
bool telemetry_mapping_build_batch(const telemetry_if_snapshot_t *snapshot,
                                   telemetry_mapping_batch_t *batch);

#endif /* TELEMETRY_MAPPING_H */
