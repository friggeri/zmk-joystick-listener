/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_joystick_listener

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct joystick_data {
  int16_t x;
  int16_t y;
  int16_t prev_x;
  int16_t prev_y;
};

static void input_handler(struct joystick_data *data, struct input_event *evt) {
  if (evt->type != INPUT_EV_ABS) {
    return;
  }

  switch (evt->code) {
  case INPUT_ABS_X:
    data->x = evt->value;
    break;
  case INPUT_ABS_Y:
    data->y = evt->value;
    break;
  }

  if (evt->sync && ((data->prev_x != data->x) || data->prev_y != data->y)) {
    data->prev_x = data->x;
    data->prev_y = data->y;
    zmk_hid_joystick_set((int8_t)data->x, (int8_t)data->y, 0);
    zmk_endpoints_send_joystick_report();
  }
}

#define JOYSTICK_INST(n)                                                       \
  static struct joystick_data data_##n = {};                                   \
  void input_handler_##n(struct input_event *evt) {                            \
    input_handler(&data_##n, evt);                                             \
  }                                                                            \
  INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_INST_PHANDLE(n, device)),             \
                        input_handler_##n);

DT_INST_FOREACH_STATUS_OKAY(JOYSTICK_INST)