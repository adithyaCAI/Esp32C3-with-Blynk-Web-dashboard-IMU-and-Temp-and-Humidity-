# Requirement traceability

Generated 2026-10-15T09:31:17.962388400+00:00
Specification frozen 2026-10-15T09:31:16.874Z

32 of 32 requirements have code recorded against them — 8 with tests passing, 0 with tests written but not passing, 24 implemented with no test at all. 0 have nothing recorded.

**Host unit tests.** No host unit-test results have been imported for this project; nothing here rests on host test evidence.

Everything below is derived from what the agent recorded with `requirement_trace` and from the frozen specification. A blank cell means nothing was recorded, which is not the same as nothing being needed.

## ESP32-C3 Sensor Hardware & Processing

gate: deferred · tasks: 5/5 done · recorded in: 9c114c52-e013-4db0-a607-5f4ab23d95e9

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `HW-001` | The firmware shall target the ESP32-C3-DevKitM-1 board. | inspection | The firmware build configuration identifies the target board as ESP32-C3-DevKitM-1. (expected: The configured board is ESP32-C3-DevKitM-1 and the target MCU is ESP32-C3.) | firmware/bsp/board.h<br>firmware/configs/app_config.h<br>sources/PRD.md | BOARD_NAME | — | verified · by inspection |
| `IF-001` | The firmware shall use GPIO2 as the MPU6050 I²C SCL signal. | inspection | The MPU6050 I²C SCL configuration assigns GPIO2. (expected: SCL is assigned to GPIO2.) | firmware/configs/app_config.h<br>firmware/platforms/esp32/sensor_platform.c | sensor_if_mpu6050_init | — | implemented, untested |
| `IF-002` | The firmware shall use GPIO3 as the MPU6050 I²C SDA signal. | inspection | The MPU6050 I²C SDA configuration assigns GPIO3. (expected: SDA is assigned to GPIO3.) | firmware/configs/app_config.h<br>firmware/platforms/esp32/sensor_platform.c | sensor_if_mpu6050_init | — | implemented, untested |
| `IF-003` | The firmware shall use GPIO1 (board pin 10) as the DHT22 data signal. | inspection | The DHT22 data configuration assigns GPIO1, identified as board pin 10. (expected: DHT22 data is assigned to GPIO1, and GPIO1 is identified as board pin 10.) | firmware/bsp/board.h<br>firmware/configs/app_config.h<br>sources/PRD.md | BOARD_NAME | — | verified · by inspection |
| `FR-001` | The firmware shall read temperature from the DHT22. | hardware / instrument | A successful DHT22 read produces a temperature measurement. (expected: The read path produces a numeric temperature measurement rather than a read failure or an absent value.) | firmware/interfaces/sensor_if.h<br>firmware/platforms/esp32/sensor_platform.c<br>firmware/services/sensor_service.c | sensor_if_dht22_read, sensor_if_mpu6050_read, sensor_service_task | — | verified · observed on hardware |
| `FR-002` | The firmware shall read humidity from the DHT22. | hardware / instrument | A successful DHT22 read produces a humidity measurement. (expected: The read path produces a numeric humidity measurement rather than a read failure or an absent value.) | firmware/interfaces/sensor_if.h<br>firmware/platforms/esp32/sensor_platform.c<br>firmware/services/sensor_service.c | sensor_if_dht22_read, sensor_if_mpu6050_read, sensor_service_task | — | verified · observed on hardware |
| `FR-003` | The firmware shall read MPU6050 acceleration and report the acceleration for each axis in m/s². | hardware / instrument | The MPU6050 measurement output reports numeric acceleration values for all three axes with m/s² units. (expected: Numeric X, Y, and Z acceleration values are reported, and each is identified in m/s².) | firmware/interfaces/sensor_if.h<br>firmware/platforms/esp32/sensor_platform.c<br>firmware/services/sensor_service.c | sensor_if_dht22_read, sensor_if_mpu6050_read, sensor_service_task | — | verified · observed on hardware |
| `FR-004` | The firmware shall calculate gyroscope-derived orientation angles in degrees. | host test | The orientation calculation returns gyroscope-derived angle values represented in degrees. (expected: The calculation produces numeric orientation angle values, and those values are represented in degrees.) | firmware/modules/orientation/orientation.c<br>firmware/modules/orientation/orientation.h | orientation_init, orientation_update | — | implemented, untested |
| `FR-005` | The firmware shall use accelerometer-assisted correction to reduce drift in orientation, including tilt axes where applicable. | host test | With the same simulated motion sequence, accelerometer-assisted tilt correction yields a smaller tilt error than gyroscope-only integration. (expected: The absolute tilt error of the accelerometer-assisted estimate is lower than the absolute tilt error of the gyroscope-only estimate.) | firmware/modules/orientation/orientation.c<br>firmware/modules/orientation/orientation.h | orientation_init, orientation_update | — | implemented, untested |
| `FR-007` | The firmware shall handle sensor read failures without terminating the measurement loop. | host test | A failed sensor read does not terminate the measurement loop, which performs a subsequent measurement-loop iteration. (expected: At least one measurement-loop iteration occurs after the failed sensor read.) | firmware/interfaces/sensor_if.h<br>firmware/platforms/esp32/sensor_platform.c<br>firmware/services/sensor_service.c | sensor_if_dht22_init, sensor_if_dht22_read, sensor_if_mpu6050_init, sensor_if_mpu6050_read, sensor_service_task | — | implemented, untested |
| `FR-010` | The firmware shall not use accelerometer correction as an absolute yaw reference, and shall allow the Z orientation to drift over time. | inspection | The orientation update does not use accelerometer-derived correction to establish yaw, and its yaw state remains capable of changing through gyroscope integration. (expected: No accelerometer-derived correction establishes an absolute yaw reference, and gyroscope integration can cause the yaw state to drift over time.) | firmware/modules/orientation/orientation.c | orientation_update | — | verified · by inspection |
| `IF-015` | The firmware shall treat the MPU6050 I²C address, sensor calibration procedure, and orientation-filter parameters as unspecified by this PRD. | inspection | The requirements and verification baseline assign no MPU6050 I²C address, calibration procedure, or orientation-filter parameter value as a PRD-specified value. (expected: None of the three items is specified as a value or procedure supplied by the PRD.) | firmware/bsp/board.h<br>firmware/configs/app_config.h<br>sources/PRD.md | BOARD_NAME | — | verified · by inspection |
| `FR-011` | The firmware shall initialize the MPU6050 before acquiring its sensor data. | host test | The MPU6050 initialization call occurs before the first MPU6050 sensor-data acquisition attempt. (expected: The MPU6050 initialization event precedes the first sensor-data acquisition attempt.) | firmware/interfaces/sensor_if.h<br>firmware/platforms/esp32/sensor_platform.c<br>firmware/services/sensor_service.c | sensor_if_dht22_init, sensor_if_dht22_read, sensor_if_mpu6050_init, sensor_if_mpu6050_read, sensor_service_task | — | implemented, untested |
| `FR-012` | The firmware shall initialize the DHT22 before attempting to read its measurements. | host test | The DHT22 initialization call occurs before the first temperature or humidity read attempt. (expected: The DHT22 initialization event precedes the first temperature and humidity read attempt.) | firmware/interfaces/sensor_if.h<br>firmware/platforms/esp32/sensor_platform.c<br>firmware/services/sensor_service.c | sensor_if_dht22_init, sensor_if_dht22_read, sensor_if_mpu6050_init, sensor_if_mpu6050_read, sensor_service_task | — | implemented, untested |

Observed on the board:
- `FR-001`, `FR-002`, `FR-003` — After configuring the user-confirmed SCL GPIO3/SDA GPIO2, a fresh deploy showed MPU6050 initialized at address 0x68 and valid readings. DHT22 reported 26.1 C and humidity 97.5–98.3%; accelerometer axes reported numeric values with m/s^2 units, latest (-0.128, -5.692, -8.040) m/s^2.
- `FR-010` — The implementation integrates gyroscope Z rate into yaw and applies accelerometer correction only to roll and pitch. During the 15-second stationary capture yaw moved from about -0.01 to -6.67 degrees.

Notes recorded with these traces:
- `HW-001`, `IF-003`, `IF-015` — The project build target is esp32c3 and BOARD_NAME is ESP32-C3-DevKitM-1; GPIO1 is configured for DHT22 and the board pin 10 mapping is declared. PRD.md states MPU6050 address, calibration procedure, and filter parameters are unspecified; 0x68 is only the implementation's default.
- `IF-001`, `IF-002` — Frozen requirements expect SCL GPIO2/SDA GPIO3. User confirmed actual wiring SCL GPIO3/SDA GPIO2 and explicitly chose firmware matching actual wiring. Configuration now follows the user's wiring, so frozen IF-001/IF-002 are not met; the conflict is preserved and reported, not redefined.
- `FR-004`, `FR-005` — Pure portable complementary filter returns roll/pitch/yaw in degrees. Host criteria remain implemented until the unit-test results are imported.
- `FR-007`, `FR-011`, `FR-012` — Host-checkable retry/loop and initialization-order criteria have no imported unit-test results; build and hardware observation do not verify them as host criteria.

## Wi-Fi Connectivity & Credential Management

gate: deferred · tasks: 3/3 done · recorded in: 9c114c52-e013-4db0-a607-5f4ab23d95e9

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `SEC-001` | The firmware shall receive Wi-Fi and Blynk credentials as configuration secrets. | inspection | The firmware obtains the Wi-Fi and Blynk credential values from configuration-secret inputs. (expected: Both Wi-Fi and Blynk credential values are supplied through configuration secrets.) | firmware/configs/app_config.h<br>firmware/configs/app_secrets.example.h<br>firmware/platforms/esp32/network_platform.c | APP_BLYNK_AUTH_TOKEN, APP_WIFI_PASSWORD, APP_WIFI_SSID, network_if_start | — | implemented, untested |
| `SEC-002` | Passwords and authentication tokens shall not be committed to a public source repository. | inspection | No password or authentication-token value appears in the commits of the public source repository. (expected: No password or authentication-token value appears in any inspected commit.) | .gitignore<br>firmware/configs/app_secrets.example.h | — | — | implemented, untested |
| `SEC-003` | The firmware shall keep credentials out of source files intended for sharing or version control. | inspection | No actual credential value appears in a source file intended for sharing or version control. (expected: No actual credential value appears in the inspected source files.) | .gitignore<br>firmware/configs/app_config.h<br>firmware/configs/app_secrets.example.h | APP_BLYNK_AUTH_TOKEN, APP_WIFI_PASSWORD, APP_WIFI_SSID | — | implemented, untested |
| `FR-008` | The firmware shall handle network disconnections without terminating the measurement loop. | hardware / instrument | The measurement-loop iteration count increases after the network is disconnected. (expected: At least one additional measurement-loop iteration is observed after the disconnection.) | firmware/platforms/esp32/network_platform.c<br>firmware/services/sensor_service.c | network_if_is_connected, sensor_service_task | — | verified · observed on hardware |
| `FR-009` | The firmware shall reconnect to the network and resume publishing when possible after a connectivity interruption. | hardware / instrument | After connectivity is restored, the device reconnects and a subsequent measurement is received by Blynk. (expected: The device reconnects, and Blynk receives at least one measurement after connectivity is restored.) | firmware/platforms/esp32/network_platform.c<br>firmware/services/sensor_service.c | network_event_handler, sensor_service_task | — | implemented, untested |

Observed on the board:
- `FR-008` — During an outage, network event logs reported disconnection while the sensor loop continued to emit temperature, humidity, acceleration and orientation snapshots at the 2-second report cadence. A fresh capture after final changes showed the configured placeholder-secret path deliberately stayed offline while measurements continued.

Notes recorded with these traces:
- `SEC-001` — Wi-Fi SSID/password are sourced from configuration-secret headers, and the firmware rejects placeholder Wi-Fi values and runs offline. Blynk token remains an example placeholder and is not consumed by runtime code in this category, so the Blynk credential portion of SEC-001 is unmet here.
- `SEC-002` — No version control repository or commit history is available in this workspace, so the public repository commit-history criterion cannot be verified.
- `SEC-003` — Inspected source uses only explicit SET_* placeholders; the actual secret header path is excluded in .gitignore. No local actual app_secrets.h was found in workspace search.
- `FR-009` — Wi-Fi disconnect events call esp_wifi_connect() asynchronously through the event loop. No configured real credentials or successful restored-IP/Blynk delivery have been observed, so recovery-and-receipt criterion is not verified.

## Blynk IoT Integration & Data Publishing

gate: not decided · tasks: 3/3 done · recorded in: 9c114c52-e013-4db0-a607-5f4ab23d95e9

| Req | Statement | Verified by | Acceptance criteria | Files | Symbols | Tests | Status |
|---|---|---|---|---|---|---|---|
| `IF-004` | The firmware shall use Blynk Template ID TMPL3VjCwAzY1 and template name ESPInterface. | inspection | The Blynk configuration defines Template ID `TMPL3VjCwAzY1` and template name `ESPInterface`. (expected: Template ID is `TMPL3VjCwAzY1` and template name is `ESPInterface`.) | firmware/configs/app_config.h<br>firmware/platforms/esp32/telemetry_platform.c | APP_BLYNK_TEMPLATE_ID, APP_BLYNK_TEMPLATE_NAME, telemetry_if_init | — | implemented, untested |
| `IF-013` | The firmware shall connect to Blynk IoT over Wi-Fi using the supplied configuration. | hardware / instrument | With the supplied configuration and available Wi-Fi, the firmware reaches the Blynk IoT connected state. (expected: The Blynk connection state is connected over Wi-Fi.) | firmware/platforms/esp32/network_platform.c<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/telemetry_service.c | network_if_is_connected, telemetry_if_init | — | implemented, untested |
| `FR-006` | The firmware shall publish all eight sensor values to their assigned Blynk datastreams every two seconds. | host test | Each publishing cycle emits all eight sensor values to their assigned Blynk datastreams, and consecutive cycles begin 2000 ms apart. (expected: Each cycle records one write for each assigned datastream V0–V7, and consecutive cycle start timestamps differ by 2000 ms.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-005` | The firmware shall publish temperature to Blynk virtual pin V0. | host test | Publishing a temperature reading records that reading on Blynk virtual pin V0. (expected: The temperature reading is written to V0.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-006` | The firmware shall publish humidity to Blynk virtual pin V1. | host test | Publishing a humidity reading records that reading on Blynk virtual pin V1. (expected: The humidity reading is written to V1.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-007` | The firmware shall publish accelerometer X to Blynk virtual pin V2. | host test | Publishing an accelerometer X reading records that reading on Blynk virtual pin V2. (expected: The accelerometer X reading is written to V2.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-008` | The firmware shall publish accelerometer Y to Blynk virtual pin V3. | host test | Publishing an accelerometer Y reading records that reading on Blynk virtual pin V3. (expected: The accelerometer Y reading is written to V3.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-009` | The firmware shall publish accelerometer Z to Blynk virtual pin V4. | host test | Publishing an accelerometer Z reading records that reading on Blynk virtual pin V4. (expected: The accelerometer Z reading is written to V4.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-010` | The firmware shall publish gyroscope orientation X to Blynk virtual pin V5. | host test | Publishing a gyroscope orientation X reading records that reading on Blynk virtual pin V5. (expected: The gyroscope orientation X reading is written to V5.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-011` | The firmware shall publish gyroscope orientation Y to Blynk virtual pin V6. | host test | Publishing a gyroscope orientation Y reading records that reading on Blynk virtual pin V6. (expected: The gyroscope orientation Y reading is written to V6.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `IF-012` | The firmware shall publish gyroscope orientation Z to Blynk virtual pin V7. | host test | Publishing a gyroscope orientation Z reading records that reading on Blynk virtual pin V7. (expected: The gyroscope orientation Z reading is written to V7.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/modules/telemetry/telemetry_mapping.h<br>firmware/platforms/esp32/telemetry_platform.c<br>firmware/services/sensor_service.c<br>firmware/services/telemetry_service.c | sensor_service_task, telemetry_if_publish, telemetry_mapping_build_batch, telemetry_service_publish | — | implemented, untested |
| `NFR-001` | The firmware shall maintain the two-second sampling and publishing cadence during normal operation. | host test | During normal operation, consecutive sensor-sampling events and consecutive publishing cycles each occur 2000 ms apart. (expected: Every pair of consecutive sample timestamps differs by 2000 ms, and every pair of consecutive publishing-cycle timestamps differs by 2000 ms.) | firmware/configs/app_config.h<br>firmware/modules/telemetry/telemetry_mapping.c<br>firmware/services/sensor_service.c | sensor_service_task, telemetry_mapping_build_batch | — | implemented, untested |
| `IF-014` | The firmware shall send values to the existing dashboard and datastreams; it shall not create the dashboard. | inspection | The firmware's Blynk code writes values to the existing datastreams and contains no operation that creates a dashboard. (expected: The code sends values to the existing datastreams, and no dashboard-creation operation is present.) | firmware/modules/telemetry/telemetry_mapping.c<br>firmware/platforms/esp32/telemetry_platform.c | telemetry_if_publish, telemetry_mapping_build_batch | — | implemented, untested |

Notes recorded with these traces:
- `IF-004` — Configuration declares Template ID TMPL3VjCwAzY1 and template name ESPInterface. Build passed. Hardware unavailable for this category checkpoint.
- `IF-013` — Connection requires configured Wi-Fi and Blynk secret headers. Existing local secret file is absent and the committed example contains placeholders only; connected state was not verifiable because the board was unavailable.
- `FR-006`, `IF-005`, `IF-006`, `IF-007`, `IF-008`, `IF-009`, `IF-010`, `IF-011`, `IF-012` — Pure mapping assigns temperature/humidity/acceleration/orientation values to V0-V7. Sensor/publish scheduling uses 2000 ms configuration, and HTTPS request work is queued to a separate task. Host-checkable requirements remain implemented until unit tests run; no board was available for newly-observable behavior.
- `NFR-001` — Configured cadence is 2000 ms; host verification requires the later unit-test run.
- `IF-014` — Firmware uses Blynk's batch datastream update endpoint and virtual pin parameters V0-V7; no dashboard creation code is present. This inspection criterion was not independently re-read after final implementation.

## Gaps

Listed rather than omitted. A matrix showing only what was built cannot be told apart from one showing full coverage.

### Host tests you could write (15)

These have a `host` acceptance criterion and cite no test. They are tested at the end, from Artifacts -> Unit Tests.

- `FR-004` (ESP32-C3 Sensor Hardware & Processing)
- `FR-005` (ESP32-C3 Sensor Hardware & Processing)
- `FR-007` (ESP32-C3 Sensor Hardware & Processing)
- `FR-011` (ESP32-C3 Sensor Hardware & Processing)
- `FR-012` (ESP32-C3 Sensor Hardware & Processing)
- `FR-006` (Blynk IoT Integration & Data Publishing)
- `IF-005` (Blynk IoT Integration & Data Publishing)
- `IF-006` (Blynk IoT Integration & Data Publishing)
- `IF-007` (Blynk IoT Integration & Data Publishing)
- `IF-008` (Blynk IoT Integration & Data Publishing)
- `IF-009` (Blynk IoT Integration & Data Publishing)
- `IF-010` (Blynk IoT Integration & Data Publishing)
- `IF-011` (Blynk IoT Integration & Data Publishing)
- `IF-012` (Blynk IoT Integration & Data Publishing)
- `NFR-001` (Blynk IoT Integration & Data Publishing)

### Acceptance needs non-host evidence (17)

No `host` acceptance criterion is recorded. Host tests cannot settle these acceptance claims, but implementation calculations and failure recovery can still need regression tests. Each line identifies the required acceptance evidence; do not substitute a host result for it.

- `HW-001` — checked by inspection · _ESP32-C3 Sensor Hardware & Processing_
- `IF-001` — checked by inspection · _ESP32-C3 Sensor Hardware & Processing_
- `IF-002` — checked by inspection · _ESP32-C3 Sensor Hardware & Processing_
- `IF-003` — checked by inspection · _ESP32-C3 Sensor Hardware & Processing_
- `FR-001` — checked by hardware / instrument · _ESP32-C3 Sensor Hardware & Processing_
- `FR-002` — checked by hardware / instrument · _ESP32-C3 Sensor Hardware & Processing_
- `FR-003` — checked by hardware / instrument · _ESP32-C3 Sensor Hardware & Processing_
- `FR-010` — checked by inspection · _ESP32-C3 Sensor Hardware & Processing_
- `IF-015` — checked by inspection · _ESP32-C3 Sensor Hardware & Processing_
- `SEC-001` — checked by inspection · _Wi-Fi Connectivity & Credential Management_
- `SEC-002` — checked by inspection · _Wi-Fi Connectivity & Credential Management_
- `SEC-003` — checked by inspection · _Wi-Fi Connectivity & Credential Management_
- `FR-008` — checked by hardware / instrument · _Wi-Fi Connectivity & Credential Management_
- `FR-009` — checked by hardware / instrument · _Wi-Fi Connectivity & Credential Management_
- `IF-004` — checked by inspection · _Blynk IoT Integration & Data Publishing_
- `IF-013` — checked by hardware / instrument · _Blynk IoT Integration & Data Publishing_
- `IF-014` — checked by inspection · _Blynk IoT Integration & Data Publishing_

### Board checks owed (2)

The counterpart of the host-test worklist, for whoever has the hardware. Each of these has a `hardware` or `target` criterion and nothing recorded as observed, so no host test and no reading of the code will ever close it. Per requirement rather than per category on purpose: a category usually mixes these with criteria that are settled by reading, and "this category needs hardware" sends someone to re-check things that do not.

- `FR-009` (Wi-Fi Connectivity & Credential Management)
- `IF-013` (Blynk IoT Integration & Data Publishing)

### Implemented, never tested (24)

Code exists and no test is recorded against it.

- `IF-001` (ESP32-C3 Sensor Hardware & Processing)
- `IF-002` (ESP32-C3 Sensor Hardware & Processing)
- `FR-004` (ESP32-C3 Sensor Hardware & Processing)
- `FR-005` (ESP32-C3 Sensor Hardware & Processing)
- `FR-007` (ESP32-C3 Sensor Hardware & Processing)
- `FR-011` (ESP32-C3 Sensor Hardware & Processing)
- `FR-012` (ESP32-C3 Sensor Hardware & Processing)
- `SEC-001` (Wi-Fi Connectivity & Credential Management)
- `SEC-002` (Wi-Fi Connectivity & Credential Management)
- `SEC-003` (Wi-Fi Connectivity & Credential Management)
- `FR-009` (Wi-Fi Connectivity & Credential Management)
- `IF-004` (Blynk IoT Integration & Data Publishing)
- `IF-013` (Blynk IoT Integration & Data Publishing)
- `FR-006` (Blynk IoT Integration & Data Publishing)
- `IF-005` (Blynk IoT Integration & Data Publishing)
- `IF-006` (Blynk IoT Integration & Data Publishing)
- `IF-007` (Blynk IoT Integration & Data Publishing)
- `IF-008` (Blynk IoT Integration & Data Publishing)
- `IF-009` (Blynk IoT Integration & Data Publishing)
- `IF-010` (Blynk IoT Integration & Data Publishing)
- `IF-011` (Blynk IoT Integration & Data Publishing)
- `IF-012` (Blynk IoT Integration & Data Publishing)
- `NFR-001` (Blynk IoT Integration & Data Publishing)
- `IF-014` (Blynk IoT Integration & Data Publishing)
