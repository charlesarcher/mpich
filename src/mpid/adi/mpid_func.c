/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#include <mpidimpl.h>
#include <dlfcn.h>

#ifdef MPIDI_USE_DYNAMIC_ADI

const char *ADI_cmd_list[] = {
  "MPIDI_Abort",
  "MPIDI_Accumulate",
  "MPIDI_Alloc_mem",
  "MPIDI_Bsend_init",
  "MPIDI_Cancel_recv",
  "MPIDI_Cancel_send",
  "MPIDI_Close_port",
  "MPIDI_Comm_create_hook",
  "MPIDI_Comm_free_hook",
  "MPIDI_Intercomm_exchange_map",
  "MPIDI_Barrier",
  "MPIDI_Bcast",
  "MPIDI_Allreduce",
  "MPIDI_Allgather",
  "MPIDI_Allgatherv",
  "MPIDI_Scatter",
  "MPIDI_Scatterv",
  "MPIDI_Gather",
  "MPIDI_Gatherv",
  "MPIDI_Alltoall",
  "MPIDI_Alltoallv",
  "MPIDI_Alltoallw",
  "MPIDI_Reduce",
  "MPIDI_Reduce_scatter",
  "MPIDI_Reduce_scatter_block",
  "MPIDI_Scan",
  "MPIDI_Exscan",
  "MPIDI_Neighbor_allgather",
  "MPIDI_Neighbor_allgatherv",
  "MPIDI_Neighbor_alltoallv",
  "MPIDI_Neighbor_alltoallw",
  "MPIDI_Neighbor_alltoall",
  "MPIDI_Ineighbor_allgather",
  "MPIDI_Ineighbor_allgatherv",
  "MPIDI_Ineighbor_alltoall",
  "MPIDI_Ineighbor_alltoallv",
  "MPIDI_Ineighbor_alltoallw",
  "MPIDI_Ibarrier",
  "MPIDI_Ibcast",
  "MPIDI_Iallgather",
  "MPIDI_Iallgatherv",
  "MPIDI_Iallreduce",
  "MPIDI_Ialltoall",
  "MPIDI_Ialltoallv",
  "MPIDI_Ialltoallw",
  "MPIDI_Iexscan",
  "MPIDI_Igather",
  "MPIDI_Igatherv",
  "MPIDI_Ireduce_scatter_block",
  "MPIDI_Ireduce_scatter",
  "MPIDI_Ireduce",
  "MPIDI_Iscan",
  "MPIDI_Iscatter",
  "MPIDI_Iscatterv",
  "MPIDI_Comm_accept",
  "MPIDI_Comm_connect",
  "MPIDI_Comm_disconnect",
  "MPIDI_Comm_group_failed",
  "MPIDI_Comm_reenable_anysource",
  "MPIDI_Comm_remote_group_failed",
  "MPIDI_Comm_spawn_multiple",
  "MPIDI_Comm_failure_get_acked",
  "MPIDI_Comm_get_all_failed_procs",
  "MPIDI_Comm_revoke",
  "MPIDI_Comm_failure_ack",
  "MPIDI_Comm_AS_enabled",
  "MPIDI_Comm_get_lpid",
  "MPIDI_Compare_and_swap",
  "MPIDI_Fetch_and_op",
  "MPIDI_Finalize",
  "MPIDI_Free_mem",
  "MPIDI_Get",
  "MPIDI_Get_accumulate",
  "MPIDI_Get_processor_name",
  "MPIDI_Get_universe_size",
  "MPIDI_Improbe",
  "MPIDI_Imrecv",
  "MPIDI_Init",
  "MPIDI_InitCompleted",
  "MPIDI_Iprobe",
  "MPIDI_Irecv",
  "MPIDI_Irsend",
  "MPIDI_Isend",
  "MPIDI_Issend",
  "MPIDI_Mprobe",
  "MPIDI_Mrecv",
  "MPIDI_Open_port",
  "MPIDI_Probe",
  "MPIDI_Progress_end",
  "MPIDI_Progress_poke",
  "MPIDI_Progress_start",
  "MPIDI_Progress_test",
  "MPIDI_Progress_wait",
  "MPIDI_Progress_register",
  "MPIDI_Progress_deregister",
  "MPIDI_Progress_activate",
  "MPIDI_Progress_deactivate",
  "MPIDI_Put",
  "MPIDI_Raccumulate",
  "MPIDI_Recv",
  "MPIDI_Recv_init",
  "MPIDI_Request_init",
  "MPIDI_Request_finalize",
  "MPIDI_Request_create_hook",
  "MPIDI_Request_free_hook",
  "MPIDI_Request_destroy_hook",
  "MPIDI_Request_complete",
  "MPIDI_Request_is_anysource",
  "MPIDI_Request_set_completed",
  "MPIDI_Rget",
  "MPIDI_Rget_accumulate",
  "MPIDI_Rput",
  "MPIDI_Rsend",
  "MPIDI_Rsend_init",
  "MPIDI_Send",
  "MPIDI_Send_init",
  "MPIDI_Ssend",
  "MPIDI_Ssend_init",
  "MPIDI_Startall",
  "MPIDI_Win_allocate",
  "MPIDI_Win_allocate_shared",
  "MPIDI_Win_attach",
  "MPIDI_Win_complete",
  "MPIDI_Win_create",
  "MPIDI_Win_create_dynamic",
  "MPIDI_Win_detach",
  "MPIDI_Win_fence",
  "MPIDI_Win_flush",
  "MPIDI_Win_flush_all",
  "MPIDI_Win_flush_local",
  "MPIDI_Win_flush_local_all",
  "MPIDI_Win_free",
  "MPIDI_Win_get_info",
  "MPIDI_Win_lock",
  "MPIDI_Win_lock_all",
  "MPIDI_Win_post",
  "MPIDI_Win_set_info",
  "MPIDI_Win_shared_query",
  "MPIDI_Win_start",
  "MPIDI_Win_sync",
  "MPIDI_Win_test",
  "MPIDI_Win_unlock",
  "MPIDI_Win_unlock_all",
  "MPIDI_Win_wait",
  "MPIDI_Get_node_id",
  "MPIDI_Get_max_node_id",
  "MPIDI_Request_is_pending_failure",
  "MPIDI_Aint_add",
  "MPIDI_Aint_diff",
  "MPIDI_Create_intercomm_from_lpids",
  /* Variables */
  "MPIDI_Request_direct",
  "MPIDI_Request_mem",
};

enum {
  I_MPIDI_Abort = 0,
  I_MPIDI_Accumulate,
  I_MPIDI_Alloc_mem,
  I_MPIDI_Bsend_init,
  I_MPIDI_Cancel_recv,
  I_MPIDI_Cancel_send,
  I_MPIDI_Close_port,
  I_MPIDI_Comm_create_hook,
  I_MPIDI_Comm_free_hook,
  I_MPIDI_Intercomm_exchange_map,
  I_MPIDI_Barrier,
  I_MPIDI_Bcast,
  I_MPIDI_Allreduce,
  I_MPIDI_Allgather,
  I_MPIDI_Allgatherv,
  I_MPIDI_Scatter,
  I_MPIDI_Scatterv,
  I_MPIDI_Gather,
  I_MPIDI_Gatherv,
  I_MPIDI_Alltoall,
  I_MPIDI_Alltoallv,
  I_MPIDI_Alltoallw,
  I_MPIDI_Reduce,
  I_MPIDI_Reduce_scatter,
  I_MPIDI_Reduce_scatter_block,
  I_MPIDI_Scan,
  I_MPIDI_Exscan,
  I_MPIDI_Neighbor_allgather,
  I_MPIDI_Neighbor_allgatherv,
  I_MPIDI_Neighbor_alltoallv,
  I_MPIDI_Neighbor_alltoallw,
  I_MPIDI_Neighbor_alltoall,
  I_MPIDI_Ineighbor_allgather,
  I_MPIDI_Ineighbor_allgatherv,
  I_MPIDI_Ineighbor_alltoall,
  I_MPIDI_Ineighbor_alltoallv,
  I_MPIDI_Ineighbor_alltoallw,
  I_MPIDI_Ibarrier,
  I_MPIDI_Ibcast,
  I_MPIDI_Iallgather,
  I_MPIDI_Iallgatherv,
  I_MPIDI_Iallreduce,
  I_MPIDI_Ialltoall,
  I_MPIDI_Ialltoallv,
  I_MPIDI_Ialltoallw,
  I_MPIDI_Iexscan,
  I_MPIDI_Igather,
  I_MPIDI_Igatherv,
  I_MPIDI_Ireduce_scatter_block,
  I_MPIDI_Ireduce_scatter,
  I_MPIDI_Ireduce,
  I_MPIDI_Iscan,
  I_MPIDI_Iscatter,
  I_MPIDI_Iscatterv,
  I_MPIDI_Comm_accept,
  I_MPIDI_Comm_connect,
  I_MPIDI_Comm_disconnect,
  I_MPIDI_Comm_group_failed,
  I_MPIDI_Comm_reenable_anysource,
  I_MPIDI_Comm_remote_group_failed,
  I_MPIDI_Comm_spawn_multiple,
  I_MPIDI_Comm_failure_get_acked,
  I_MPIDI_Comm_get_all_failed_procs,
  I_MPIDI_Comm_revoke,
  I_MPIDI_Comm_failure_ack,
  I_MPIDI_Comm_AS_enabled,
  I_MPIDI_Comm_get_lpid,
  I_MPIDI_Compare_and_swap,
  I_MPIDI_Fetch_and_op,
  I_MPIDI_Finalize,
  I_MPIDI_Free_mem,
  I_MPIDI_Get,
  I_MPIDI_Get_accumulate,
  I_MPIDI_Get_processor_name,
  I_MPIDI_Get_universe_size,
  I_MPIDI_Improbe,
  I_MPIDI_Imrecv,
  I_MPIDI_Init,
  I_MPIDI_InitCompleted,
  I_MPIDI_Iprobe,
  I_MPIDI_Irecv,
  I_MPIDI_Irsend,
  I_MPIDI_Isend,
  I_MPIDI_Issend,
  I_MPIDI_Mprobe,
  I_MPIDI_Mrecv,
  I_MPIDI_Open_port,
  I_MPIDI_Probe,
  I_MPIDI_Progress_end,
  I_MPIDI_Progress_poke,
  I_MPIDI_Progress_start,
  I_MPIDI_Progress_test,
  I_MPIDI_Progress_wait,
  I_MPIDI_Progress_register,
  I_MPIDI_Progress_deregister,
  I_MPIDI_Progress_activate,
  I_MPIDI_Progress_deactivate,
  I_MPIDI_Put,
  I_MPIDI_Raccumulate,
  I_MPIDI_Recv,
  I_MPIDI_Recv_init,
  I_MPIDI_Request_init,
  I_MPIDI_Request_finalize,
  I_MPIDI_Request_create_hook,
  I_MPIDI_Request_free_hook,
  I_MPIDI_Request_destroy_hook,
  I_MPIDI_Request_complete,
  I_MPIDI_Request_is_anysource,
  I_MPIDI_Request_set_completed,
  I_MPIDI_Rget,
  I_MPIDI_Rget_accumulate,
  I_MPIDI_Rput,
  I_MPIDI_Rsend,
  I_MPIDI_Rsend_init,
  I_MPIDI_Send,
  I_MPIDI_Send_init,
  I_MPIDI_Ssend,
  I_MPIDI_Ssend_init,
  I_MPIDI_Startall,
  I_MPIDI_Win_allocate,
  I_MPIDI_Win_allocate_shared,
  I_MPIDI_Win_attach,
  I_MPIDI_Win_complete,
  I_MPIDI_Win_create,
  I_MPIDI_Win_create_dynamic,
  I_MPIDI_Win_detach,
  I_MPIDI_Win_fence,
  I_MPIDI_Win_flush,
  I_MPIDI_Win_flush_all,
  I_MPIDI_Win_flush_local,
  I_MPIDI_Win_flush_local_all,
  I_MPIDI_Win_free,
  I_MPIDI_Win_get_info,
  I_MPIDI_Win_lock,
  I_MPIDI_Win_lock_all,
  I_MPIDI_Win_post,
  I_MPIDI_Win_set_info,
  I_MPIDI_Win_shared_query,
  I_MPIDI_Win_start,
  I_MPIDI_Win_sync,
  I_MPIDI_Win_test,
  I_MPIDI_Win_unlock,
  I_MPIDI_Win_unlock_all,
  I_MPIDI_Win_wait,
  I_MPIDI_Get_node_id,
  I_MPIDI_Get_max_node_id,
  I_MPIDI_Request_is_pending_failure,
  I_MPIDI_Aint_add,
  I_MPIDI_Aint_diff,
  I_MPIDI_Create_intercomm_from_lpids,
  /* Variables */
  I_MPIDI_Request_direct,
  I_MPIDI_Request_mem,
};

struct MPIDFunc _g_adi = {0};
static inline void *import(void       *dlopen_file,
                           const char *funcname)
{
  dlerror();    /* Clear any existing error */
  void *handle = NULL;
  char *error  = NULL;
  handle       = dlsym(dlopen_file, funcname);

  if((error = dlerror()) != NULL) {
    fprintf(stderr, "Error when loading ADI function %s: %s\n",
            funcname, error);
    handle = NULL;
  }

  return handle;
}


#define LDTABLE(x,y) table->x = import(dlopen_file, ADI_cmd_list[I_MPIDI_##y])
__attribute__((__constructor__))
int adi_load()
{
  void            *dlopen_file = NULL;
  struct MPIDFunc *table       = &_g_adi;
  char *filename = "libadiofi.so";
  char       *c        = getenv("MPICH_ADI_MODULE");

  if(!c) c=filename;

  dlopen_file = dlopen(c, RTLD_NOW | RTLD_GLOBAL);

  if(NULL == dlopen_file) {
    fprintf(stderr, "Error opening %s: %s\n", filename, dlerror());
    return -1;
  }

  LDTABLE(abort,Abort);
  LDTABLE(accumulate,Accumulate);
  LDTABLE(alloc_mem,Alloc_mem);
  LDTABLE(bsend_init,Bsend_init);
  LDTABLE(cancel_recv,Cancel_recv);
  LDTABLE(cancel_send,Cancel_send);
  LDTABLE(close_port,Close_port);
  LDTABLE(comm_create_hook,Comm_create_hook);
  LDTABLE(comm_free_hook,Comm_free_hook);
  LDTABLE(intercomm_exchange_map,Intercomm_exchange_map);
  LDTABLE(barrier,Barrier);
  LDTABLE(bcast,Bcast);
  LDTABLE(allreduce,Allreduce);
  LDTABLE(allgather,Allgather);
  LDTABLE(allgatherv,Allgatherv);
  LDTABLE(scatter,Scatter);
  LDTABLE(scatterv,Scatterv);
  LDTABLE(gather,Gather);
  LDTABLE(gatherv,Gatherv);
  LDTABLE(alltoall,Alltoall);
  LDTABLE(alltoallv,Alltoallv);
  LDTABLE(alltoallw,Alltoallw);
  LDTABLE(reduce,Reduce);
  LDTABLE(reduce_scatter,Reduce_scatter);
  LDTABLE(reduce_scatter_block,Reduce_scatter_block);
  LDTABLE(scan,Scan);
  LDTABLE(exscan,Exscan);
  LDTABLE(neighbor_allgather,Neighbor_allgather);
  LDTABLE(neighbor_allgatherv,Neighbor_allgatherv);
  LDTABLE(neighbor_alltoallv,Neighbor_alltoallv);
  LDTABLE(neighbor_alltoallw,Neighbor_alltoallw);
  LDTABLE(neighbor_alltoall,Neighbor_alltoall);
  LDTABLE(ineighbor_allgather,Ineighbor_allgather);
  LDTABLE(ineighbor_allgatherv,Ineighbor_allgatherv);
  LDTABLE(ineighbor_alltoall,Ineighbor_alltoall);
  LDTABLE(ineighbor_alltoallv,Ineighbor_alltoallv);
  LDTABLE(ineighbor_alltoallw,Ineighbor_alltoallw);
  LDTABLE(ibarrier,Ibarrier);
  LDTABLE(ibcast,Ibcast);
  LDTABLE(iallgather,Iallgather);
  LDTABLE(iallgatherv,Iallgatherv);
  LDTABLE(iallreduce,Iallreduce);
  LDTABLE(ialltoall,Ialltoall);
  LDTABLE(ialltoallv,Ialltoallv);
  LDTABLE(ialltoallw,Ialltoallw);
  LDTABLE(iexscan,Iexscan);
  LDTABLE(igather,Igather);
  LDTABLE(igatherv,Igatherv);
  LDTABLE(ireduce_scatter_block,Ireduce_scatter_block);
  LDTABLE(ireduce_scatter,Ireduce_scatter);
  LDTABLE(ireduce,Ireduce);
  LDTABLE(iscan,Iscan);
  LDTABLE(iscatter,Iscatter);
  LDTABLE(iscatterv,Iscatterv);
  LDTABLE(comm_accept,Comm_accept);
  LDTABLE(comm_connect,Comm_connect);
  LDTABLE(comm_disconnect,Comm_disconnect);
  LDTABLE(comm_group_failed,Comm_group_failed);
  LDTABLE(comm_reenable_anysource,Comm_reenable_anysource);
  LDTABLE(comm_remote_group_failed,Comm_remote_group_failed);
  LDTABLE(comm_spawn_multiple,Comm_spawn_multiple);
  LDTABLE(comm_failure_get_acked,Comm_failure_get_acked);
  LDTABLE(comm_get_all_failed_procs,Comm_get_all_failed_procs);
  LDTABLE(comm_revoke,Comm_revoke);
  LDTABLE(comm_failure_ack,Comm_failure_ack);
  LDTABLE(comm_AS_enabled,Comm_AS_enabled);
  LDTABLE(comm_get_lpid,Comm_get_lpid);
  LDTABLE(compare_and_swap,Compare_and_swap);
  LDTABLE(fetch_and_op,Fetch_and_op);
  LDTABLE(finalize,Finalize);
  LDTABLE(free_mem,Free_mem);
  LDTABLE(get,Get);
  LDTABLE(get_accumulate,Get_accumulate);
  LDTABLE(get_processor_name,Get_processor_name);
  LDTABLE(get_universe_size,Get_universe_size);
  LDTABLE(improbe,Improbe);
  LDTABLE(imrecv,Imrecv);
  LDTABLE(init,Init);
  LDTABLE(initcompleted,InitCompleted);
  LDTABLE(iprobe,Iprobe);
  LDTABLE(irecv,Irecv);
  LDTABLE(irsend,Irsend);
  LDTABLE(isend,Isend);
  LDTABLE(issend,Issend);
  LDTABLE(mprobe,Mprobe);
  LDTABLE(mrecv,Mrecv);
  LDTABLE(open_port,Open_port);
  LDTABLE(probe,Probe);
  LDTABLE(progress_end,Progress_end);
  LDTABLE(progress_poke,Progress_poke);
  LDTABLE(progress_start,Progress_start);
  LDTABLE(progress_test,Progress_test);
  LDTABLE(progress_wait,Progress_wait);
  LDTABLE(progress_register,Progress_register);
  LDTABLE(progress_deregister,Progress_deregister);
  LDTABLE(progress_activate,Progress_activate);
  LDTABLE(progress_deactivate,Progress_deactivate);
  LDTABLE(put,Put);
  LDTABLE(raccumulate,Raccumulate);
  LDTABLE(recv,Recv);
  LDTABLE(recv_init,Recv_init);
  LDTABLE(request_init,Request_init);
  LDTABLE(request_finalize,Request_finalize);
  LDTABLE(request_create_hook,Request_create_hook);
  LDTABLE(request_free_hook,Request_free_hook);
  LDTABLE(request_destroy_hook,Request_destroy_hook);
  LDTABLE(request_complete,Request_complete);
  LDTABLE(request_is_anysource,Request_is_anysource);
  LDTABLE(request_set_completed,Request_set_completed);
  LDTABLE(rget,Rget);
  LDTABLE(rget_accumulate,Rget_accumulate);
  LDTABLE(rput,Rput);
  LDTABLE(rsend,Rsend);
  LDTABLE(rsend_init,Rsend_init);
  LDTABLE(send,Send);
  LDTABLE(send_init,Send_init);
  LDTABLE(ssend,Ssend);
  LDTABLE(ssend_init,Ssend_init);
  LDTABLE(startall,Startall);
  LDTABLE(win_allocate,Win_allocate);
  LDTABLE(win_allocate_shared,Win_allocate_shared);
  LDTABLE(win_attach,Win_attach);
  LDTABLE(win_complete,Win_complete);
  LDTABLE(win_create,Win_create);
  LDTABLE(win_create_dynamic,Win_create_dynamic);
  LDTABLE(win_detach,Win_detach);
  LDTABLE(win_fence,Win_fence);
  LDTABLE(win_flush,Win_flush);
  LDTABLE(win_flush_all,Win_flush_all);
  LDTABLE(win_flush_local,Win_flush_local);
  LDTABLE(win_flush_local_all,Win_flush_local_all);
  LDTABLE(win_free,Win_free);
  LDTABLE(win_get_info,Win_get_info);
  LDTABLE(win_lock,Win_lock);
  LDTABLE(win_lock_all,Win_lock_all);
  LDTABLE(win_post,Win_post);
  LDTABLE(win_set_info,Win_set_info);
  LDTABLE(win_shared_query,Win_shared_query);
  LDTABLE(win_start,Win_start);
  LDTABLE(win_sync,Win_sync);
  LDTABLE(win_test,Win_test);
  LDTABLE(win_unlock,Win_unlock);
  LDTABLE(win_unlock_all,Win_unlock_all);
  LDTABLE(win_wait,Win_wait);
  LDTABLE(get_node_id,Get_node_id);
  LDTABLE(get_max_node_id,Get_max_node_id);
  LDTABLE(request_is_pending_failure,Request_is_pending_failure);
  LDTABLE(aint_add,Aint_add);
  LDTABLE(aint_diff,Aint_diff);
  LDTABLE(create_intercomm_from_lpids,Create_intercomm_from_lpids);

  /* Variables */
  LDTABLE(request_direct,Request_direct);
  LDTABLE(request_mem,Request_mem);
  return 0;
}
#else

#ifdef ADI_DEVICE_OFI
#include "../ofi/src/mpid_globals.c"
#endif

#ifdef ADI_DEVICE_STUB
#include "../stub/src/mpid_globals.c"
#endif

#endif /* MPIDI_USE_DYNAMIC_ADI */

/* Hack for broken MPICH header file */
#undef MPID_Abort
int MPID_Abort(MPIR_Comm  *comm,
               int         mpi_errno,
               int         exit_code,
               const char *error_msg)
{
  int err = MPI_SUCCESS;
  assert(0);
  return err;
}
