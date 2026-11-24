#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/ble.h>
#include <zmk/battery.h>
#include <raw_hid/raw_hid.h>

static uint8_t periph_bat = 255;

static void send_status_report(void) {
    uint8_t report[32] = {0};
    report[0] = 0x01; // Packet header
    report[1] = zmk_battery_state_of_charge();           // Left (Central) %
    report[2] = periph_bat;                              // Right (Peripheral) %
    report[3] = zmk_ble_active_profile_index();          // Profile index (0-4)
    report[4] = zmk_ble_active_profile_is_connected() ? 1 : 0;
    report[5] = zmk_keymap_highest_layer_active();       // Active layer index

    raw_hid_send(report, sizeof(report));
}

static int status_listener(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *p_bat = as_zmk_peripheral_battery_state_changed(eh);
    if (p_bat) {
        periph_bat = p_bat->state_of_charge;
    }
    send_status_report();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(hid_status, status_listener);
ZMK_SUBSCRIPTION(hid_status, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(hid_status, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(hid_status, zmk_battery_state_changed);
ZMK_SUBSCRIPTION(hid_status, zmk_peripheral_battery_state_changed);
