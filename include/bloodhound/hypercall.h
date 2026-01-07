/*
 * Bloodhound Hypercall Interface
 *
 * Defines hypercall numbers for guest-to-hypervisor communication.
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#ifndef BLOODHOUND_HYPERCALL_H
#define BLOODHOUND_HYPERCALL_H

#include <stdint.h>

/* Base for all Bloodhound hypercalls (matches kernel) */
#define BLOODHOUND_HYPERCALL_BASE    0x424800

/* Check if a hypercall number is a Bloodhound hypercall */
#define IS_BLOODHOUND_HYPERCALL(nr) \
    (((nr) & 0xFFFFFF00) == BLOODHOUND_HYPERCALL_BASE)

/* Hypercall numbers (must match kernel's linux/bloodhound.h) */

/* Get deterministic random bytes */
#define BLOODHOUND_GET_RANDOM        (BLOODHOUND_HYPERCALL_BASE + 0)

/* Get current virtual time (nanoseconds) */
#define BLOODHOUND_GET_TIME          (BLOODHOUND_HYPERCALL_BASE + 1)

/* Create a snapshot, returns snapshot ID */
#define BLOODHOUND_SNAPSHOT_CREATE   (BLOODHOUND_HYPERCALL_BASE + 2)

/* Restore to a snapshot by ID */
#define BLOODHOUND_SNAPSHOT_RESTORE  (BLOODHOUND_HYPERCALL_BASE + 3)

/* Log an event for tracing */
#define BLOODHOUND_LOG_EVENT         (BLOODHOUND_HYPERCALL_BASE + 4)

/* Process hypercall from guest */
uint64_t bloodhound_handle_hypercall(uint64_t nr, uint64_t arg1,
                                      uint64_t arg2, uint64_t arg3);

#endif /* BLOODHOUND_HYPERCALL_H */
