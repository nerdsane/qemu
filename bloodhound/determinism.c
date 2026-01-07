/*
 * Bloodhound Deterministic Execution Core
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "bloodhound/state.h"

/* Global Bloodhound state */
BloodhoundState bh_state;

/* Default scheduling quantum: check every 10000 instructions */
#define DEFAULT_SCHED_QUANTUM 10000

/* Default TSC frequency: 3 GHz */
#define DEFAULT_TSC_FREQ 3000000000ULL

void bloodhound_init(uint64_t seed)
{
    memset(&bh_state, 0, sizeof(bh_state));

    bh_state.rng_seed = seed;
    bh_state.sched_quantum = DEFAULT_SCHED_QUANTUM;
    bh_state.tsc_frequency = DEFAULT_TSC_FREQ;
    bh_state.deterministic_mode = true;
    bh_state.snapshot_id = 0;

    qemu_log("Bloodhound: Initialized with seed %" PRIu64 "\n", seed);
}

void bloodhound_reset(void)
{
    uint64_t seed = bh_state.rng_seed;
    bloodhound_init(seed);
}

void bloodhound_tb_executed(uint64_t icount)
{
    bh_state.instruction_count += icount;

    /* Update virtual time proportionally to instructions
     * Assume 1 instruction = 1 nanosecond (simplified model)
     */
    bh_state.virtual_time_ns += icount;

    /* Check for scheduling point */
    if (bloodhound_at_schedule_point()) {
        bloodhound_process_pending();
    }
}

bool bloodhound_at_schedule_point(void)
{
    return (bh_state.instruction_count % bh_state.sched_quantum) == 0;
}

void bloodhound_process_pending(void)
{
    /* Process pending timers */
    bloodhound_check_timers();

    /* Deliver pending network packets */
    bloodhound_deliver_packets();

    /* Complete pending disk I/O */
    bloodhound_complete_disk_io();
}

uint64_t bloodhound_get_virtual_tsc(void)
{
    return bh_state.instruction_count;
}

/* Weak stub implementations - overridden by other modules */
__attribute__((weak)) void bloodhound_check_timers(void) {}
__attribute__((weak)) void bloodhound_deliver_packets(void) {}
__attribute__((weak)) void bloodhound_complete_disk_io(void) {}
