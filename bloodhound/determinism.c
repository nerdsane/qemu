/*
 * Bloodhound Deterministic Execution Core
 */

#include "qemu/osdep.h"
#include "bloodhound/state.h"

/* Global Bloodhound state */
static BloodhoundState bh_state = {
    .instruction_count = 0,
    .virtual_time_ns = 0,
    .time_frozen = false,
    .rng_seed = 0,
    .rng_counter = 0,
    .rng_initialized = false,
    .current_snapshot_id = 0,
    .next_snapshot_id = 1,
    .determinism_enabled = true,
};

BloodhoundState *bloodhound_get_state(void)
{
    return &bh_state;
}

void bloodhound_state_init(void)
{
    memset(&bh_state, 0, sizeof(bh_state));
    bh_state.next_snapshot_id = 1;
    bh_state.determinism_enabled = true;
}

void bloodhound_set_seed(uint64_t seed)
{
    bh_state.rng_seed = seed;
    bh_state.rng_state[0] = seed;
    bh_state.rng_state[1] = seed ^ 0x9e3779b97f4a7c15ULL;
    bh_state.rng_counter = 0;
    bh_state.rng_initialized = true;
}

/* Virtual time implementation */
uint64_t bloodhound_get_virtual_time_ns(void)
{
    return bh_state.virtual_time_ns;
}

void bloodhound_set_virtual_time_ns(uint64_t time_ns)
{
    bh_state.virtual_time_ns = time_ns;
}

void bloodhound_advance_time_ns(uint64_t delta_ns)
{
    if (!bh_state.time_frozen) {
        bh_state.virtual_time_ns += delta_ns;
    }
}

void bloodhound_freeze_time(void)
{
    bh_state.time_frozen = true;
}

void bloodhound_unfreeze_time(void)
{
    bh_state.time_frozen = false;
}

/* Virtual TSC - uses instruction count for determinism */
uint64_t bloodhound_get_virtual_tsc(void)
{
    /* 1 instruction ≈ 1 TSC tick for simplicity */
    return bh_state.instruction_count;
}

/* xorshift128+ PRNG - fast and deterministic */
uint64_t bloodhound_deterministic_random(void)
{
    if (!bh_state.rng_initialized) {
        /* Default seed if not set */
        bloodhound_set_seed(0x12345678DEADBEEFULL);
    }

    uint64_t s1 = bh_state.rng_state[0];
    uint64_t s0 = bh_state.rng_state[1];
    uint64_t result = s0 + s1;

    bh_state.rng_state[0] = s0;
    s1 ^= s1 << 23;
    bh_state.rng_state[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);

    bh_state.rng_counter++;
    return result;
}

void bloodhound_get_random_bytes(uint8_t *buf, size_t len)
{
    size_t i;
    uint64_t r;
    
    for (i = 0; i < len; i += 8) {
        r = bloodhound_deterministic_random();
        size_t chunk = (len - i) < 8 ? (len - i) : 8;
        memcpy(buf + i, &r, chunk);
    }
}

/* Instruction counting */
void bloodhound_add_instructions(uint64_t count)
{
    bh_state.instruction_count += count;
    
    /* Also advance virtual time proportionally */
    /* Assume ~1ns per instruction (simplified) */
    if (!bh_state.time_frozen) {
        bh_state.virtual_time_ns += count;
    }
}

uint64_t bloodhound_get_instruction_count(void)
{
    return bh_state.instruction_count;
}
