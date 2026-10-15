/* Portable network service delegates radio operation to the platform contract. */
#include "network_service.h"

#include "network_if.h"

bool network_service_start(void)
{
    return network_if_start();
}

bool network_service_is_connected(void)
{
    return network_if_is_connected();
}
