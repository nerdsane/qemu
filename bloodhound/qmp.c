/*
 * Bloodhound QMP Command Handler
 * 
 * Implements the bloodhound-ctrl QMP command for external control
 * of deterministic execution.
 */

#include "qemu/osdep.h"
#include "qapi/qapi-commands-bloodhound.h"
#include "qapi/error.h"
#include "bloodhound/state.h"
#include <string.h>

BloodhoundDeterminismInfo *qmp_bloodhound_ctrl(const char *type,
                                                bool has_time_ns, uint64_t time_ns,
                                                bool has_delta_ns, uint64_t delta_ns,
                                                bool has_seed, uint64_t seed,
                                                Error **errp)
{
    BloodhoundState *state = bloodhound_get_state();
    BloodhoundDeterminismInfo *info = g_new0(BloodhoundDeterminismInfo, 1);

    if (strcmp(type, "set_time") == 0) {
        if (has_time_ns) {
            bloodhound_set_virtual_time_ns(time_ns);
        } else {
            error_setg(errp, "set_time requires time-ns parameter");
            g_free(info);
            return NULL;
        }
    } else if (strcmp(type, "advance_time") == 0) {
        if (has_delta_ns) {
            bloodhound_advance_time_ns(delta_ns);
        } else {
            error_setg(errp, "advance_time requires delta-ns parameter");
            g_free(info);
            return NULL;
        }
    } else if (strcmp(type, "freeze_time") == 0) {
        bloodhound_freeze_time();
    } else if (strcmp(type, "unfreeze_time") == 0) {
        bloodhound_unfreeze_time();
    } else if (strcmp(type, "set_seed") == 0) {
        if (has_seed) {
            bloodhound_set_seed(seed);
        } else {
            error_setg(errp, "set_seed requires seed parameter");
            g_free(info);
            return NULL;
        }
    } else if (strcmp(type, "query_determinism") == 0) {
        /* Just return current state */
    } else {
        error_setg(errp, "Unknown bloodhound-ctrl type: %s", type);
        g_free(info);
        return NULL;
    }

    /* Fill in current state */
    info->enabled = state->determinism_enabled;
    info->virtual_time_ns = state->virtual_time_ns;
    info->instruction_count = state->instruction_count;
    info->rng_seed = state->rng_seed;
    info->snapshot_id = state->current_snapshot_id;

    return info;
}
