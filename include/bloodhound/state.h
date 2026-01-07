/*
 * Bloodhound Deterministic Execution State
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#ifndef BLOODHOUND_STATE_H
#define BLOODHOUND_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Core state for deterministic execution tracking.
 */
typedef struct BloodhoundState {
    uint64_t instruction_count;
    uint64_t virtual_time_ns;
    uint64_t rng_seed;
    uint64_t rng_counter;
    uint64_t snapshot_id;
    uint64_t sched_quantum;
    bool deterministic_mode;
    uint64_t tsc_frequency;
} BloodhoundState;

/* Global state instance */
extern BloodhoundState bh_state;

/* Core functions (determinism.c) */
void bloodhound_init(uint64_t seed);
void bloodhound_reset(void);
void bloodhound_tb_executed(uint64_t icount);
bool bloodhound_at_schedule_point(void);
void bloodhound_process_pending(void);
uint64_t bloodhound_get_virtual_tsc(void);

/* Random functions (random.c) */
uint64_t bloodhound_deterministic_random(void);
void bloodhound_random_bytes(void *buf, size_t len);
void bloodhound_random_reset(void);

/* Time functions (time.c) */
uint64_t bloodhound_get_virtual_time_ns(void);
uint64_t bloodhound_get_hpet_counter(void);
uint64_t bloodhound_get_pit_counter(void);
uint64_t bloodhound_get_rtc_time(void);
void bloodhound_advance_time(uint64_t ns);
void bloodhound_check_timers(void);

/* I/O functions (io.c) */
void bloodhound_queue_packet(void *data, size_t len, void *opaque,
                             uint64_t delay_icount);
void bloodhound_deliver_packets(void);
void bloodhound_queue_disk_io(void *opaque, uint64_t delay_icount);
void bloodhound_complete_disk_io(void);

/* Snapshot functions (snapshot.c) */
uint64_t bloodhound_snapshot_create(void);
int bloodhound_snapshot_restore(uint64_t id);
void bloodhound_snapshot_delete(uint64_t id);
uint64_t bloodhound_snapshot_current(void);
uint64_t bloodhound_snapshot_count(void);

#endif /* BLOODHOUND_STATE_H */
