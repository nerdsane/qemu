/*
 * Bloodhound Hypercall Handler
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "bloodhound/state.h"
#include "bloodhound/hypercall.h"

/* Ensure bloodhound is initialized */
static void ensure_initialized(void)
{
    static bool initialized = false;
    if (!initialized) {
        bloodhound_init(0x12345678);  /* Default seed */
        initialized = true;
    }
}

/*
 * Handle a hypercall from the guest.
 *
 * @nr: Hypercall number
 * @arg1, arg2, arg3: Arguments from guest
 * @return: Result to return to guest
 */
uint64_t bloodhound_handle_hypercall(uint64_t nr, uint64_t arg1,
                                      uint64_t arg2, uint64_t arg3)
{
    ensure_initialized();

    if (!IS_BLOODHOUND_HYPERCALL(nr)) {
        return (uint64_t)-1;
    }

    switch (nr) {
    case BLOODHOUND_GET_RANDOM:
        /* Return a deterministic random value */
        return bloodhound_deterministic_random();

    case BLOODHOUND_GET_TIME:
        /* arg1 = subfunction: 0 = virtual time ns, 1 = instruction count */
        if (arg1 == 0) {
            return bh_state.virtual_time_ns;
        } else {
            return bh_state.instruction_count;
        }

    case BLOODHOUND_SNAPSHOT_CREATE:
        return bloodhound_snapshot_create();

    case BLOODHOUND_SNAPSHOT_RESTORE:
        return bloodhound_snapshot_restore(arg1);

    case BLOODHOUND_LOG_EVENT:
        /* arg1 = event type, arg2 = data */
        qemu_log("Bloodhound: Guest event type=%" PRIu64 " data=%" PRIu64 "\n",
                 arg1, arg2);
        return 0;

    default:
        qemu_log("Bloodhound: Unknown hypercall %" PRIx64 "\n", nr);
        return (uint64_t)-1;
    }
}
