/*
 * Bloodhound Virtual Time Management
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#include "qemu/osdep.h"
#include "qemu/timer.h"
#include "bloodhound/state.h"

/*
 * Get current virtual time in nanoseconds.
 * Time advances based on instruction count for determinism.
 */
uint64_t bloodhound_get_virtual_time_ns(void)
{
    return bh_state.virtual_time_ns;
}

/*
 * Get virtual HPET counter value.
 * HPET runs at ~10 MHz, so divide nanoseconds by 100.
 */
uint64_t bloodhound_get_hpet_counter(void)
{
    return bh_state.virtual_time_ns / 100;
}

/*
 * Get virtual PIT counter value.
 * PIT runs at 1.193182 MHz.
 */
uint64_t bloodhound_get_pit_counter(void)
{
    /* 1193182 Hz = approximately 838 ns per tick */
    return bh_state.virtual_time_ns / 838;
}

/*
 * Get virtual RTC time.
 * Returns seconds since epoch plus virtual time offset.
 */
uint64_t bloodhound_get_rtc_time(void)
{
    /* Use a fixed epoch start for reproducibility */
    uint64_t epoch_start = 1704067200;  /* 2024-01-01 00:00:00 UTC */
    return epoch_start + (bh_state.virtual_time_ns / 1000000000ULL);
}

/*
 * Advance virtual time by specified nanoseconds.
 * Used when sleeping or waiting for I/O.
 */
void bloodhound_advance_time(uint64_t ns)
{
    bh_state.virtual_time_ns += ns;
}

/*
 * Check and fire pending timers.
 * Called at scheduling points.
 */
void bloodhound_check_timers(void)
{
    /* Timer checking is handled by QEMU's timer infrastructure
     * We just need to ensure time advances deterministically */
}
