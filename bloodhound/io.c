/*
 * Bloodhound Deterministic I/O Ordering
 *
 * Copyright (c) 2024 Bloodhound Project
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 */

#include "qemu/osdep.h"
#include "qemu/queue.h"
#include "bloodhound/state.h"

/*
 * Pending network packet structure
 */
typedef struct PendingPacket {
    uint64_t delivery_icount;  /* Instruction count when to deliver */
    void *data;
    size_t len;
    void *opaque;
    QTAILQ_ENTRY(PendingPacket) next;
} PendingPacket;

static QTAILQ_HEAD(, PendingPacket) pending_packets =
    QTAILQ_HEAD_INITIALIZER(pending_packets);

/*
 * Pending disk I/O structure
 */
typedef struct PendingDiskIO {
    uint64_t completion_icount;
    void *opaque;
    QTAILQ_ENTRY(PendingDiskIO) next;
} PendingDiskIO;

static QTAILQ_HEAD(, PendingDiskIO) pending_disk_io =
    QTAILQ_HEAD_INITIALIZER(pending_disk_io);

/*
 * Queue a network packet for deterministic delivery.
 */
void bloodhound_queue_packet(void *data, size_t len, void *opaque,
                             uint64_t delay_icount)
{
    PendingPacket *pkt = g_new0(PendingPacket, 1);
    pkt->delivery_icount = bh_state.instruction_count + delay_icount;
    pkt->data = g_memdup(data, len);
    pkt->len = len;
    pkt->opaque = opaque;

    /* Insert in sorted order by delivery time */
    PendingPacket *p;
    QTAILQ_FOREACH(p, &pending_packets, next) {
        if (pkt->delivery_icount < p->delivery_icount) {
            QTAILQ_INSERT_BEFORE(p, pkt, next);
            return;
        }
    }
    QTAILQ_INSERT_TAIL(&pending_packets, pkt, next);
}

/*
 * Deliver pending network packets that are due.
 */
void bloodhound_deliver_packets(void)
{
    PendingPacket *pkt, *next;

    QTAILQ_FOREACH_SAFE(pkt, &pending_packets, next, next) {
        if (pkt->delivery_icount <= bh_state.instruction_count) {
            /* Packet delivery would happen here */
            QTAILQ_REMOVE(&pending_packets, pkt, next);
            g_free(pkt->data);
            g_free(pkt);
        } else {
            break;  /* List is sorted, no more due packets */
        }
    }
}

/*
 * Queue disk I/O for deterministic completion.
 */
void bloodhound_queue_disk_io(void *opaque, uint64_t delay_icount)
{
    PendingDiskIO *io = g_new0(PendingDiskIO, 1);
    io->completion_icount = bh_state.instruction_count + delay_icount;
    io->opaque = opaque;

    /* Insert in sorted order */
    PendingDiskIO *p;
    QTAILQ_FOREACH(p, &pending_disk_io, next) {
        if (io->completion_icount < p->completion_icount) {
            QTAILQ_INSERT_BEFORE(p, io, next);
            return;
        }
    }
    QTAILQ_INSERT_TAIL(&pending_disk_io, io, next);
}

/*
 * Complete pending disk I/O operations that are due.
 */
void bloodhound_complete_disk_io(void)
{
    PendingDiskIO *io, *next;

    QTAILQ_FOREACH_SAFE(io, &pending_disk_io, next, next) {
        if (io->completion_icount <= bh_state.instruction_count) {
            /* I/O completion would happen here */
            QTAILQ_REMOVE(&pending_disk_io, io, next);
            g_free(io);
        } else {
            break;
        }
    }
}
