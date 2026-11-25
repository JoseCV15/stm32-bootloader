#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "status.h"

typedef enum {
    TIMER_ONE_SHOT_MODE = 0,
    TIMER_PERIODIC_MODE
} timer_mode_t;

typedef enum {
    TIMER_STOPPED = 0,
    TIMER_RUNNING,
    TIMER_EXPIRED
} timer_state_t;

typedef struct {
    uint64_t start_time;      // when the timer started
    uint64_t timeout_ms;      // how long the timer should wait
    uint64_t elapsed_ms;      // how much timer has passed since the start
    timer_mode_t mode;
    timer_state_t state;
} timer_t;

/**
 * @brief Initialize the timer
 * @param timer Timer instance
 * @param timeout_ms Timeout in milliseconds
 * @param mode Timer operating mode
*/
status_t timer_init(timer_t *timer, uint64_t timeout_ms, timer_mode_t mode);
status_t timer_start(timer_t *timer);
void timer_stop(timer_t *timer);
status_t timer_reset(timer_t *timer);
status_t timer_set_timeout(timer_t *timer, uint64_t new_timeout);

/**
 * @brief Check if the timer has expired
 * On-shot: remains expired until restarted or reset
 * Periodic: returns true once when one or more periods elapsed
 */
bool timer_is_expired(timer_t *timer);
bool timer_is_running(const timer_t *timer);
uint64_t timer_get_elapsed(const timer_t *timer);
uint64_t timer_get_remaining(const timer_t *timer);

#endif 