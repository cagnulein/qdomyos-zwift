/*
 * QZ ProForm Wi-Fi -> ANT+ FE-C bridge
 * XIAO nRF52840 side
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This application is intentionally implemented on top of Garmin's ANT for
 * nRF Connect SDK add-on. It does not embed or redistribute the ANT+ network
 * key; ant_plus_key_set() is provided by the licensed ANT SDK dependency.
 *
 * UART protocol:
 *   ESP32 -> XIAO:
 *     M,<power_w>,<cadence_rpm>,<speed_mm_s>,<distance_m>,<elapsed_quarter_s>
 *
 *   XIAO -> ESP32:
 *     T4,<target_power_quarter_watts>
 *
 * The ANT FE-C target power page (49 / 0x31) encodes target power in 0.25 W.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ant_error.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_host_init.h"
#include "ant_channel_config.h"
#include "ant_key_manager.h"

// -----------------------------------------------------------------------------
// Hardware / ANT+ FE-C configuration
// -----------------------------------------------------------------------------

#define UART_NODE DT_NODELABEL(uart0)

static const struct device *const bridge_uart = DEVICE_DT_GET(UART_NODE);

static const uint8_t FEC_CHANNEL = 0;
static const uint8_t FEC_NETWORK = 0;
static const uint16_t FEC_DEVICE_NUMBER = 12345;
static const uint8_t FEC_DEVICE_TYPE = 0x11;       // ANT+ Fitness Equipment
static const uint8_t FEC_TRANSMISSION_TYPE = 0x05;
static const uint8_t FEC_RF_FREQUENCY = 0x39;      // 2457 MHz
static const uint16_t FEC_CHANNEL_PERIOD = 8192;   // 4.00 Hz

static const uint8_t FEC_EQUIPMENT_TRAINER = 25;
static const uint8_t FEC_STATE_IN_USE = 3;

// FE-C pages used by this bridge.
static const uint8_t PAGE_GENERAL_FE_DATA = 16;
static const uint8_t PAGE_SPECIFIC_TRAINER = 25;
static const uint8_t PAGE_TARGET_POWER = 49;
static const uint8_t PAGE_FE_CAPABILITIES = 54;
static const uint8_t PAGE_COMMAND_STATUS = 71;
static const uint8_t PAGE_REQUEST = 70;
static const uint8_t PAGE_MANUFACTURER_INFO = 80;
static const uint8_t PAGE_PRODUCT_INFO = 81;

// We implement Target Power / ERG. Do not advertise Basic Resistance or
// Simulation Mode until those commands are actually translated by the ESP32.
static const uint8_t FEC_CAP_TARGET_POWER = 0x02;

// Placeholder ANT+ common-page identification.
// Replace with assigned values if this becomes a certified product.
static const uint8_t HW_REVISION = 1;
static const uint16_t MANUFACTURER_ID = 255;
static const uint16_t MODEL_NUMBER = 1;
static const uint8_t SW_REVISION_MAJOR = 1;
static const uint8_t SW_REVISION_MINOR = 0;
static const uint32_t SERIAL_NUMBER = 1;

// -----------------------------------------------------------------------------
// Runtime state
// -----------------------------------------------------------------------------

struct bridge_metrics {
    uint16_t power_w;
    uint8_t cadence_rpm;
    uint16_t speed_mm_s;
    uint32_t distance_m;
    uint8_t elapsed_quarter_s;
};

static volatile struct bridge_metrics metrics;

static uint8_t trainer_event_count;
static uint16_t accumulated_power;

static uint16_t current_target_x4;
static uint8_t command_sequence = 0xFF;
static uint8_t last_command_id = 0xFF;
static uint8_t last_command_status = 0xFF;

// Target received from Garmin and waiting to be forwarded to the ESP32.
// -1 means no pending target.
static atomic_t pending_target_x4 = ATOMIC_INIT(-1);

static uint8_t requested_page;
static uint8_t requested_transmissions;
static bool requested_ack;

static uint32_t tx_count;

// -----------------------------------------------------------------------------
// Small endian helpers
// -----------------------------------------------------------------------------

static inline void put_u16_le(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)(value & 0xFF);
    dst[1] = (uint8_t)((value >> 8) & 0xFF);
}

static inline void put_u32_le(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)(value & 0xFF);
    dst[1] = (uint8_t)((value >> 8) & 0xFF);
    dst[2] = (uint8_t)((value >> 16) & 0xFF);
    dst[3] = (uint8_t)((value >> 24) & 0xFF);
}

static inline uint16_t get_u16_le(const uint8_t *src)
{
    return (uint16_t)src[0] | ((uint16_t)src[1] << 8);
}

// -----------------------------------------------------------------------------
// UART bridge
// -----------------------------------------------------------------------------

static void uart_write_string(const char *text)
{
    while (*text != '\0') {
        uart_poll_out(bridge_uart, (unsigned char)*text++);
    }
}

static void publish_pending_target_to_esp32(void)
{
    const atomic_val_t raw = atomic_set(&pending_target_x4, -1);

    if (raw < 0) {
        return;
    }

    char line[32];
    snprintf(line, sizeof(line), "T4,%u\n", (unsigned int)raw);
    uart_write_string(line);
}

static void process_metrics_line(const char *line)
{
    unsigned int power;
    unsigned int cadence;
    unsigned int speed;
    unsigned long distance;
    unsigned int elapsed;

    if (sscanf(line, "M,%u,%u,%u,%lu,%u",
               &power, &cadence, &speed, &distance, &elapsed) != 5) {
        return;
    }

    struct bridge_metrics next = {
        .power_w = (uint16_t)MIN(power, 4095U),
        .cadence_rpm = (uint8_t)MIN(cadence, 254U),
        .speed_mm_s = (uint16_t)MIN(speed, 65535U),
        .distance_m = (uint32_t)distance,
        .elapsed_quarter_s = (uint8_t)elapsed,
    };

    /*
     * All members are naturally aligned <= 32-bit values on Cortex-M4.
     * Copy field-by-field so the ANT callback always sees valid scalar values.
     */
    metrics.power_w = next.power_w;
    metrics.cadence_rpm = next.cadence_rpm;
    metrics.speed_mm_s = next.speed_mm_s;
    metrics.distance_m = next.distance_m;
    metrics.elapsed_quarter_s = next.elapsed_quarter_s;
}

static void service_uart(void)
{
    static char line[96];
    static size_t pos;
    unsigned char c;

    while (uart_poll_in(bridge_uart, &c) == 0) {
        if (c == '\n') {
            line[pos] = '\0';
            if (pos > 0) {
                process_metrics_line(line);
            }
            pos = 0;
            continue;
        }

        if (c == '\r') {
            continue;
        }

        if (pos < sizeof(line) - 1) {
            line[pos++] = (char)c;
        } else {
            pos = 0;
        }
    }
}

// -----------------------------------------------------------------------------
// ANT+ FE-C page builders
// -----------------------------------------------------------------------------

static void build_page16(uint8_t page[8])
{
    const struct bridge_metrics m = {
        .power_w = metrics.power_w,
        .cadence_rpm = metrics.cadence_rpm,
        .speed_mm_s = metrics.speed_mm_s,
        .distance_m = metrics.distance_m,
        .elapsed_quarter_s = metrics.elapsed_quarter_s,
    };

    memset(page, 0xFF, 8);

    page[0] = PAGE_GENERAL_FE_DATA;
    page[1] = FEC_EQUIPMENT_TRAINER;
    page[2] = m.elapsed_quarter_s;
    page[3] = (uint8_t)(m.distance_m & 0xFF);
    put_u16_le(&page[4], m.speed_mm_s);
    page[6] = 0xFF; // heart rate not supplied by this bridge

    /*
     * Page 16 byte 7:
     *   low nibble: capabilities (bit 2 = distance traveled enabled)
     *   high nibble: FE state
     */
    page[7] = (uint8_t)((FEC_STATE_IN_USE << 4) | 0x04);
}

static void build_page25(uint8_t page[8])
{
    const uint16_t power = metrics.power_w;
    const uint8_t cadence = metrics.cadence_rpm;

    accumulated_power = (uint16_t)(accumulated_power + power);

    memset(page, 0, 8);

    page[0] = PAGE_SPECIFIC_TRAINER;
    page[1] = trainer_event_count++;
    page[2] = cadence;
    put_u16_le(&page[3], accumulated_power);

    // Instantaneous power uses 12 bits.
    page[5] = (uint8_t)(power & 0xFF);
    page[6] = (uint8_t)((power >> 8) & 0x0F);

    // Upper nibble = FE state, lower nibble = target power limit flags.
    page[7] = (uint8_t)(FEC_STATE_IN_USE << 4);
}

static void build_page49(uint8_t page[8])
{
    memset(page, 0xFF, 8);
    page[0] = PAGE_TARGET_POWER;
    put_u16_le(&page[6], current_target_x4);
}

static void build_page54(uint8_t page[8])
{
    memset(page, 0xFF, 8);
    page[0] = PAGE_FE_CAPABILITIES;

    // Maximum resistance unknown/not applicable for this Wi-Fi bridge.
    page[5] = 0xFF;
    page[6] = 0xFF;

    // Target Power supported, Basic Resistance and Simulation not advertised.
    page[7] = FEC_CAP_TARGET_POWER;
}

static void build_page71(uint8_t page[8])
{
    memset(page, 0xFF, 8);
    page[0] = PAGE_COMMAND_STATUS;
    page[1] = last_command_id;
    page[2] = command_sequence;
    page[3] = last_command_status;

    if (last_command_id == PAGE_TARGET_POWER) {
        // FE-C command-status response data for page 49 keeps the target in
        // response bytes 2..3 (wire bytes 6..7).
        put_u16_le(&page[6], current_target_x4);
    }
}

static void build_page80(uint8_t page[8])
{
    memset(page, 0xFF, 8);
    page[0] = PAGE_MANUFACTURER_INFO;
    page[3] = HW_REVISION;
    put_u16_le(&page[4], MANUFACTURER_ID);
    put_u16_le(&page[6], MODEL_NUMBER);
}

static void build_page81(uint8_t page[8])
{
    memset(page, 0xFF, 8);
    page[0] = PAGE_PRODUCT_INFO;
    page[2] = SW_REVISION_MINOR;
    page[3] = SW_REVISION_MAJOR;
    put_u32_le(&page[4], SERIAL_NUMBER);
}

static bool build_requested_page(uint8_t page_number, uint8_t page[8])
{
    switch (page_number) {
    case PAGE_GENERAL_FE_DATA:
        build_page16(page);
        return true;
    case PAGE_SPECIFIC_TRAINER:
        build_page25(page);
        return true;
    case PAGE_TARGET_POWER:
        build_page49(page);
        return true;
    case PAGE_FE_CAPABILITIES:
        build_page54(page);
        return true;
    case PAGE_COMMAND_STATUS:
        build_page71(page);
        return true;
    case PAGE_MANUFACTURER_INFO:
        build_page80(page);
        return true;
    case PAGE_PRODUCT_INFO:
        build_page81(page);
        return true;
    default:
        return false;
    }
}

// -----------------------------------------------------------------------------
// ANT TX scheduler
// -----------------------------------------------------------------------------

static void send_next_fec_page(void)
{
    uint8_t page[8];
    bool use_ack = false;

    if (requested_page != 0 && requested_transmissions > 0) {
        if (build_requested_page(requested_page, page)) {
            use_ack = requested_ack;
        } else {
            requested_transmissions = 0;
            requested_page = 0;
            build_page25(page);
        }

        if (requested_transmissions > 0) {
            requested_transmissions--;
        }

        if (requested_transmissions == 0) {
            requested_page = 0;
            requested_ack = false;
        }
    } else {
        /*
         * Normal FE-C broadcast pattern:
         *  - pages 16 and 25 carry live trainer data
         *  - page 54 is inserted periodically to advertise ERG capability
         *  - common pages 80/81 are sent periodically for identification
         */
        const uint32_t slot = tx_count++ & 0x3F;

        if (slot == 60) {
            build_page54(page);
        } else if (slot == 61) {
            build_page80(page);
        } else if (slot == 62) {
            build_page81(page);
        } else if ((slot & 0x01) == 0) {
            build_page16(page);
        } else {
            build_page25(page);
        }
    }

    ant_err_t err;

    if (use_ack) {
        err = ant_acknowledge_message_tx(FEC_CHANNEL, sizeof(page), page);
        if (err == 0) {
            return;
        }
        // If an acknowledged response cannot be started, keep FE-C alive by
        // falling back to the normal broadcast path.
    }

    (void)ant_broadcast_message_tx(FEC_CHANNEL, sizeof(page), page);
}

// -----------------------------------------------------------------------------
// ANT RX: Garmin FE-C commands
// -----------------------------------------------------------------------------

static void handle_page49_target_power(const uint8_t payload[8])
{
    const uint16_t target_x4 = get_u16_le(&payload[6]);

    last_command_id = PAGE_TARGET_POWER;
    command_sequence = (command_sequence == 0xFF) ? 0 : (uint8_t)(command_sequence + 1);

    // FE-C target power range: 0..4000 W in 0.25 W units.
    if (target_x4 > 16000U) {
        last_command_status = 3; // rejected
        return;
    }

    current_target_x4 = target_x4;
    last_command_status = 0; // pass

    // Main loop forwards this to the ESP32 as T4,<quarter-watts>.
    atomic_set(&pending_target_x4, (atomic_val_t)target_x4);
}

static void handle_page70_request(const uint8_t payload[8])
{
    // Common page 70:
    // byte 5 = transmission response, byte 6 = requested page,
    // byte 7 = command type (1 = request data page).
    if (payload[7] != 0x01) {
        return;
    }

    const uint8_t requested = payload[6];
    uint8_t count = (uint8_t)(payload[5] & 0x7F);

    // 0x80 means "transmit until successful acknowledge". For this small
    // bridge, one acknowledged response is enough; regular count=0 is also
    // normalized to one response.
    requested_ack = (payload[5] & 0x80) != 0;
    if (count == 0) {
        count = 1;
    }

    requested_page = requested;
    requested_transmissions = count;
}

static void ant_evt_handler(ant_evt_t *evt)
{
    if (evt->channel != FEC_CHANNEL) {
        return;
    }

    switch (evt->event) {
    case EVENT_TX:
        send_next_fec_page();
        break;

    case EVENT_RX:
        if (evt->message.ANT_MESSAGE_ucMesgID == MESG_BROADCAST_DATA_ID ||
            evt->message.ANT_MESSAGE_ucMesgID == MESG_ACKNOWLEDGED_DATA_ID ||
            evt->message.ANT_MESSAGE_ucMesgID == MESG_BURST_DATA_ID) {

            const uint8_t *payload = evt->message.ANT_MESSAGE_aucPayload;

            if (payload[0] == PAGE_TARGET_POWER) {
                handle_page49_target_power(payload);
            } else if (payload[0] == PAGE_REQUEST) {
                handle_page70_request(payload);
            } else {
                /*
                 * Basic Resistance (48), Wind (50) and Track Resistance (51)
                 * are deliberately not advertised in page 54 and are ignored.
                 * This keeps this first implementation focused on ERG mode.
                 */
            }
        }
        break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// ANT setup
// -----------------------------------------------------------------------------

static int setup_ant_fec(void)
{
    ant_err_t err = ant_init();
    if (err) {
        return err;
    }

    err = ant_cb_register(&ant_evt_handler);
    if (err) {
        return err;
    }

    // The licensed ANT add-on provides the ANT+ key internally.
    err = ant_plus_key_set(FEC_NETWORK);
    if (err) {
        return err;
    }

    const ant_channel_config_t channel = {
        .channel_number = FEC_CHANNEL,
        .channel_type = CHANNEL_TYPE_MASTER,
        .ext_assign = 0x00,
        .rf_freq = FEC_RF_FREQUENCY,
        .transmission_type = FEC_TRANSMISSION_TYPE,
        .device_type = FEC_DEVICE_TYPE,
        .device_number = FEC_DEVICE_NUMBER,
        .channel_period = FEC_CHANNEL_PERIOD,
        .network_number = FEC_NETWORK,
    };

    err = ant_channel_init(&channel);
    if (err) {
        return err;
    }

    // Prime the first frame before opening the channel, like Garmin's
    // ant_broadcast_tx sample.
    send_next_fec_page();

    return ant_channel_open(FEC_CHANNEL);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main(void)
{
    if (!device_is_ready(bridge_uart)) {
        k_oops();
        return 0;
    }

    if (setup_ant_fec() != 0) {
        k_oops();
        return 0;
    }

    for (;;) {
        service_uart();
        publish_pending_target_to_esp32();
        k_sleep(K_MSEC(2));
    }

    return 0;
}
