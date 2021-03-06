//
// Created by x3vikan on 2/18/18.
//
#include "../../kernel/kernel.h"

#define PIT_A 0x40
#define PIT_B 0x41
#define PIT_C 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xFF
#define PIT_SCALE 1193180
#define PIT_SET 0x34

#define TIMER_IRQ 0

#define SUBTICKS_PER_TICK 1000
#define RESYNC_TIME 1

/*
 * Set the phase (in hertz) for the Programmable
 * Interrupt Timer (PIT).
 */
void
timer_phase(int hz) {
    int divisor = PIT_SCALE / hz;
    outportb(PIT_CONTROL, PIT_SET);
    outportb(PIT_A, divisor & PIT_MASK);
    outportb(PIT_A, (divisor >> 8) & PIT_MASK);
}

/*
 * Internal timer counters
 */
unsigned long timer_ticks = 0;
unsigned long timer_subticks = 0;
signed long timer_drift = 0;
signed long _timer_drift = 0;

static int behind = 0;

/*
 * IRQ handler for when the timer fires
 */
int timer_handler(state *r) {
    if (++timer_subticks == SUBTICKS_PER_TICK || (behind && ++timer_subticks == SUBTICKS_PER_TICK)) {
        timer_ticks++;
        timer_subticks = 0;
        if (timer_ticks % RESYNC_TIME == 0) {
            uint32_t new_time = read_cmos();
            _timer_drift = new_time - boot_time - timer_ticks;
            if (_timer_drift > 0) behind = 1;
            else behind = 0;
        }
    }
    return 1;
}

void relative_time(unsigned long seconds, unsigned long subseconds, unsigned long * out_seconds, unsigned long * out_subseconds) {
    if (subseconds + timer_subticks > SUBTICKS_PER_TICK) {
        *out_seconds    = timer_ticks + seconds + 1;
        *out_subseconds = (subseconds + timer_subticks) - SUBTICKS_PER_TICK;
    } else {
        *out_seconds    = timer_ticks + seconds;
        *out_subseconds = timer_subticks + subseconds;
    }
}

void timer_install(void) {
    boot_time = read_cmos();
    addInterruptHandler(TIMER_IRQ, timer_handler);
    timer_phase(SUBTICKS_PER_TICK); /* 100Hz */
}