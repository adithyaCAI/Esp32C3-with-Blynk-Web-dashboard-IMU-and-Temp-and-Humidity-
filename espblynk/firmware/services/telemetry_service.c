/* Portable telemetry policy forwards complete snapshots to the platform adapter. */
#include "telemetry_service.h"

#include "network_service.h"
#include "telemetry_mapping.h"

#include <stddef.h>

bool telemetry_service_init(void)
{
    return telemetry_if_init();
}

bool telemetry_service_publish(const telemetry_if_snapshot_t *snapshot)
{
    telemetry_mapping_batch_t mapped_batch;
    bool is_publishable = telemetry_mapping_build_batch(snapshot, &mapped_batch) &&
                          network_service_is_connected();

    (void)mapped_batch;
    return is_publishable && telemetry_if_publish(snapshot);
}
