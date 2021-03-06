/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef SHM_DIRECT
#define SHM_DISABLE_INLINES
#include <mpidimpl.h>
#include "shm_direct.h"
MPIDI_SHM_funcs_t MPIDI_SHM_stubshm_funcs = {
    MPIDI_SHM_mpi_init_hook,
    MPIDI_SHM_mpi_finalize_hook,
    MPIDI_SHM_progress,
    MPIDI_SHM_am_reg_handler,
    MPIDI_SHM_mpi_comm_connect,
    MPIDI_SHM_mpi_comm_disconnect,
    MPIDI_SHM_mpi_open_port,
    MPIDI_SHM_mpi_close_port,
    MPIDI_SHM_mpi_comm_accept,
    MPIDI_SHM_am_send_hdr,
    MPIDI_SHM_am_isend,
    MPIDI_SHM_inject_am,
    MPIDI_SHM_am_isendv,
    MPIDI_SHM_inject_amv,
    MPIDI_SHM_am_send_hdr_reply,
    MPIDI_SHM_am_isend_reply,
    MPIDI_SHM_inject_am_reply,
    MPIDI_SHM_inject_amv_reply,
    MPIDI_SHM_am_hdr_max_sz,
    MPIDI_SHM_am_inject_max_sz,
    MPIDI_SHM_am_recv,
    MPIDI_SHM_comm_get_lpid,
    MPIDI_SHM_gpid_get,
    MPIDI_SHM_get_node_id,
    MPIDI_SHM_get_max_node_id,
    MPIDI_SHM_getallincomm,
    MPIDI_SHM_gpid_tolpidarray,
    MPIDI_SHM_create_intercomm_from_lpids,
    MPIDI_SHM_mpi_comm_create_hook,
    MPIDI_SHM_mpi_comm_free_hook,
    MPIDI_SHM_mpi_type_create_hook,
    MPIDI_SHM_mpi_type_free_hook,
    MPIDI_SHM_mpi_op_create_hook,
    MPIDI_SHM_mpi_op_free_hook,
    MPIDI_SHM_am_request_init,
};

MPIDI_SHM_native_funcs_t MPIDI_SHM_native_stubshm_funcs = {
    MPIDI_SHM_mpi_send,
    MPIDI_SHM_mpi_ssend,
    MPIDI_SHM_mpi_startall,
    MPIDI_SHM_mpi_send_init,
    MPIDI_SHM_mpi_ssend_init,
    MPIDI_SHM_mpi_rsend_init,
    MPIDI_SHM_mpi_bsend_init,
    MPIDI_SHM_mpi_isend,
    MPIDI_SHM_mpi_issend,
    MPIDI_SHM_mpi_cancel_send,
    MPIDI_SHM_mpi_recv_init,
    MPIDI_SHM_mpi_recv,
    MPIDI_SHM_mpi_irecv,
    MPIDI_SHM_mpi_imrecv,
    MPIDI_SHM_mpi_cancel_recv,
    MPIDI_SHM_mpi_alloc_mem,
    MPIDI_SHM_mpi_free_mem,
    MPIDI_SHM_mpi_improbe,
    MPIDI_SHM_mpi_iprobe,
    MPIDI_SHM_mpi_win_set_info,
    MPIDI_SHM_mpi_win_shared_query,
    MPIDI_SHM_mpi_put,
    MPIDI_SHM_mpi_win_start,
    MPIDI_SHM_mpi_win_complete,
    MPIDI_SHM_mpi_win_post,
    MPIDI_SHM_mpi_win_wait,
    MPIDI_SHM_mpi_win_test,
    MPIDI_SHM_mpi_win_lock,
    MPIDI_SHM_mpi_win_unlock,
    MPIDI_SHM_mpi_win_get_info,
    MPIDI_SHM_mpi_get,
    MPIDI_SHM_mpi_win_free,
    MPIDI_SHM_mpi_win_fence,
    MPIDI_SHM_mpi_win_create,
    MPIDI_SHM_mpi_accumulate,
    MPIDI_SHM_mpi_win_attach,
    MPIDI_SHM_mpi_win_allocate_shared,
    MPIDI_SHM_mpi_rput,
    MPIDI_SHM_mpi_win_flush_local,
    MPIDI_SHM_mpi_win_detach,
    MPIDI_SHM_mpi_compare_and_swap,
    MPIDI_SHM_mpi_raccumulate,
    MPIDI_SHM_mpi_rget_accumulate,
    MPIDI_SHM_mpi_fetch_and_op,
    MPIDI_SHM_mpi_win_allocate,
    MPIDI_SHM_mpi_win_flush,
    MPIDI_SHM_mpi_win_flush_local_all,
    MPIDI_SHM_mpi_win_unlock_all,
    MPIDI_SHM_mpi_win_create_dynamic,
    MPIDI_SHM_mpi_rget,
    MPIDI_SHM_mpi_win_sync,
    MPIDI_SHM_mpi_win_flush_all,
    MPIDI_SHM_mpi_get_accumulate,
    MPIDI_SHM_mpi_win_lock_all,
    MPIDI_SHM_mpi_barrier,
    MPIDI_SHM_mpi_bcast,
    MPIDI_SHM_mpi_allreduce,
    MPIDI_SHM_mpi_allgather,
    MPIDI_SHM_mpi_allgatherv,
    MPIDI_SHM_mpi_scatter,
    MPIDI_SHM_mpi_scatterv,
    MPIDI_SHM_mpi_gather,
    MPIDI_SHM_mpi_gatherv,
    MPIDI_SHM_mpi_alltoall,
    MPIDI_SHM_mpi_alltoallv,
    MPIDI_SHM_mpi_alltoallw,
    MPIDI_SHM_mpi_reduce,
    MPIDI_SHM_mpi_reduce_scatter,
    MPIDI_SHM_mpi_reduce_scatter_block,
    MPIDI_SHM_mpi_scan,
    MPIDI_SHM_mpi_exscan,
    MPIDI_SHM_mpi_neighbor_allgather,
    MPIDI_SHM_mpi_neighbor_allgatherv,
    MPIDI_SHM_mpi_neighbor_alltoall,
    MPIDI_SHM_mpi_neighbor_alltoallv,
    MPIDI_SHM_mpi_neighbor_alltoallw,
    MPIDI_SHM_mpi_ineighbor_allgather,
    MPIDI_SHM_mpi_ineighbor_allgatherv,
    MPIDI_SHM_mpi_ineighbor_alltoall,
    MPIDI_SHM_mpi_ineighbor_alltoallv,
    MPIDI_SHM_mpi_ineighbor_alltoallw,
    MPIDI_SHM_mpi_ibarrier,
    MPIDI_SHM_mpi_ibcast,
    MPIDI_SHM_mpi_iallgather,
    MPIDI_SHM_mpi_iallgatherv,
    MPIDI_SHM_mpi_iallreduce,
    MPIDI_SHM_mpi_ialltoall,
    MPIDI_SHM_mpi_ialltoallv,
    MPIDI_SHM_mpi_ialltoallw,
    MPIDI_SHM_mpi_iexscan,
    MPIDI_SHM_mpi_igather,
    MPIDI_SHM_mpi_igatherv,
    MPIDI_SHM_mpi_ireduce_scatter_block,
    MPIDI_SHM_mpi_ireduce_scatter,
    MPIDI_SHM_mpi_ireduce,
    MPIDI_SHM_mpi_iscan,
    MPIDI_SHM_mpi_iscatter,
    MPIDI_SHM_mpi_iscatterv,
};
#endif
