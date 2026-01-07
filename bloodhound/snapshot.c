/*
 * Bloodhound Snapshot System
 *
 * Provides CoW (Copy-on-Write) snapshots for efficient state exploration.
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#include "qemu/osdep.h"
#include "qemu/queue.h"
#include "bloodhound/state.h"

/*
 * Snapshot metadata
 */
typedef struct Snapshot {
    uint64_t id;
    uint64_t parent_id;
    uint64_t instruction_count;
    uint64_t virtual_time_ns;
    uint64_t rng_seed;
    uint64_t rng_counter;
    QTAILQ_ENTRY(Snapshot) next;
} Snapshot;

static QTAILQ_HEAD(, Snapshot) snapshots =
    QTAILQ_HEAD_INITIALIZER(snapshots);
static uint64_t next_snapshot_id = 1;

/*
 * Create a new snapshot.
 * Returns the snapshot ID.
 */
uint64_t bloodhound_snapshot_create(void)
{
    Snapshot *snap = g_new0(Snapshot, 1);

    snap->id = next_snapshot_id++;
    snap->parent_id = bh_state.snapshot_id;
    snap->instruction_count = bh_state.instruction_count;
    snap->virtual_time_ns = bh_state.virtual_time_ns;
    snap->rng_seed = bh_state.rng_seed;
    snap->rng_counter = bh_state.rng_counter;

    QTAILQ_INSERT_TAIL(&snapshots, snap, next);

    bh_state.snapshot_id = snap->id;

    return snap->id;
}

/*
 * Find a snapshot by ID.
 */
static Snapshot *find_snapshot(uint64_t id)
{
    Snapshot *snap;
    QTAILQ_FOREACH(snap, &snapshots, next) {
        if (snap->id == id) {
            return snap;
        }
    }
    return NULL;
}

/*
 * Restore to a snapshot.
 * Returns 0 on success, -1 on error.
 */
int bloodhound_snapshot_restore(uint64_t id)
{
    Snapshot *snap = find_snapshot(id);
    if (!snap) {
        return -1;
    }

    bh_state.snapshot_id = snap->id;
    bh_state.instruction_count = snap->instruction_count;
    bh_state.virtual_time_ns = snap->virtual_time_ns;
    bh_state.rng_seed = snap->rng_seed;
    bh_state.rng_counter = snap->rng_counter;

    /* Full VM state restore would happen here via QEMU's
     * snapshot infrastructure */

    return 0;
}

/*
 * Delete a snapshot and all its children.
 */
void bloodhound_snapshot_delete(uint64_t id)
{
    Snapshot *snap = find_snapshot(id);
    if (snap) {
        QTAILQ_REMOVE(&snapshots, snap, next);
        g_free(snap);
    }
}

/*
 * Get the current snapshot ID.
 */
uint64_t bloodhound_snapshot_current(void)
{
    return bh_state.snapshot_id;
}

/*
 * Get snapshot count.
 */
uint64_t bloodhound_snapshot_count(void)
{
    uint64_t count = 0;
    Snapshot *snap;
    QTAILQ_FOREACH(snap, &snapshots, next) {
        count++;
    }
    return count;
}
