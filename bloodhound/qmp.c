/*
 * Bloodhound QMP Command Implementation
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#include "qemu/osdep.h"
#include "qapi/qapi-commands-bloodhound.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "bloodhound/state.h"

/* Track active network faults */
static struct {
    bool active;
    BloodhoundNetworkFaultType type;
    char *interface;
    uint64_t duration_ns;
    double probability;
    uint64_t delay_ns;
    uint64_t start_time_ns;
} active_network_fault;

/* Track active disk faults */
static struct {
    bool active;
    BloodhoundDiskFaultType type;
    char *drive;
    uint64_t duration_ns;
    double probability;
    uint64_t delay_ns;
    uint64_t start_time_ns;
} active_disk_fault;

/*
 * qmp_bloodhound_ctrl - Main entry point for bloodhound-ctrl QMP command
 *
 * Handles all Bloodhound control operations for deterministic simulation:
 * - set_time: Set virtual time
 * - advance_time: Advance virtual time by delta
 * - freeze_time: Freeze time (stop advancing)
 * - unfreeze_time: Unfreeze time
 * - set_seed: Set RNG seed
 * - inject_network_fault: Inject network fault
 * - inject_disk_fault: Inject disk fault
 * - clear_faults: Clear all faults
 * - deterministic_snapshot: Take snapshot
 * - restore_snapshot: Restore snapshot
 * - query_determinism: Query state
 */
BloodhoundDeterminismInfo *qmp_bloodhound_ctrl(const char *type,
                                                bool has_time_ns, uint64_t time_ns,
                                                bool has_delta_ns, uint64_t delta_ns,
                                                bool has_seed, uint64_t seed,
                                                const char *id,
                                                BloodhoundNetworkFault *network_fault,
                                                BloodhoundDiskFault *disk_fault,
                                                Error **errp)
{
    BloodhoundDeterminismInfo *info;

    qemu_log("bloodhound-ctrl: type=%s\n", type);

    /* Handle each command type */
    if (strcmp(type, "set_time") == 0) {
        if (!has_time_ns) {
            error_setg(errp, "set_time requires time-ns argument");
            return NULL;
        }
        bh_state.virtual_time_ns = time_ns;
        qemu_log("bloodhound: set_time to %" PRIu64 " ns\n", time_ns);

    } else if (strcmp(type, "advance_time") == 0) {
        if (!has_delta_ns) {
            error_setg(errp, "advance_time requires delta-ns argument");
            return NULL;
        }
        bh_state.virtual_time_ns += delta_ns;
        qemu_log("bloodhound: advance_time by %" PRIu64 " ns (now %" PRIu64 ")\n",
                 delta_ns, bh_state.virtual_time_ns);

    } else if (strcmp(type, "freeze_time") == 0) {
        /* For now, just log - full implementation would stop timer updates */
        qemu_log("bloodhound: freeze_time\n");

    } else if (strcmp(type, "unfreeze_time") == 0) {
        qemu_log("bloodhound: unfreeze_time\n");

    } else if (strcmp(type, "set_seed") == 0) {
        if (!has_seed) {
            error_setg(errp, "set_seed requires seed argument");
            return NULL;
        }
        bh_state.rng_seed = seed;
        bh_state.rng_counter = 0;
        qemu_log("bloodhound: set_seed to %" PRIu64 "\n", seed);

    } else if (strcmp(type, "inject_network_fault") == 0) {
        if (!network_fault) {
            error_setg(errp, "inject_network_fault requires network-fault argument");
            return NULL;
        }
        active_network_fault.active = true;
        active_network_fault.type = network_fault->fault_type;
        g_free(active_network_fault.interface);
        active_network_fault.interface = network_fault->interface ?
            g_strdup(network_fault->interface) : NULL;
        active_network_fault.duration_ns = network_fault->has_duration_ns ?
            network_fault->duration_ns : 0;
        active_network_fault.probability = network_fault->probability;
        active_network_fault.delay_ns = network_fault->has_delay_ns ?
            network_fault->delay_ns : 0;
        active_network_fault.start_time_ns = bh_state.virtual_time_ns;
        qemu_log("bloodhound: inject_network_fault type=%d prob=%.2f\n",
                 network_fault->fault_type, network_fault->probability);

    } else if (strcmp(type, "inject_disk_fault") == 0) {
        if (!disk_fault) {
            error_setg(errp, "inject_disk_fault requires disk-fault argument");
            return NULL;
        }
        active_disk_fault.active = true;
        active_disk_fault.type = disk_fault->fault_type;
        g_free(active_disk_fault.drive);
        active_disk_fault.drive = g_strdup(disk_fault->drive);
        active_disk_fault.duration_ns = disk_fault->has_duration_ns ?
            disk_fault->duration_ns : 0;
        active_disk_fault.probability = disk_fault->probability;
        active_disk_fault.delay_ns = disk_fault->has_delay_ns ?
            disk_fault->delay_ns : 0;
        active_disk_fault.start_time_ns = bh_state.virtual_time_ns;
        qemu_log("bloodhound: inject_disk_fault drive=%s type=%d prob=%.2f\n",
                 disk_fault->drive, disk_fault->fault_type, disk_fault->probability);

    } else if (strcmp(type, "clear_faults") == 0) {
        active_network_fault.active = false;
        g_free(active_network_fault.interface);
        active_network_fault.interface = NULL;
        active_disk_fault.active = false;
        g_free(active_disk_fault.drive);
        active_disk_fault.drive = NULL;
        qemu_log("bloodhound: clear_faults\n");

    } else if (strcmp(type, "deterministic_snapshot") == 0) {
        if (!id) {
            error_setg(errp, "deterministic_snapshot requires id argument");
            return NULL;
        }
        bh_state.snapshot_id++;
        qemu_log("bloodhound: deterministic_snapshot id=%s (internal %" PRIu64 ")\n",
                 id, bh_state.snapshot_id);

    } else if (strcmp(type, "restore_snapshot") == 0) {
        if (!id) {
            error_setg(errp, "restore_snapshot requires id argument");
            return NULL;
        }
        qemu_log("bloodhound: restore_snapshot id=%s\n", id);

    } else if (strcmp(type, "query_determinism") == 0) {
        /* Just return the info below */
        qemu_log("bloodhound: query_determinism\n");

    } else {
        error_setg(errp, "Unknown bloodhound-ctrl type: %s", type);
        return NULL;
    }

    /* Always return current state */
    info = g_malloc0(sizeof(*info));
    info->enabled = bh_state.deterministic_mode;
    info->virtual_time_ns = bh_state.virtual_time_ns;
    info->instruction_count = bh_state.instruction_count;
    info->rng_seed = bh_state.rng_seed;
    info->snapshot_id = bh_state.snapshot_id;

    return info;
}
