#ifndef BOARD_H
#define BOARD_H

/* Board facts for the ESP32-C3-DevKitM-1 and attached sensors. */

/* Board support — the ONE place that records what is physically wired to what.
 *
 * This header is DECLARATIVE. It records board facts (which pin, which active level, which
 * bus) and pure helpers over them. It performs no I/O and includes no vendor SDK header:
 * the code that actually drives a pin lives in platforms/<vendor>/.
 *
 * Raw pin numbers stay in configs/app_config.h, because the Firmware Topology editor reads
 * and writes the APP_* macros there. This header turns them into named board facts so that
 * app/ and services/ never handle a GPIO number and never guess a polarity.
 */

#include <stdbool.h>
#include <stdint.h>

#include "app_config.h"

#define BOARD_NAME "ESP32-C3-DevKitM-1"
#define BOARD_MPU6050_SCL_GPIO APP_MPU6050_SCL_GPIO
#define BOARD_MPU6050_SDA_GPIO APP_MPU6050_SDA_GPIO
#define BOARD_DHT22_DATA_GPIO APP_DHT22_DATA_GPIO

/* Board identity, for logs and diagnostics. */
const char *board_name(void);

/* ---- Indicator LED -------------------------------------------------------------------
 * An LED's ACTIVE level is a board fact, not a preference: an active-low LED lights on 0.
 * Getting it backwards inverts every state at once and reads like several unrelated bugs.
 * It is knowable only from the schematic or board user manual — never from the pin number.
 *
 * So there is deliberately NO default here. Declaring a pin without its polarity warns; the
 * helper below simply does not exist until the polarity is declared, so any code that tries
 * to use it fails at the call site with a clear message rather than silently inverting.
 */
#ifdef APP_LED_GPIO
#ifdef BOARD_LED_ACTIVE_LEVEL

/* The raw pin level that expresses the logical state `on`. Pure — performs no I/O. */
static inline uint32_t board_led_level(bool on)
{
    return on ? (uint32_t)(BOARD_LED_ACTIVE_LEVEL)
              : (uint32_t)(!(BOARD_LED_ACTIVE_LEVEL));
}

#else
#warning "APP_LED_GPIO is defined but BOARD_LED_ACTIVE_LEVEL is not, so board_led_level() is unavailable. An LED's active level is a board fact and cannot be inferred from the pin number: add '#define BOARD_LED_ACTIVE_LEVEL 1' (active-high) or 0 (active-low) here, taking the value from the board schematic or user manual."
#endif /* BOARD_LED_ACTIVE_LEVEL */
#endif /* APP_LED_GPIO */

/* Add further board facts the same way — bus pins, address straps, button polarity, e.g.:
 *
 *   #define BOARD_I2C_SDA_GPIO   APP_I2C_SDA_GPIO
 *   #define BOARD_I2C_SCL_GPIO   APP_I2C_SCL_GPIO
 *   #define BOARD_I2C_FREQ_HZ    APP_I2C_FREQ_HZ
 */

#endif /* BOARD_H */
