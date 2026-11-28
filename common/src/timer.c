#include "timer.h"
#include "systick.h"

#define TIMER_MAX_TIMEOUT_MS    0x7FFFFFFFUL

static uint32_t timer_calculate_elapsed(const timer_t *timer) {
    return (systick_get_tick() - timer->start_time);
}

static bool timer_is_valid(const timer_t *timer)
{
    if (!timer) return false;
    if ((timer->timeout_ms == 0U) || (timer->timeout_ms > TIMER_MAX_TIMEOUT_MS)) {
        return false;
    }
    if ((timer->mode != TIMER_ONE_SHOT_MODE) && (timer->mode != TIMER_PERIODIC_MODE)){
        return false;
    }
    return true;
}

status_t timer_init(timer_t *timer, uint32_t timeout_ms, timer_mode_t mode)
{
    if (!timer) return STATUS_INVALID_ARGUMENT;
    if ((timeout_ms == 0U) || (timeout_ms > TIMER_MAX_TIMEOUT_MS)) {
        return STATUS_INVALID_ARGUMENT;
    }
    if ((mode != TIMER_ONE_SHOT_MODE) && (mode != TIMER_PERIODIC_MODE)) {
        return STATUS_INVALID_ARGUMENT;
    }

    timer->start_time = 0U;
    timer->timeout_ms = timeout_ms;
    timer->elapsed_ms = 0U;
    timer->mode = mode;
    timer->state = TIMER_STOPPED;

    return STATUS_OK;
}


status_t timer_start(timer_t *timer)
{
    if (!timer_is_valid(timer)) {
        return STATUS_INVALID_ARGUMENT;
    }
    timer->start_time = systick_get_tick();
    timer->elapsed_ms = 0U;
    timer->state = TIMER_RUNNING;

    return STATUS_OK;
}

void timer_stop(timer_t *timer)
{
    if (!timer || timer->state != TIMER_RUNNING) {
        return;
    }
    timer->state = TIMER_STOPPED;
    timer->elapsed_ms = timer_calculate_elapsed(timer);
}


status_t timer_reset(timer_t *timer)
{
    return timer_start(timer);
}

status_t timer_set_timeout(timer_t *timer, uint32_t new_timeout)
{
    if (!timer || (new_timeout == 0U) || (new_timeout > TIMER_MAX_TIMEOUT_MS)) {
        return STATUS_INVALID_ARGUMENT;
    }

    timer->timeout_ms = new_timeout;

    // Restart if the timer was running
    if (timer->state == TIMER_RUNNING) {
        timer->start_time = systick_get_tick();
        timer->elapsed_ms = 0U;
    }
    else if (timer->state == TIMER_EXPIRED) {
        timer->elapsed_ms = 0U;
        timer->state = TIMER_STOPPED;
    }

    return STATUS_OK;
}

bool timer_is_expired(timer_t *timer)
{
    uint32_t elapsed;
    uint32_t periods;

    if (!timer_is_valid(timer) || timer->state != TIMER_RUNNING) {
        return (timer != NULL && timer->state == TIMER_EXPIRED);
    }

    elapsed = timer_calculate_elapsed(timer);

    if (elapsed < timer->timeout_ms) {
        return false;
    }

    if (timer->mode == TIMER_ONE_SHOT_MODE) {
        timer->elapsed_ms = timer->timeout_ms;
        timer->state = TIMER_EXPIRED;
        return true;
    }

    periods = elapsed / timer->timeout_ms;
    timer->start_time += periods * timer->timeout_ms;

    return true;

}
bool timer_is_running(const timer_t *timer)
{
    if(!timer) return false;
    return (timer->state == TIMER_RUNNING);
}

uint32_t timer_get_elapsed(const timer_t *timer)
{
    if (!timer) {
        return 0U;
    }

    if (timer->state == TIMER_RUNNING) {
        return timer_calculate_elapsed(timer);
    }

    return timer->elapsed_ms;
}

uint32_t timer_get_remaining(const timer_t *timer)
{
    uint32_t elapsed;
    if (!timer || (timer->state != TIMER_RUNNING)) {
        return 0U;
    }
    elapsed = timer_calculate_elapsed(timer);
    if (elapsed >= timer->timeout_ms) {
        return 0U;
    }
    return timer->timeout_ms - elapsed;
}