/*
 * Bloodhound Deterministic Random Number Generator
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#include "qemu/osdep.h"
#include "bloodhound/state.h"

/*
 * Deterministic random number generator.
 * Uses xorshift128+ seeded from simulation seed.
 */
static uint64_t rng_state[2] = {0, 0};
static bool rng_initialized = false;

static void init_rng(void)
{
    if (!rng_initialized) {
        rng_state[0] = bh_state.rng_seed;
        rng_state[1] = bh_state.rng_seed ^ 0x9e3779b97f4a7c15ULL;
        if (rng_state[0] == 0 && rng_state[1] == 0) {
            rng_state[0] = 1;  /* Avoid all-zero state */
        }
        rng_initialized = true;
    }
}

uint64_t bloodhound_deterministic_random(void)
{
    init_rng();

    uint64_t s1 = rng_state[0];
    uint64_t s0 = rng_state[1];
    uint64_t result = s0 + s1;

    rng_state[0] = s0;
    s1 ^= s1 << 23;
    rng_state[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);

    bh_state.rng_counter++;

    return result;
}

/*
 * Fill buffer with deterministic random bytes.
 */
void bloodhound_random_bytes(void *buf, size_t len)
{
    uint8_t *p = buf;
    size_t i;

    for (i = 0; i + 8 <= len; i += 8) {
        uint64_t r = bloodhound_deterministic_random();
        memcpy(p + i, &r, 8);
    }

    if (i < len) {
        uint64_t r = bloodhound_deterministic_random();
        memcpy(p + i, &r, len - i);
    }
}

/*
 * Reset RNG state (for snapshot restore)
 */
void bloodhound_random_reset(void)
{
    rng_initialized = false;
    init_rng();
}
