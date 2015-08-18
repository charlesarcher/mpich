/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#if !defined(MPICH_MPIDFUNC_H_INCLUDED)
#define MPICH_MPIDFUNC_H_INCLUDED


#ifdef MPIDI_USE_DYNAMIC_ADI
#define __ADI_INLINE__

#define DECL_API(rc,ptrname,fcnname,...)         \
  typedef rc (*ptrname)(__VA_ARGS__);            \
  rc MPIDI_##fcnname(__VA_ARGS__)

#define __SI__  static inline

#else  /*MPIDI_USE_DYNAMIC_ADI*/
#define __ADI_INLINE__ static inline
#define DECL_API(rc,ptrname,fcnname,...)         \
  typedef rc (*ptrname)(__VA_ARGS__);            \
  __ADI_INLINE__ rc MPIDI_##fcnname(__VA_ARGS__) \
  __attribute__((always_inline))

#define __SI__ __attribute__((always_inline)) static inline
#endif

/* Example: expands to                                      */
/* typedef int (*init_fn)(int*,char***,int,int*,int*,int*); */
/* int MPIDI_Init(int*,char***,int*,int*,int*);             */

DECL_API(int,init_fn,Init,int *,char ** *,int,int *,int *,int *);
DECL_API(int,initcompleted_fn,InitCompleted,void);
DECL_API(int,abort_fn,Abort,MPIR_Comm *,int,int,const char *);
DECL_API(int,cancel_recv_fn,Cancel_recv,MPIR_Request *);
DECL_API(int,cancel_send_fn,Cancel_send,MPIR_Request *);
DECL_API(int,comm_disconnect_fn,Comm_disconnect,MPIR_Comm *);
DECL_API(int,comm_spawn_multiple_fn,Comm_spawn_multiple,
         int,char *[],char **[],const int[],MPIR_Info*[],int,
         MPIR_Comm *,MPIR_Comm **,int[]);
DECL_API(int,comm_failure_get_acked_fn,Comm_failure_get_acked,
         MPIR_Comm *,MPIR_Group **);
DECL_API(int,comm_get_all_failed_procs_fn,Comm_get_all_failed_procs,
         MPIR_Comm *,MPIR_Group **, int);
DECL_API(int,comm_revoke_fn,Comm_revoke,MPIR_Comm *,int);
DECL_API(int,comm_failure_ack_fn,Comm_failure_ack,MPIR_Comm *);
DECL_API(int,comm_AS_enabled_fn,Comm_AS_enabled,MPIR_Comm *);
DECL_API(int,comm_get_lpid_fn,Comm_get_lpid,MPIR_Comm *,int,int*,MPL_bool);
DECL_API(int,finalize_fn,Finalize,void);
DECL_API(int,get_universe_size_fn,Get_universe_size,int *);
DECL_API(int,get_processor_name_fn,Get_processor_name,
         char *,int,int *);
DECL_API(int,iprobe_fn,Iprobe,int,int,MPIR_Comm *,
         int,int *,MPI_Status *);
DECL_API(int,irecv_fn,Irecv,void *,int,MPI_Datatype,int,int,
         MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,isend_fn,Isend,const void *,int,MPI_Datatype,
         int,int,MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,issend_fn,Issend,const void *,int,MPI_Datatype,
         int,int,MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,mrecv_fn,Mrecv,void *,int,MPI_Datatype,
         MPIR_Request *,MPI_Status *);
DECL_API(int,imrecv_fn,Imrecv,void *,int,MPI_Datatype,
         MPIR_Request *,MPIR_Request **);
DECL_API(int,open_port_fn,Open_port,MPIR_Info *,char *);
DECL_API(int,close_port_fn,Close_port,const char *);
DECL_API(int,comm_accept_fn,Comm_accept,const char *,
         MPIR_Info *,int,MPIR_Comm *,MPIR_Comm **);
DECL_API(int,comm_connect_fn,Comm_connect,const char *,
         MPIR_Info *, int, MPIR_Comm *, MPIR_Comm **);
DECL_API(int,probe_fn,Probe,int,int,MPIR_Comm *,int,MPI_Status *);
DECL_API(int,mprobe_fn,Mprobe,int,int,MPIR_Comm *,int,
         MPIR_Request **,MPI_Status *);
DECL_API(int,improbe_fn,Improbe,int,int,MPIR_Comm *,int,
         int *,MPIR_Request **,MPI_Status *);
DECL_API(int,progress_test_fn,Progress_test,void);
DECL_API(int,progress_poke_fn,Progress_poke,void);
DECL_API(void,progress_start_fn,Progress_start,MPID_Progress_state *);
DECL_API(void,progress_end_fn,Progress_end,MPID_Progress_state *);
DECL_API(int,progress_wait_fn,Progress_wait,MPID_Progress_state *);
DECL_API(int,progress_register_fn,Progress_register,int (*progress_fn)(int*),int*);
DECL_API(int,progress_deregister_fn,Progress_deregister,int);
DECL_API(int,progress_activate_fn,Progress_activate,int);
DECL_API(int,progress_deactivate_fn,Progress_deactivate,int);
DECL_API(int,recv_fn,Recv,void *,int,MPI_Datatype,int,int,MPIR_Comm *,
         int,MPI_Status *,MPIR_Request **);
DECL_API(int,recv_init_fn,Recv_init,void *,int,MPI_Datatype,int,int,
         MPIR_Comm *,int,MPIR_Request **);
DECL_API(void,request_set_completed_fn,Request_set_completed,MPIR_Request *);
DECL_API(void,request_free_hook_fn,Request_free_hook,MPIR_Request *);
DECL_API(void,request_destroy_hook_fn,Request_destroy_hook,MPIR_Request *);
DECL_API(int,request_complete_fn,Request_complete,MPIR_Request *);
DECL_API(int,request_is_anysource_fn,Request_is_anysource,MPIR_Request *);
DECL_API(void,request_init_fn,Request_init,MPIR_Request *);
DECL_API(void,request_finalize_fn,Request_finalize,MPIR_Request *);
DECL_API(void,request_create_hook_fn,Request_create_hook,MPIR_Request *);
DECL_API(int,send_fn,Send,const void *,int,MPI_Datatype,int,int,
         MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,ssend_fn,Ssend,const void *,int,MPI_Datatype,int,int,
         MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,rsend_fn,Rsend,const void *,int,MPI_Datatype,int,int,
         MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,irsend_fn,Irsend,const void *,int,MPI_Datatype,int,int,
         MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,send_init_fn,Send_init,const void *,int,MPI_Datatype,int,
         int,MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,ssend_init_fn,Ssend_init,const void *,int,MPI_Datatype,int,
         int,MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,bsend_init_fn,Bsend_init,const void *,int,MPI_Datatype,int,
         int,MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,rsend_init_fn,Rsend_init,const void *,int,MPI_Datatype,int,
         int,MPIR_Comm *,int,MPIR_Request **);
DECL_API(int,startall_fn,Startall,int,MPIR_Request*[]);
DECL_API(int,accumulate_fn,Accumulate,const void *,int,MPI_Datatype,int,
         MPI_Aint,int,MPI_Datatype,MPI_Op,MPIR_Win *);
DECL_API(int,win_create_fn,Win_create,void *,MPI_Aint,int,MPIR_Info *,
         MPIR_Comm *,MPIR_Win **);
DECL_API(int,win_fence_fn,Win_fence,int,MPIR_Win *);
DECL_API(int,win_free_fn,Win_free,MPIR_Win **);
DECL_API(int,get_fn,Get,void *,int,MPI_Datatype,int,MPI_Aint,
         int,MPI_Datatype,MPIR_Win *);
DECL_API(int,win_get_info_fn,Win_get_info,MPIR_Win *,MPIR_Info **);
DECL_API(int,win_lock_fn,Win_lock,int,int,int,MPIR_Win *);
DECL_API(int,win_unlock_fn,Win_unlock,int,MPIR_Win *);
DECL_API(int,win_start_fn,Win_start,MPIR_Group *,int,MPIR_Win *);
DECL_API(int,win_complete_fn,Win_complete,MPIR_Win *);
DECL_API(int,win_post_fn,Win_post,MPIR_Group *,int,MPIR_Win *);
DECL_API(int,win_wait_fn,Win_wait,MPIR_Win *);
DECL_API(int,win_test_fn,Win_test,MPIR_Win *,int *);
DECL_API(int,put_fn,Put,const void *,int,MPI_Datatype,int,MPI_Aint,
         int,MPI_Datatype,MPIR_Win *);
DECL_API(int,win_set_info_fn,Win_set_info,MPIR_Win *, MPIR_Info *);
DECL_API(int,comm_reenable_anysource_fn,Comm_reenable_anysource,
         MPIR_Comm *, MPIR_Group **);
DECL_API(int,comm_remote_group_failed_fn,Comm_remote_group_failed,
         MPIR_Comm *, MPIR_Group **);
DECL_API(int,comm_group_failed_fn,Comm_group_failed,
         MPIR_Comm *, MPIR_Group **);
DECL_API(int,win_attach_fn,Win_attach,MPIR_Win *,void *,MPI_Aint);
DECL_API(int,win_allocate_shared_fn,Win_allocate_shared,MPI_Aint,
         int,MPIR_Info *,MPIR_Comm *,void **,MPIR_Win **);
DECL_API(int,rput_fn,Rput,const void *,int,MPI_Datatype,int,MPI_Aint,
         int,MPI_Datatype,MPIR_Win *,MPIR_Request **);
DECL_API(int,win_flush_local_fn,Win_flush_local,int,MPIR_Win *);
DECL_API(int,win_detach_fn,Win_detach,MPIR_Win *,const void *);
DECL_API(int,compare_and_swap_fn,Compare_and_swap,const void *,
         const void *,void *,MPI_Datatype,int,MPI_Aint,MPIR_Win *);
DECL_API(int,raccumulate_fn,Raccumulate,const void *,int,MPI_Datatype,
         int,MPI_Aint,int,MPI_Datatype,MPI_Op,MPIR_Win *,MPIR_Request **);
DECL_API(int,rget_accumulate_fn,Rget_accumulate,const void *,int,MPI_Datatype,
         void *,int,MPI_Datatype,int,MPI_Aint,int,MPI_Datatype,MPI_Op,
         MPIR_Win *,MPIR_Request **);
DECL_API(int,fetch_and_op_fn,Fetch_and_op,const void *,void *,MPI_Datatype,
         int,MPI_Aint,MPI_Op,MPIR_Win *);
DECL_API(int,win_shared_query_fn,Win_shared_query,MPIR_Win *,int,MPI_Aint *,
         int *, void *);
DECL_API(int,win_allocate_fn,Win_allocate,MPI_Aint,int,MPIR_Info *,
         MPIR_Comm *,void *,MPIR_Win **);
DECL_API(int,win_flush_fn,Win_flush,int,MPIR_Win *);
DECL_API(int,win_flush_local_all_fn,Win_flush_local_all,MPIR_Win *);
DECL_API(int,win_unlock_all_fn,Win_unlock_all,MPIR_Win *);
DECL_API(int,win_create_dynamic_fn,Win_create_dynamic,MPIR_Info *,
         MPIR_Comm *,MPIR_Win **);
DECL_API(int,rget_fn,Rget,void *,int,MPI_Datatype,int,MPI_Aint,int,
         MPI_Datatype,MPIR_Win *,MPIR_Request **);
DECL_API(int,win_sync_fn,Win_sync,MPIR_Win *);
DECL_API(int,win_flush_all_fn,Win_flush_all,MPIR_Win *);
DECL_API(int,get_accumulate_fn,Get_accumulate,const void *,int,
         MPI_Datatype,void *,int,MPI_Datatype,int,MPI_Aint,
         int,MPI_Datatype,MPI_Op,MPIR_Win *);
DECL_API(int,win_lock_all_fn,Win_lock_all,int,MPIR_Win *);
DECL_API(void *,alloc_mem_fn,Alloc_mem,size_t,MPIR_Info *);
DECL_API(int,free_mem_fn,Free_mem,void *);
DECL_API(int,get_node_id_fn,Get_node_id,MPIR_Comm *,int rank,MPID_Node_id_t *);
DECL_API(int,get_max_node_id_fn,Get_max_node_id,MPIR_Comm *,MPID_Node_id_t *);
DECL_API(int,request_is_pending_failure_fn,Request_is_pending_failure,MPIR_Request *);
DECL_API(MPI_Aint,aint_add_fn,Aint_add,MPI_Aint,MPI_Aint);
DECL_API(MPI_Aint,aint_diff_fn,Aint_diff,MPI_Aint,MPI_Aint);
DECL_API(int,create_intercomm_from_lpids_fn,Create_intercomm_from_lpids,MPIR_Comm *,int,const int[]);
DECL_API(int,comm_create_hook_fn,Comm_create_hook,MPIR_Comm *);
DECL_API(int,comm_free_hook_fn,Comm_free_hook,MPIR_Comm *);
DECL_API(int,intercomm_exchange_map_fn,intercomm_exchange_map,MPIR_Comm *,int,MPIR_Comm*,
         int,int *,int **,int *);

/* Collectives */
DECL_API(int,barrier_fn,Barrier,MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,bcast_fn,Bcast,void*, int, MPI_Datatype,int, MPIR_Comm*, MPIR_Errflag_t*);
DECL_API(int,allreduce_fn,Allreduce,const void*, void*,int,MPI_Datatype,
         MPI_Op,MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,allgather_fn,Allgather,const void *,int,MPI_Datatype,
         void *,int,MPI_Datatype, MPIR_Comm *,MPIR_Errflag_t *);
DECL_API(int,allgatherv_fn,Allgatherv,const void *,int,MPI_Datatype,
         void *,const int*, const int*,MPI_Datatype, MPIR_Comm *,
         MPIR_Errflag_t *);
DECL_API(int,scatter_fn,Scatter,const void*,int,MPI_Datatype,void*,
         int,MPI_Datatype,int,MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,scatterv_fn,Scatterv,const void*,const int*,const int*,
         MPI_Datatype,void*,int,MPI_Datatype,int,MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,gather_fn,Gather,const void *, int, MPI_Datatype,
         void *, int, MPI_Datatype,int, MPIR_Comm*, MPIR_Errflag_t*);
DECL_API(int,gatherv_fn,Gatherv,const void*,int,MPI_Datatype,void*,
         const int*, const int*,MPI_Datatype,int,MPIR_Comm*,
         MPIR_Errflag_t *);
DECL_API(int,alltoall_fn,Alltoall,const void*,int,MPI_Datatype, void*,
         int,MPI_Datatype, MPIR_Comm*,MPIR_Errflag_t *);
DECL_API(int,alltoallv_fn,Alltoallv,const void*,const int *,const int*,
         MPI_Datatype,void*,const int*,const int*,MPI_Datatype,MPIR_Comm*,
         MPIR_Errflag_t*);
DECL_API(int,alltoallw_fn,Alltoallw,const void*,const int[],const int[],
         const MPI_Datatype[],void*,const int[],const int[],const MPI_Datatype[],
         MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,reduce_fn,Reduce,const void*,void*,int,MPI_Datatype,MPI_Op,int,
         MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,reduce_scatter_fn,Reduce_scatter,const void*,void*,const int[],
         MPI_Datatype,MPI_Op,MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,reduce_scatter_block_fn,Reduce_scatter_block,const void*,
         void*,int,MPI_Datatype,MPI_Op,MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,scan_fn,Scan,const void*,void*,int,MPI_Datatype,MPI_Op,
         MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,exscan_fn,Exscan,const void*,void*,int,MPI_Datatype,MPI_Op,
         MPIR_Comm*,MPIR_Errflag_t*);
DECL_API(int,neighbor_allgather_fn,Neighbor_allgather,const void*,int,
         MPI_Datatype,void*,int,MPI_Datatype,MPIR_Comm*);
DECL_API(int,neighbor_allgatherv_fn,Neighbor_allgatherv,const void*,int,
         MPI_Datatype,void*,const int*, const int*,MPI_Datatype,
         MPIR_Comm*);
DECL_API(int,neighbor_alltoallv_fn,Neighbor_alltoallv,const void*,const int*,
         const int*,MPI_Datatype,void*,const int*,const int*,MPI_Datatype,
         MPIR_Comm*);
DECL_API(int,neighbor_alltoallw_fn,Neighbor_alltoallw,const void*,
         const int*,const MPI_Aint *,const MPI_Datatype*,void*,
         const int*,const MPI_Aint*,const MPI_Datatype *,
         MPIR_Comm *);
DECL_API(int,neighbor_alltoall_fn,Neighbor_alltoall,const void*,int,
         MPI_Datatype,void*,int,MPI_Datatype,MPIR_Comm *);
DECL_API(int,ineighbor_allgather_fn,Ineighbor_allgather,const void*,int,
         MPI_Datatype,void*,int,MPI_Datatype,MPIR_Comm*,MPI_Request*);
DECL_API(int,ineighbor_allgatherv_fn,Ineighbor_allgatherv,const void*,int,
         MPI_Datatype,void*,const int*,const int*,MPI_Datatype,MPIR_Comm*,
         MPI_Request*);
DECL_API(int,ineighbor_alltoall_fn,Ineighbor_alltoall,const void*,int,
         MPI_Datatype,void*,int,MPI_Datatype,MPIR_Comm*,MPI_Request*);
DECL_API(int,ineighbor_alltoallv_fn,Ineighbor_alltoallv,const void*,const int*,
         const int*,MPI_Datatype,void*,const int*,const int*,MPI_Datatype,
         MPIR_Comm*,MPI_Request*);
DECL_API(int,ineighbor_alltoallw_fn,Ineighbor_alltoallw,const void*,const int*,
         const MPI_Aint*,const MPI_Datatype*,void*,const int*,const MPI_Aint*,
         const MPI_Datatype*,MPIR_Comm*,MPI_Request*);
DECL_API(int,ibarrier_fn,Ibarrier,MPIR_Comm*,MPI_Request*);
DECL_API(int,ibcast_fn,Ibcast,void*,int,MPI_Datatype,int,MPIR_Comm*,MPI_Request*);
DECL_API(int,iallgather_fn,Iallgather,const void*,int,MPI_Datatype,void*,int,
         MPI_Datatype,MPIR_Comm*,MPI_Request*);
DECL_API(int,iallgatherv_fn,Iallgatherv,const void*,int,MPI_Datatype,void*,
         const int*,const int*,MPI_Datatype,MPIR_Comm*,MPI_Request*);
DECL_API(int,iallreduce_fn,Iallreduce,const void*,void*,int,MPI_Datatype,
         MPI_Op,MPIR_Comm*,MPI_Request*);
DECL_API(int,ialltoall_fn,Ialltoall,const void*,int,MPI_Datatype,void*,
         int,MPI_Datatype,MPIR_Comm*,MPI_Request*);
DECL_API(int,ialltoallv_fn,Ialltoallv,const void*,const int*,const int*,
         MPI_Datatype,void*,const int*,const int*,MPI_Datatype,
         MPIR_Comm*,MPI_Request*);
DECL_API(int,ialltoallw_fn,Ialltoallw,const void*,const int*,const int*,
         const MPI_Datatype*,void*,const int*,const int*,const MPI_Datatype*,
         MPIR_Comm*,MPI_Request*);
DECL_API(int,iexscan_fn,Iexscan,const void*,void*,int,MPI_Datatype,MPI_Op,
         MPIR_Comm*,MPI_Request*);
DECL_API(int,igather_fn,Igather,const void*,int,MPI_Datatype,void*,int,
         MPI_Datatype,int,MPIR_Comm*,MPI_Request*);
DECL_API(int,igatherv_fn,Igatherv,const void*,int,MPI_Datatype,void*,
         const int*,const int*,MPI_Datatype,int,MPIR_Comm*,MPI_Request*);
DECL_API(int,ireduce_scatter_block_fn,Ireduce_scatter_block,const void*,
         void*,int,MPI_Datatype,MPI_Op,MPIR_Comm*,MPI_Request*);
DECL_API(int,ireduce_scatter_fn,Ireduce_scatter,const void*,void*,
         const int*,MPI_Datatype,MPI_Op,MPIR_Comm*,MPI_Request*);
DECL_API(int,ireduce_fn,Ireduce,const void*, void*,int,MPI_Datatype,
         MPI_Op,int,MPIR_Comm*,MPI_Request*);
DECL_API(int,iscan_fn,Iscan,const void*,void*,int,MPI_Datatype,MPI_Op,
         MPIR_Comm*,MPI_Request*);
DECL_API(int,iscatter_fn,Iscatter,const void*,int,MPI_Datatype,void*,
         int,MPI_Datatype,int,MPIR_Comm*,MPI_Request*);
DECL_API(int,iscatterv_fn,Iscatterv,const void*,const int*,
         const int*,MPI_Datatype,void*,int,MPI_Datatype,
         int, MPIR_Comm*,MPI_Request*);

/* Dynamically loadable API */
/* MPID Function typedefs */
#ifdef MPIDI_USE_DYNAMIC_ADI

/* MPID Function table layout */
#define DECL_F(x) x##_fn x
struct MPIDFunc {
  /* mpid_init.c */
  DECL_F(init);
  DECL_F(initcompleted);
  DECL_F(get_universe_size);
  DECL_F(get_processor_name);
  DECL_F(finalize);
  DECL_F(abort);
  DECL_F(free_mem);
  DECL_F(alloc_mem);
  DECL_F(create_intercomm_from_lpids);

  /* mpid_comm.c */
  DECL_F(comm_accept);
  DECL_F(comm_connect);
  DECL_F(comm_disconnect);
  DECL_F(comm_spawn_multiple);
  DECL_F(comm_failure_get_acked);
  DECL_F(comm_get_all_failed_procs);
  DECL_F(comm_revoke);
  DECL_F(comm_failure_ack);
  DECL_F(comm_AS_enabled);
  DECL_F(comm_get_lpid);

  DECL_F(comm_reenable_anysource);
  DECL_F(comm_remote_group_failed);
  DECL_F(comm_group_failed);
  DECL_F(open_port);
  DECL_F(close_port);
  DECL_F(comm_create_hook);
  DECL_F(comm_free_hook);
  DECL_F(intercomm_exchange_map);

  DECL_F(barrier);
  DECL_F(bcast);
  DECL_F(allreduce);
  DECL_F(allgather);
  DECL_F(allgatherv);
  DECL_F(scatter);
  DECL_F(scatterv);
  DECL_F(gather);
  DECL_F(gatherv);
  DECL_F(alltoall);
  DECL_F(alltoallv);
  DECL_F(alltoallw);
  DECL_F(reduce);
  DECL_F(reduce_scatter);
  DECL_F(reduce_scatter_block);
  DECL_F(scan);
  DECL_F(exscan);
  DECL_F(neighbor_allgather);
  DECL_F(neighbor_allgatherv);
  DECL_F(neighbor_alltoallv);
  DECL_F(neighbor_alltoallw);
  DECL_F(neighbor_alltoall);
  DECL_F(ineighbor_allgather);
  DECL_F(ineighbor_allgatherv);
  DECL_F(ineighbor_alltoall);
  DECL_F(ineighbor_alltoallv);
  DECL_F(ineighbor_alltoallw);
  DECL_F(ibarrier);
  DECL_F(ibcast);
  DECL_F(iallgather);
  DECL_F(iallgatherv);
  DECL_F(iallreduce);
  DECL_F(ialltoall);
  DECL_F(ialltoallv);
  DECL_F(ialltoallw);
  DECL_F(iexscan);
  DECL_F(igather);
  DECL_F(igatherv);
  DECL_F(ireduce_scatter_block);
  DECL_F(ireduce_scatter);
  DECL_F(ireduce);
  DECL_F(iscan);
  DECL_F(iscatter);
  DECL_F(iscatterv);

  /* mpid_probe.c */
  DECL_F(iprobe);
  DECL_F(probe);
  DECL_F(mprobe);
  DECL_F(improbe);

  /* mpid_recv.c */
  DECL_F(recv);
  DECL_F(recv_init);
  DECL_F(irecv);
  DECL_F(mrecv);
  DECL_F(imrecv);
  DECL_F(cancel_recv);

  /* mpid_send.c */
  DECL_F(send);
  DECL_F(ssend);
  DECL_F(rsend);
  DECL_F(irsend);
  DECL_F(isend);
  DECL_F(issend);
  DECL_F(send_init);
  DECL_F(ssend_init);
  DECL_F(bsend_init);
  DECL_F(rsend_init);
  DECL_F(cancel_send);
  DECL_F(startall);

  /* mpid_progress.c */
  DECL_F(progress_test);
  DECL_F(progress_poke);
  DECL_F(progress_start);
  DECL_F(progress_end);
  DECL_F(progress_wait);
  DECL_F(progress_register);
  DECL_F(progress_deregister);
  DECL_F(progress_activate);
  DECL_F(progress_deactivate);

  /* mpid_request.c */
  DECL_F(request_set_completed);
  DECL_F(request_free_hook);
  DECL_F(request_destroy_hook);
  DECL_F(request_complete);
  DECL_F(request_is_anysource);
  DECL_F(request_init);
  DECL_F(request_finalize);
  DECL_F(request_create_hook);

  /* mpid_win.c */
  DECL_F(win_create);
  DECL_F(win_fence);
  DECL_F(win_complete);
  DECL_F(win_free);
  DECL_F(win_get_info);
  DECL_F(win_lock);
  DECL_F(win_unlock);
  DECL_F(win_start);
  DECL_F(win_post);
  DECL_F(win_wait);
  DECL_F(win_test);
  DECL_F(win_set_info);
  DECL_F(win_attach);
  DECL_F(win_allocate_shared);
  DECL_F(win_flush_local);
  DECL_F(win_detach);
  DECL_F(win_shared_query);
  DECL_F(win_allocate);
  DECL_F(win_flush);
  DECL_F(win_flush_local_all);
  DECL_F(win_unlock_all);
  DECL_F(win_create_dynamic);
  DECL_F(win_sync);
  DECL_F(win_flush_all);
  DECL_F(win_lock_all);
  DECL_F(get);
  DECL_F(accumulate);
  DECL_F(put);
  DECL_F(rput);
  DECL_F(compare_and_swap);
  DECL_F(raccumulate);
  DECL_F(rget_accumulate);
  DECL_F(fetch_and_op);
  DECL_F(rget);
  DECL_F(get_accumulate);

  DECL_F(get_node_id);
  DECL_F(get_max_node_id);
  DECL_F(request_is_pending_failure);
  DECL_F(aint_add);
  DECL_F(aint_diff);

  /* Variables */
  MPIR_Request                    *request_direct;
  MPIR_Object_alloc_t             *request_mem;
};

extern struct MPIDFunc _g_adi;
/*#define DECL_F(x) x##_fnx*/

#define FUNC_OBJ _g_adi
#define CALL_F(obj,x) obj.x

#define MPID_Abort                    CALL_F(FUNC_OBJ,abort)
#define MPID_Accumulate               CALL_F(FUNC_OBJ,accumulate)
#define MPID_Alloc_mem                CALL_F(FUNC_OBJ,alloc_mem)
#define MPID_Bsend_init               CALL_F(FUNC_OBJ,bsend_init)
#define MPID_Cancel_recv              CALL_F(FUNC_OBJ,cancel_recv)
#define MPID_Cancel_send              CALL_F(FUNC_OBJ,cancel_send)
#define MPID_Close_port               CALL_F(FUNC_OBJ,close_port)
#define MPID_Comm_accept              CALL_F(FUNC_OBJ,comm_accept)
#define MPID_Comm_connect             CALL_F(FUNC_OBJ,comm_connect)
#define MPID_Comm_disconnect          CALL_F(FUNC_OBJ,comm_disconnect)
#define MPID_Comm_group_failed        CALL_F(FUNC_OBJ,comm_group_failed)
#define MPID_Comm_reenable_anysource  CALL_F(FUNC_OBJ,comm_reenable_anysource)
#define MPID_Comm_remote_group_failed CALL_F(FUNC_OBJ,comm_remote_group_failed)
#define MPID_Comm_spawn_multiple      CALL_F(FUNC_OBJ,comm_spawn_multiple)
#define MPID_Comm_failure_get_acked   CALL_F(FUNC_OBJ,comm_failure_get_acked)
#define MPID_Comm_get_all_failed_procs CALL_F(FUNC_OBJ,comm_get_all_failed_procs)
#define MPID_Comm_revoke              CALL_F(FUNC_OBJ,comm_revoke)
#define MPID_Comm_failure_ack         CALL_F(FUNC_OBJ,comm_failure_ack)
#define MPID_Comm_AS_enabled          CALL_F(FUNC_OBJ,comm_AS_enabled)
#define MPID_Comm_get_lpid            CALL_F(FUNC_OBJ,comm_get_lpid)
#define MPID_Compare_and_swap         CALL_F(FUNC_OBJ,compare_and_swap)
#define MPID_Fetch_and_op             CALL_F(FUNC_OBJ,fetch_and_op)
#define MPID_Finalize                 CALL_F(FUNC_OBJ,finalize)
#define MPID_Free_mem                 CALL_F(FUNC_OBJ,free_mem)
#define MPID_Get                      CALL_F(FUNC_OBJ,get)
#define MPID_Get_accumulate           CALL_F(FUNC_OBJ,get_accumulate)
#define MPID_Get_processor_name       CALL_F(FUNC_OBJ,get_processor_name)
#define MPID_Get_universe_size        CALL_F(FUNC_OBJ,get_universe_size)
#define MPID_Improbe                  CALL_F(FUNC_OBJ,improbe)
#define MPID_Imrecv                   CALL_F(FUNC_OBJ,imrecv)
#define MPID_Init                     CALL_F(FUNC_OBJ,init)
#define MPID_InitCompleted            CALL_F(FUNC_OBJ,initcompleted)
#define MPID_Iprobe                   CALL_F(FUNC_OBJ,iprobe)
#define MPID_Irecv                    CALL_F(FUNC_OBJ,irecv)
#define MPID_Irsend                   CALL_F(FUNC_OBJ,irsend)
#define MPID_Isend                    CALL_F(FUNC_OBJ,isend)
#define MPID_Issend                   CALL_F(FUNC_OBJ,issend)
#define MPID_Mprobe                   CALL_F(FUNC_OBJ,mprobe)
#define MPID_Mrecv                    CALL_F(FUNC_OBJ,mrecv)
#define MPID_Open_port                CALL_F(FUNC_OBJ,open_port)
#define MPID_Probe                    CALL_F(FUNC_OBJ,probe)
#define MPID_Progress_end             CALL_F(FUNC_OBJ,progress_end)
#define MPID_Progress_poke            CALL_F(FUNC_OBJ,progress_poke)
#define MPID_Progress_start           CALL_F(FUNC_OBJ,progress_start)
#define MPID_Progress_test            CALL_F(FUNC_OBJ,progress_test)
#define MPID_Progress_wait            CALL_F(FUNC_OBJ,progress_wait)
#define MPID_Progress_register        CALL_F(FUNC_OBJ,progress_register)
#define MPID_Progress_deregister      CALL_F(FUNC_OBJ,progress_deregister)
#define MPID_Progress_activate        CALL_F(FUNC_OBJ,progress_activate)
#define MPID_Progress_deactivate      CALL_F(FUNC_OBJ,progress_deactivate)
#define MPID_Put                      CALL_F(FUNC_OBJ,put)
#define MPID_Raccumulate              CALL_F(FUNC_OBJ,raccumulate)
#define MPID_Recv                     CALL_F(FUNC_OBJ,recv)
#define MPID_Recv_init                CALL_F(FUNC_OBJ,recv_init)
#define MPID_Request_init             CALL_F(FUNC_OBJ,request_init)
#define MPID_Request_finalize         CALL_F(FUNC_OBJ,request_finalize)
#define MPID_Request_create_hook      CALL_F(FUNC_OBJ,request_create_hook)
#define MPID_Request_free_hook        CALL_F(FUNC_OBJ,request_free_hook)
#define MPID_Request_destroy_hook     CALL_F(FUNC_OBJ,request_destroy_hook)
#define MPID_Request_complete         CALL_F(FUNC_OBJ,request_complete)
#define MPID_Request_is_anysource     CALL_F(FUNC_OBJ,request_is_anysource)
#define MPID_Request_set_completed    CALL_F(FUNC_OBJ,request_set_completed)
#define MPID_Rget                     CALL_F(FUNC_OBJ,rget)
#define MPID_Rget_accumulate          CALL_F(FUNC_OBJ,rget_accumulate)
#define MPID_Rput                     CALL_F(FUNC_OBJ,rput)
#define MPID_Rsend                    CALL_F(FUNC_OBJ,rsend)
#define MPID_Rsend_init               CALL_F(FUNC_OBJ,rsend_init)
#define MPID_Send                     CALL_F(FUNC_OBJ,send)
#define MPID_Send_init                CALL_F(FUNC_OBJ,send_init)
#define MPID_Ssend                    CALL_F(FUNC_OBJ,ssend)
#define MPID_Ssend_init               CALL_F(FUNC_OBJ,ssend_init)
#define MPID_Startall                 CALL_F(FUNC_OBJ,startall)
#define MPID_Win_allocate             CALL_F(FUNC_OBJ,win_allocate)
#define MPID_Win_allocate_shared      CALL_F(FUNC_OBJ,win_allocate_shared)
#define MPID_Win_attach               CALL_F(FUNC_OBJ,win_attach)
#define MPID_Win_complete             CALL_F(FUNC_OBJ,win_complete)
#define MPID_Win_create               CALL_F(FUNC_OBJ,win_create)
#define MPID_Win_create_dynamic       CALL_F(FUNC_OBJ,win_create_dynamic)
#define MPID_Win_detach               CALL_F(FUNC_OBJ,win_detach)
#define MPID_Win_fence                CALL_F(FUNC_OBJ,win_fence)
#define MPID_Win_flush                CALL_F(FUNC_OBJ,win_flush)
#define MPID_Win_flush_all            CALL_F(FUNC_OBJ,win_flush_all)
#define MPID_Win_flush_local          CALL_F(FUNC_OBJ,win_flush_local)
#define MPID_Win_flush_local_all      CALL_F(FUNC_OBJ,win_flush_local_all)
#define MPID_Win_free                 CALL_F(FUNC_OBJ,win_free)
#define MPID_Win_get_info             CALL_F(FUNC_OBJ,win_get_info)
#define MPID_Win_lock                 CALL_F(FUNC_OBJ,win_lock)
#define MPID_Win_lock_all             CALL_F(FUNC_OBJ,win_lock_all)
#define MPID_Win_post                 CALL_F(FUNC_OBJ,win_post)
#define MPID_Win_set_info             CALL_F(FUNC_OBJ,win_set_info)
#define MPID_Win_shared_query         CALL_F(FUNC_OBJ,win_shared_query)
#define MPID_Win_start                CALL_F(FUNC_OBJ,win_start)
#define MPID_Win_sync                 CALL_F(FUNC_OBJ,win_sync)
#define MPID_Win_test                 CALL_F(FUNC_OBJ,win_test)
#define MPID_Win_unlock               CALL_F(FUNC_OBJ,win_unlock)
#define MPID_Win_unlock_all           CALL_F(FUNC_OBJ,win_unlock_all)
#define MPID_Win_wait                 CALL_F(FUNC_OBJ,win_wait)
#define MPID_Get_node_id              CALL_F(FUNC_OBJ,get_node_id)
#define MPID_Get_max_node_id          CALL_F(FUNC_OBJ,get_max_node_id)
#define MPID_Request_is_pending_failure CALL_F(FUNC_OBJ,request_is_pending_failure)
#define MPID_Aint_add                 CALL_F(FUNC_OBJ,aint_add)
#define MPID_Aint_diff                CALL_F(FUNC_OBJ,aint_diff)
#define MPID_Create_intercomm_from_lpids CALL_F(FUNC_OBJ,create_intercomm_from_lpids)
#define MPID_Comm_create_hook         CALL_F(FUNC_OBJ,comm_create_hook)
#define MPID_Comm_free_hook           CALL_F(FUNC_OBJ,comm_free_hook)
#define MPID_Intercomm_exchange_map   CALL_F(FUNC_OBJ,intercomm_exchange_map)

#define MPID_Barrier                  CALL_F(FUNC_OBJ,barrier)
#define MPID_Bcast                    CALL_F(FUNC_OBJ,bcast)
#define MPID_Allreduce                CALL_F(FUNC_OBJ,allreduce)
#define MPID_Allgather                CALL_F(FUNC_OBJ,allgather)
#define MPID_Allgatherv               CALL_F(FUNC_OBJ,allgatherv)
#define MPID_Scatter                  CALL_F(FUNC_OBJ,scatter)
#define MPID_Scatterv                 CALL_F(FUNC_OBJ,scatterv)
#define MPID_Gather                   CALL_F(FUNC_OBJ,gather)
#define MPID_Gatherv                  CALL_F(FUNC_OBJ,gatherv)
#define MPID_Alltoall                 CALL_F(FUNC_OBJ,alltoall)
#define MPID_Alltoallv                CALL_F(FUNC_OBJ,alltoallv)
#define MPID_Alltoallw                CALL_F(FUNC_OBJ,alltoallw)
#define MPID_Reduce                   CALL_F(FUNC_OBJ,reduce)
#define MPID_Reduce_scatter           CALL_F(FUNC_OBJ,reduce_scatter)
#define MPID_Reduce_scatter_block     CALL_F(FUNC_OBJ,reduce_scatter_block)
#define MPID_Scan                     CALL_F(FUNC_OBJ,scan)
#define MPID_Exscan                   CALL_F(FUNC_OBJ,exscan)
#define MPID_Neighbor_allgather       CALL_F(FUNC_OBJ,neighbor_allgather)
#define MPID_Neighbor_allgatherv      CALL_F(FUNC_OBJ,neighbor_allgatherv)
#define MPID_Neighbor_alltoallv       CALL_F(FUNC_OBJ,neighbor_alltoallv)
#define MPID_Neighbor_alltoallw       CALL_F(FUNC_OBJ,neighbor_alltoallw)
#define MPID_Neighbor_alltoall        CALL_F(FUNC_OBJ,neighbor_alltoall)
#define MPID_Ineighbor_allgather      CALL_F(FUNC_OBJ,ineighbor_allgather)
#define MPID_Ineighbor_allgatherv     CALL_F(FUNC_OBJ,ineighbor_allgatherv)
#define MPID_Ineighbor_alltoall       CALL_F(FUNC_OBJ,ineighbor_alltoall)
#define MPID_Ineighbor_alltoallv      CALL_F(FUNC_OBJ,ineighbor_alltoallv)
#define MPID_Ineighbor_alltoallw      CALL_F(FUNC_OBJ,ineighbor_alltoallw)
#define MPID_Ibarrier                 CALL_F(FUNC_OBJ,ibarrier)
#define MPID_Ibcast                   CALL_F(FUNC_OBJ,ibcast)
#define MPID_Iallgather               CALL_F(FUNC_OBJ,iallgather)
#define MPID_Iallgatherv              CALL_F(FUNC_OBJ,iallgatherv)
#define MPID_Iallreduce               CALL_F(FUNC_OBJ,iallreduce)
#define MPID_Ialltoall                CALL_F(FUNC_OBJ,ialltoall)
#define MPID_Ialltoallv               CALL_F(FUNC_OBJ,ialltoallv)
#define MPID_Ialltoallw               CALL_F(FUNC_OBJ,ialltoallw)
#define MPID_Iexscan                  CALL_F(FUNC_OBJ,iexscan)
#define MPID_Igather                  CALL_F(FUNC_OBJ,igather)
#define MPID_Igatherv                 CALL_F(FUNC_OBJ,igatherv)
#define MPID_Ireduce_scatter_block    CALL_F(FUNC_OBJ,ireduce_scatter_block)
#define MPID_Ireduce_scatter          CALL_F(FUNC_OBJ,ireduce_scatter)
#define MPID_Ireduce                  CALL_F(FUNC_OBJ,ireduce)
#define MPID_Iscan                    CALL_F(FUNC_OBJ,iscan)
#define MPID_Iscatter                 CALL_F(FUNC_OBJ,iscatter)
#define MPID_Iscatterv                CALL_F(FUNC_OBJ,iscatterv)

/* Variables */
#define MPID_Request_direct           _g_adi.request_direct
#define MPID_Request_mem             (*_g_adi.request_mem)




#else /*  Non-dynamic API */

#include "./mpid_inliner.h"
#define MPID_Abort                    MPIDI_Abort
#define MPID_Accumulate               MPIDI_Accumulate
#define MPID_Alloc_mem                MPIDI_Alloc_mem
#define MPID_Bsend_init               MPIDI_Bsend_init
#define MPID_Cancel_recv              MPIDI_Cancel_recv
#define MPID_Cancel_send              MPIDI_Cancel_send
#define MPID_Close_port               MPIDI_Close_port
#define MPID_Comm_accept              MPIDI_Comm_accept
#define MPID_Comm_connect             MPIDI_Comm_connect
#define MPID_Comm_disconnect          MPIDI_Comm_disconnect
#define MPID_Comm_group_failed        MPIDI_Comm_group_failed
#define MPID_Comm_reenable_anysource  MPIDI_Comm_reenable_anysource
#define MPID_Comm_remote_group_failed MPIDI_Comm_remote_group_failed
#define MPID_Comm_spawn_multiple      MPIDI_Comm_spawn_multiple
#define MPID_Comm_failure_get_acked   MPIDI_Comm_failure_get_acked
#define MPID_Comm_get_all_failed_procs MPIDI_Comm_get_all_failed_procs
#define MPID_Comm_revoke              MPIDI_Comm_revoke
#define MPID_Comm_failure_ack         MPIDI_Comm_failure_ack
#define MPID_Comm_AS_enabled          MPIDI_Comm_AS_enabled
#define MPID_Comm_get_lpid            MPIDI_Comm_get_lpid
#define MPID_Compare_and_swap         MPIDI_Compare_and_swap
#define MPID_Fetch_and_op             MPIDI_Fetch_and_op
#define MPID_Finalize                 MPIDI_Finalize
#define MPID_Free_mem                 MPIDI_Free_mem
#define MPID_Get                      MPIDI_Get
#define MPID_Get_accumulate           MPIDI_Get_accumulate
#define MPID_Get_processor_name       MPIDI_Get_processor_name
#define MPID_Get_universe_size        MPIDI_Get_universe_size
#define MPID_Improbe                  MPIDI_Improbe
#define MPID_Imrecv                   MPIDI_Imrecv
#define MPID_Init                     MPIDI_Init
#define MPID_InitCompleted            MPIDI_InitCompleted
#define MPID_Iprobe                   MPIDI_Iprobe
#define MPID_Irecv                    MPIDI_Irecv
#define MPID_Irsend                   MPIDI_Irsend
#define MPID_Isend                    MPIDI_Isend
#define MPID_Issend                   MPIDI_Issend
#define MPID_Mprobe                   MPIDI_Mprobe
#define MPID_Mrecv                    MPIDI_Mrecv
#define MPID_Open_port                MPIDI_Open_port
#define MPID_Probe                    MPIDI_Probe
#define MPID_Progress_end             MPIDI_Progress_end
#define MPID_Progress_poke            MPIDI_Progress_poke
#define MPID_Progress_start           MPIDI_Progress_start
#define MPID_Progress_test            MPIDI_Progress_test
#define MPID_Progress_wait            MPIDI_Progress_wait
#define MPID_Progress_register        MPIDI_Progress_register
#define MPID_Progress_deregister      MPIDI_Progress_deregister
#define MPID_Progress_activate        MPIDI_Progress_activate
#define MPID_Progress_deactivate      MPIDI_Progress_deactivate
#define MPID_Put                      MPIDI_Put
#define MPID_Raccumulate              MPIDI_Raccumulate
#define MPID_Recv                     MPIDI_Recv
#define MPID_Recv_init                MPIDI_Recv_init
#define MPID_Request_init             MPIDI_Request_init
#define MPID_Request_finalize         MPIDI_Request_finalize
#define MPID_Request_create_hook      MPIDI_Request_create_hook
#define MPID_Request_free_hook        MPIDI_Request_free_hook
#define MPID_Request_destroy_hook     MPIDI_Request_destroy_hook
#define MPID_Request_complete         MPIDI_Request_complete
#define MPID_Request_is_anysource     MPIDI_Request_is_anysource
#define MPID_Request_set_completed    MPIDI_Request_set_completed
#define MPID_Rget                     MPIDI_Rget
#define MPID_Rget_accumulate          MPIDI_Rget_accumulate
#define MPID_Rput                     MPIDI_Rput
#define MPID_Rsend                    MPIDI_Rsend
#define MPID_Rsend_init               MPIDI_Rsend_init
#define MPID_Send                     MPIDI_Send
#define MPID_Send_init                MPIDI_Send_init
#define MPID_Ssend                    MPIDI_Ssend
#define MPID_Ssend_init               MPIDI_Ssend_init
#define MPID_Startall                 MPIDI_Startall
#define MPID_Win_allocate             MPIDI_Win_allocate
#define MPID_Win_allocate_shared      MPIDI_Win_allocate_shared
#define MPID_Win_attach               MPIDI_Win_attach
#define MPID_Win_complete             MPIDI_Win_complete
#define MPID_Win_create               MPIDI_Win_create
#define MPID_Win_create_dynamic       MPIDI_Win_create_dynamic
#define MPID_Win_detach               MPIDI_Win_detach
#define MPID_Win_fence                MPIDI_Win_fence
#define MPID_Win_flush                MPIDI_Win_flush
#define MPID_Win_flush_all            MPIDI_Win_flush_all
#define MPID_Win_flush_local          MPIDI_Win_flush_local
#define MPID_Win_flush_local_all      MPIDI_Win_flush_local_all
#define MPID_Win_free                 MPIDI_Win_free
#define MPID_Win_get_info             MPIDI_Win_get_info
#define MPID_Win_lock                 MPIDI_Win_lock
#define MPID_Win_lock_all             MPIDI_Win_lock_all
#define MPID_Win_post                 MPIDI_Win_post
#define MPID_Win_set_info             MPIDI_Win_set_info
#define MPID_Win_shared_query         MPIDI_Win_shared_query
#define MPID_Win_start                MPIDI_Win_start
#define MPID_Win_sync                 MPIDI_Win_sync
#define MPID_Win_test                 MPIDI_Win_test
#define MPID_Win_unlock               MPIDI_Win_unlock
#define MPID_Win_unlock_all           MPIDI_Win_unlock_all
#define MPID_Win_wait                 MPIDI_Win_wait
#define MPID_Get_node_id              MPIDI_Get_node_id
#define MPID_Get_max_node_id          MPIDI_Get_max_node_id
#define MPID_Request_is_pending_failure MPIDI_Request_is_pending_failure
#define MPID_Aint_add                 MPIDI_Aint_add
#define MPID_Aint_diff                MPIDI_Aint_diff
#define MPID_Create_intercomm_from_lpids MPIDI_Create_intercomm_from_lpids
#define MPID_Comm_create_hook         MPIDI_Comm_create_hook
#define MPID_Comm_free_hook           MPIDI_Comm_free_hook
#define MPID_Intercomm_exchange_map   MPIDI_Intercomm_exchange_map

/* Collectives */
#define MPID_Barrier                          MPIDI_Barrier
#define MPID_Bcast                            MPIDI_Bcast
#define MPID_Allreduce                        MPIDI_Allreduce
#define MPID_Allgather                        MPIDI_Allgather
#define MPID_Allgatherv                       MPIDI_Allgatherv
#define MPID_Scatter                          MPIDI_Scatter
#define MPID_Scatterv                         MPIDI_Scatterv
#define MPID_Gather                           MPIDI_Gather
#define MPID_Gatherv                          MPIDI_Gatherv
#define MPID_Alltoall                         MPIDI_Alltoall
#define MPID_Alltoallv                        MPIDI_Alltoallv
#define MPID_Alltoallw                        MPIDI_Alltoallw
#define MPID_Reduce                           MPIDI_Reduce
#define MPID_Reduce_scatter                   MPIDI_Reduce_scatter
#define MPID_Reduce_scatter_block             MPIDI_Reduce_scatter_block
#define MPID_Scan                             MPIDI_Scan
#define MPID_Exscan                           MPIDI_Exscan
#define MPID_Neighbor_allgather               MPIDI_Neighbor_allgather
#define MPID_Neighbor_allgatherv              MPIDI_Neighbor_allgatherv
#define MPID_Neighbor_alltoallv               MPIDI_Neighbor_alltoallv
#define MPID_Neighbor_alltoallw               MPIDI_Neighbor_alltoallw
#define MPID_Neighbor_alltoall                MPIDI_Neighbor_alltoall
#define MPID_Ineighbor_allgather              MPIDI_Ineighbor_allgather
#define MPID_Ineighbor_allgatherv             MPIDI_Ineighbor_allgatherv
#define MPID_Ineighbor_alltoall               MPIDI_Ineighbor_alltoall
#define MPID_Ineighbor_alltoallv              MPIDI_Ineighbor_alltoallv
#define MPID_Ineighbor_alltoallw              MPIDI_Ineighbor_alltoallw
#define MPID_Ibarrier                         MPIDI_Ibarrier
#define MPID_Ibcast                           MPIDI_Ibcast
#define MPID_Iallgather                       MPIDI_Iallgather
#define MPID_Iallgatherv                      MPIDI_Iallgatherv
#define MPID_Iallreduce                       MPIDI_Iallreduce
#define MPID_Ialltoall                        MPIDI_Ialltoall
#define MPID_Ialltoallv                       MPIDI_Ialltoallv
#define MPID_Ialltoallw                       MPIDI_Ialltoallw
#define MPID_Iexscan                          MPIDI_Iexscan
#define MPID_Igather                          MPIDI_Igather
#define MPID_Igatherv                         MPIDI_Igatherv
#define MPID_Ireduce_scatter_block            MPIDI_Ireduce_scatter_block
#define MPID_Ireduce_scatter                  MPIDI_Ireduce_scatter
#define MPID_Ireduce                          MPIDI_Ireduce
#define MPID_Iscan                            MPIDI_Iscan
#define MPID_Iscatter                         MPIDI_Iscatter
#define MPID_Iscatterv                        MPIDI_Iscatterv

/* Variables */
#define MPID_Request_direct           MPIDI_Request_direct
#define MPID_Request_mem              MPIDI_Request_mem


#endif /* USE_DYNAMIC_API */

#endif /* !defined(MPICH_MPIDFUNC_H_INCLUDED) */
