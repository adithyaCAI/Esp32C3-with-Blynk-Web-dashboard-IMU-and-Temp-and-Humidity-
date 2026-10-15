/* Vendor-neutral network contract consumed by telemetry and sensor services. */
#ifndef NETWORK_IF_H
#define NETWORK_IF_H

#include <stdbool.h>

/**
 * @brief Start Wi-Fi station operation using locally supplied credentials.
 * @return true when Wi-Fi initialization and station start succeed.
 */
bool network_if_start(void);

/**
 * @brief Report whether the station currently has an IP connection.
 * @return true only while the platform has a current station IP lease.
 */
bool network_if_is_connected(void);

#endif /* NETWORK_IF_H */
