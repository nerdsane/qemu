/*
 * Bloodhound Deterministic Execution State
 * 
 * This header defines the core state structures for deterministic
 * VM execution in Bloodhound.
 */

#ifndef BLOODHOUND_STATE_H
#define BLOODHOUND_STATE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct BloodhoundState {
    /* Instruction counting */
    uint64_t instruction_count;
    
    /* Virtual time (nanoseconds since epoch) */
    uint64_t virtual_time_ns;
    bool time_frozen;
    
    /* Deterministic RNG state */
    uint64_t rng_seed;
    uint64_t rng_state[2];  /* xorshift128+ state */
    uint64_t rng_counter;
    bool rng_initialized;
    
    /* Snapshot tracking */
    uint64_t current_snapshot_id;
    uint64_t next_snapshot_id;
    
    /* Determinism enabled flag */
    bool determinism_enabled;
} BloodhoundState;

/* Global state accessor */
BloodhoundState *bloodhound_get_state(void);

/* State initialization */
void bloodhound_state_init(void);
void bloodhound_set_seed(uint64_t seed);

/* Virtual time */
uint64_t bloodhound_get_virtual_time_ns(void);
void bloodhound_set_virtual_time_ns(uint64_t time_ns);
void bloodhound_advance_time_ns(uint64_t delta_ns);
void bloodhound_freeze_time(void);
void bloodhound_unfreeze_time(void);

/* Virtual TSC (for RDTSC instruction) */
uint64_t bloodhound_get_virtual_tsc(void);

/* Deterministic RNG */
uint64_t bloodhound_deterministic_random(void);
void bloodhound_get_random_bytes(uint8_t *buf, size_t len);

/* Instruction counting */
void bloodhound_add_instructions(uint64_t count);
uint64_t bloodhound_get_instruction_count(void);

#endif /* BLOODHOUND_STATE_H */
