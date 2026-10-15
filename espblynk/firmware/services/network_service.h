/* Vendor-neutral Wi-Fi connectivity service contract. */
#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <stdbool.h>

/**
 * @brief Start station connectivity using the configured secret values.
 * @return true when platform initialization succeeds; false on setup failure.
 */
bool network_service_start(void);

/**
 * @brief Check connectivity and request nonblocking recovery when disconnected.
 * @return true when Wi-Fi currently has an IP connection.
 */
bool network_service_is_connected(void);

#endif /* NETWORK_SERVICE_H */
