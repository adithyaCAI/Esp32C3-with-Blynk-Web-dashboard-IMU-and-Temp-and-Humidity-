/* Pure eight-field mapping for the existing Blynk virtual datastreams. */
#include "telemetry_mapping.h"

#include <stddef.h>

bool telemetry_mapping_build_batch(const telemetry_if_snapshot_t *snapshot,
                                   telemetry_mapping_batch_t *batch)
{
    bool is_valid = (snapshot != NULL) && (batch != NULL);

    if (is_valid)
    {
        batch->virtual_pins[0] = 0U;
        batch->virtual_pins[1] = 1U;
        batch->virtual_pins[2] = 2U;
        batch->virtual_pins[3] = 3U;
        batch->virtual_pins[4] = 4U;
        batch->virtual_pins[5] = 5U;
        batch->virtual_pins[6] = 6U;
        batch->virtual_pins[7] = 7U;

        batch->values[0] = snapshot->temperature_c;
        batch->values[1] = snapshot->humidity_percent;
        batch->values[2] = snapshot->acceleration_x_mps2;
        batch->values[3] = snapshot->acceleration_y_mps2;
        batch->values[4] = snapshot->acceleration_z_mps2;
        batch->values[5] = snapshot->orientation_x_deg;
        batch->values[6] = snapshot->orientation_y_deg;
        batch->values[7] = snapshot->orientation_z_deg;
    }

    return is_valid;
}
