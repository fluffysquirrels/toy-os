#pragma once

#include "stdint.h"
#include "syscalls.h"

typedef uint32_t timer_id_t;
struct timer_callback_data_t {
  timer_id_t timer_id;
};
typedef void (*timer_callback_t)(struct timer_callback_data_t data, void *callback_state);

err_t timer_queue(
  duration_t d,
  timer_callback_t callback,
  void *callback_state,
  timer_id_t *out_timer_id);

// Returns nanoseconds since some offset.
time timer_systemnow();

void timer_do_expired_callbacks();

time timer_get_earliest_deadline();

void timer_init();

// TODO: Make this static, callers should use higher level functions.
void timer_set_arch_timeout(duration_t d);
