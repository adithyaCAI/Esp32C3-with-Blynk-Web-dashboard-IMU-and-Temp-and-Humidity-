/* Portable sensor acquisition task entry point. */
#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <stdint.h>

/**
 * @brief Run sensor reads and orientation processing until the task is stopped.
 * @param[in] argument Unused task argument reserved for OSAL task creation.
 */
void sensor_service_task(void *argument);

#endif /* SENSOR_SERVICE_H */
