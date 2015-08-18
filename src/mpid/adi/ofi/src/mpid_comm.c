/* -*- Mode: C; c-basic-offset:2 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#include <pmi.h>
#include <mpidimpl.h>
#include "mpid_types.h"
#include "mpl_utlist.h"
/*#include "coll/transports/mpich/transport.h"*/

__ADI_INLINE__ int MPIDI_Comm_AS_enabled(MPIR_Comm *comm)
{
  assert(0);
  return MPI_SUCCESS;
}
__ADI_INLINE__ int MPIDI_Comm_reenable_anysource(MPIR_Comm   *comm,
                                                 MPIR_Group **failed_group_ptr)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Comm_remote_group_failed(MPIR_Comm   *comm,
                                                  MPIR_Group **failed_group_ptr)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Comm_group_failed(MPIR_Comm   *comm_ptr,
                                           MPIR_Group **failed_group_ptr)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Comm_failure_ack(MPIR_Comm *comm_ptr)
{
  assert(0);
  return 0;
}

__ADI_INLINE__ int MPIDI_Comm_failure_get_acked(MPIR_Comm *comm_ptr, MPIR_Group **failed_group_ptr)
{
  assert(0);
  return 0;
}

__ADI_INLINE__ int MPIDI_Comm_revoke(MPIR_Comm *comm_ptr, int is_remote)
{
  assert(0);
  return 0;
}

__ADI_INLINE__ int MPIDI_Comm_get_all_failed_procs(MPIR_Comm *comm_ptr, MPIR_Group **failed_group, int tag)
{
  assert(0);
  return 0;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_split_type)
__SI__ int MPIDI_Comm_split_type(MPIR_Comm  *comm_ptr,
                                 int         split_type,
                                 int         key,
                                 MPIR_Info  *info_ptr,
                                 MPIR_Comm **newcomm_ptr)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  if(split_type == MPI_COMM_TYPE_SHARED) {
    MPIDI_VCR      idx     = COMM_OFI(comm_ptr)->vcrt->vcr_table[comm_ptr->rank];
    int            node_id = MPIDI_Global.node_map[idx.addr_idx];
    mpi_errno              = MPIR_Comm_split_impl(comm_ptr,node_id,key,newcomm_ptr);
  } else
    mpi_errno              = MPIR_Comm_split_impl(comm_ptr, MPI_UNDEFINED, key, newcomm_ptr);

  END_FUNC(FCNAME);
  return mpi_errno;
}

static void init_comm()__attribute__((constructor));
static void init_comm()
{
  MPIR_Comm_fns             = &MPIDI_Global.MPIR_Comm_fns_store;
  MPIR_Comm_fns->split_type =  MPIDI_Comm_split_type;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_VCR_Dup)
__SI__ int MPIDI_VCR_Dup(MPIDI_VCR orig_vcr, MPIDI_VCR *new_vcr)
{
  BEGIN_FUNC(FCNAME);
  *new_vcr = orig_vcr;
  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_VCRT_Add_ref)
__SI__ int MPIDI_VCRT_Add_ref(struct MPIDI_VCRT *vcrt)
{
  BEGIN_FUNC(FCNAME);
  MPIR_Object_add_ref(vcrt);
  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(dup_vcrt)
__SI__ void dup_vcrt(struct MPIDI_VCRT *src_vcrt,
                     struct MPIDI_VCRT **dest_vcrt,
                     MPIR_Comm_map_t    *mapper,
                     int                 src_comm_size,
                     int                 vcrt_size,
                     int                 vcrt_offset)
{
  BEGIN_FUNC(FCNAME);
  int flag, i;

  /* try to find the simple case where the new comm is a simple
   * duplicate of the previous comm.  in that case, we simply add a
   * reference to the previous VCRT instead of recreating it. */
  if (mapper->type == MPIR_COMM_MAP_TYPE__DUP && src_comm_size == vcrt_size) {
    *dest_vcrt = src_vcrt;
    MPIDI_VCRT_Add_ref(src_vcrt);
    goto fn_exit;
  }
  else if (mapper->type == MPIR_COMM_MAP_TYPE__IRREGULAR &&
           mapper->src_mapping_size == vcrt_size) {
    /* if the mapping array is exactly the same as the original
     * comm's VC list, there is no need to create a new VCRT.
     * instead simply point to the original comm's VCRT and bump
     * up it's reference count */
    flag = 1;
    for (i = 0; i < mapper->src_mapping_size; i++)
      if (mapper->src_mapping[i] != i)
        flag = 0;

    if (flag) {
      *dest_vcrt = src_vcrt;
      MPIDI_VCRT_Add_ref(src_vcrt);
      goto fn_exit;
    }
  }

  /* we are in the more complex case where we need to allocate a new
   * VCRT */

  if (!vcrt_offset)
    MPIDI_VCRT_Create(vcrt_size, dest_vcrt);

  if (mapper->type == MPIR_COMM_MAP_TYPE__DUP) {
    for (i = 0; i < src_comm_size; i++)
      MPIDI_VCR_Dup(src_vcrt->vcr_table[i],
                    &((*dest_vcrt)->vcr_table[i + vcrt_offset]));
  }
  else {
    for (i = 0; i < mapper->src_mapping_size; i++)
      MPIDI_VCR_Dup(src_vcrt->vcr_table[mapper->src_mapping[i]],
                    &((*dest_vcrt)->vcr_table[i + vcrt_offset]));
  }
  if (mapper->type == MPIR_COMM_MAP_TYPE__ENDPOINT) {
    for (i = 0; i < mapper->src_mapping_size;i++)
      (*dest_vcrt)->vcr_table[i].ep_idx  = mapper->ep_mapping[i];
  }

fn_exit:
  END_FUNC(FCNAME);
  return;
}

__SI__ int map_size(MPIR_Comm_map_t map)
{
  if (map.type == MPIR_COMM_MAP_TYPE__IRREGULAR || map.type == MPIR_COMM_MAP_TYPE__ENDPOINT)
    return map.src_mapping_size;
  else if (map.dir == MPIR_COMM_MAP_DIR__L2L || map.dir == MPIR_COMM_MAP_DIR__L2R)
    return map.src_comm->local_size;
  else
    return map.src_comm->remote_size;
}

#undef FCNAME
#define FCNAME DECL_FUNC(alloc_tables)
static inline int alloc_tables(MPIR_Comm *comm)
{
  int mpi_errno = MPI_SUCCESS;
  MPIR_Comm_map_t *mapper;
  MPIR_Comm *src_comm;
  int vcrt_size, vcrt_offset;

  BEGIN_FUNC(FCNAME);
  /* do some sanity checks */
  MPL_LL_FOREACH(comm->mapper_head, mapper) {
    if (mapper->src_comm->comm_kind == MPIR_COMM_KIND__INTRACOMM)
      MPIR_Assert(mapper->dir == MPIR_COMM_MAP_DIR__L2L ||
                  mapper->dir == MPIR_COMM_MAP_DIR__L2R);
    if (comm->comm_kind == MPIR_COMM_KIND__INTRACOMM)
      MPIR_Assert(mapper->dir == MPIR_COMM_MAP_DIR__L2L ||
                  mapper->dir == MPIR_COMM_MAP_DIR__R2L);
  }

  /* First, handle all the mappers that contribute to the local part
   * of the comm */
  vcrt_size = 0;
  MPL_LL_FOREACH(comm->mapper_head, mapper) {
    if (mapper->dir == MPIR_COMM_MAP_DIR__L2R ||
        mapper->dir == MPIR_COMM_MAP_DIR__R2R)
      continue;

    vcrt_size += map_size(*mapper);
  }
  vcrt_offset = 0;
  MPL_LL_FOREACH(comm->mapper_head, mapper) {
    src_comm = mapper->src_comm;

    if (mapper->dir == MPIR_COMM_MAP_DIR__L2R ||
        mapper->dir == MPIR_COMM_MAP_DIR__R2R)
      continue;

    if (mapper->dir == MPIR_COMM_MAP_DIR__L2L) {
      if (src_comm->comm_kind == MPIR_COMM_KIND__INTRACOMM && comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper, mapper->src_comm->local_size,
                 vcrt_size, vcrt_offset);
      }
      else if (src_comm->comm_kind == MPIR_COMM_KIND__INTRACOMM && comm->comm_kind == MPIR_COMM_KIND__INTERCOMM)
        dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->local_vcrt, mapper, mapper->src_comm->local_size,
                 vcrt_size, vcrt_offset);
      else if (src_comm->comm_kind == MPIR_COMM_KIND__INTERCOMM && comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        dup_vcrt(COMM_OFI(src_comm)->local_vcrt, &COMM_OFI(comm)->vcrt, mapper, mapper->src_comm->local_size,
                 vcrt_size, vcrt_offset);
      }
      else
        dup_vcrt(COMM_OFI(src_comm)->local_vcrt, &COMM_OFI(comm)->local_vcrt, mapper,
                 mapper->src_comm->local_size, vcrt_size, vcrt_offset);
    }
    else {  /* mapper->dir == MPIR_COMM_MAP_DIR__R2L */
      MPIR_Assert(src_comm->comm_kind == MPIR_COMM_KIND__INTERCOMM);
      if (comm->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper, mapper->src_comm->remote_size,
                 vcrt_size, vcrt_offset);
      }
      else
        dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->local_vcrt, mapper, mapper->src_comm->remote_size,
                 vcrt_size, vcrt_offset);
    }
    vcrt_offset += map_size(*mapper);
  }

  /* Next, handle all the mappers that contribute to the remote part
   * of the comm (only valid for intercomms) */
  vcrt_size = 0;
  MPL_LL_FOREACH(comm->mapper_head, mapper) {
    if (mapper->dir == MPIR_COMM_MAP_DIR__L2L ||
        mapper->dir == MPIR_COMM_MAP_DIR__R2L)
      continue;

    vcrt_size += map_size(*mapper);
  }
  vcrt_offset = 0;
  MPL_LL_FOREACH(comm->mapper_head, mapper) {
    src_comm = mapper->src_comm;

    if (mapper->dir == MPIR_COMM_MAP_DIR__L2L ||
        mapper->dir == MPIR_COMM_MAP_DIR__R2L)
      continue;

    MPIR_Assert(comm->comm_kind == MPIR_COMM_KIND__INTERCOMM);

    if (mapper->dir == MPIR_COMM_MAP_DIR__L2R) {
      if (src_comm->comm_kind == MPIR_COMM_KIND__INTRACOMM)
        dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper, mapper->src_comm->local_size,
                 vcrt_size, vcrt_offset);
      else
        dup_vcrt(COMM_OFI(src_comm)->local_vcrt, &COMM_OFI(comm)->vcrt, mapper,
                 mapper->src_comm->local_size, vcrt_size, vcrt_offset);
    }
    else {  /* mapper->dir == MPIR_COMM_MAP_DIR__R2R */
      MPIR_Assert(src_comm->comm_kind == MPIR_COMM_KIND__INTERCOMM);
      dup_vcrt(COMM_OFI(src_comm)->vcrt, &COMM_OFI(comm)->vcrt, mapper, mapper->src_comm->remote_size,
               vcrt_size, vcrt_offset);
    }
    vcrt_offset += map_size(*mapper);
  }

  if (comm->comm_kind == MPIR_COMM_KIND__INTERCOMM) {
    /* setup the vcrt for the local_comm in the intercomm */
    if (comm->local_comm) {
      COMM_OFI(comm->local_comm)->vcrt = COMM_OFI(comm)->local_vcrt;
      MPIDI_VCRT_Add_ref(COMM_OFI(comm)->local_vcrt);
    }
  }

  END_FUNC(FCNAME);
  return mpi_errno;
}



#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_create_hook)
__ADI_INLINE__ int MPIDI_Comm_create_hook(MPIR_Comm *comm)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  uint64_t mapid;

  alloc_tables(comm);

  mapid = (((uint64_t)COMM_TO_EP(comm,comm->rank))<<32) | comm->context_id;
  MPIDI_Map_set(MPIDI_Global.comm_map,mapid,comm);
  MPIDI_Map_create(&COMM_OFI(comm)->huge_send_counters);
  MPIDI_Map_create(&COMM_OFI(comm)->huge_recv_counters);
  COMM_OFI(comm)->window_instance=0;

  /* Do not handle intercomms */
  if(comm->comm_kind == MPIR_COMM_KIND__INTERCOMM)
    goto fn_exit;

  MPIR_Assert(comm->coll_fns != NULL);

  COLL_Comm_init((COLL_COMM)comm);
#if 0
#define COLLCAST(x,y) ((typeof(comm->coll_fns->x))COLL_##y)
  comm->coll_fns->Barrier                   = COLLCAST(Barrier,Barrier);
  comm->coll_fns->Bcast                     = COLLCAST(Bcast,Bcast);
  comm->coll_fns->Allreduce                 = COLLCAST(Allreduce,Allreduce);
  comm->coll_fns->Reduce                    = COLLCAST(Reduce,Reduce);
  comm->coll_fns->Ibarrier_req              = COLLCAST(Ibarrier_req,Ibarrier);
  comm->coll_fns->Ireduce_req               = COLLCAST(Ireduce_req,Ireduce);
  comm->coll_fns->Iallreduce_req            = COLLCAST(Iallreduce_req,Iallreduce);
  comm->coll_fns->Ibcast_req                = COLLCAST(Ibcast_req,Ibcast);
#endif

#if 0
  /* Stock Collective overrides */
  comm->coll_fns->Barrier                   = COLLCAST(Barrier,barrier);
  comm->coll_fns->Bcast                     = COLLCAST(Bcast,bcast);
  comm->coll_fns->Allreduce                 = COLLCAST(Allreduce,allreduce);
  comm->coll_fns->Allgather                 = COLLCAST(Allgather,allgather);
  comm->coll_fns->Allgatherv                = COLLCAST(Allgatherv,allgatherv);
  comm->coll_fns->Scatterv                  = COLLCAST(Scatterv,scatterv);
  comm->coll_fns->Scatter                   = COLLCAST(Scatter,scatter);
  comm->coll_fns->Gather                    = COLLCAST(Gather,gather);
  comm->coll_fns->Alltoallv                 = COLLCAST(Alltoallv,alltoallv);
  comm->coll_fns->Alltoall                  = COLLCAST(Alltoall,alltoall);
  comm->coll_fns->Gatherv                   = COLLCAST(Gatherv,gatherv);
  comm->coll_fns->Reduce                    = COLLCAST(Reduce,reduce);
  comm->coll_fns->Scan                      = COLLCAST(Scan,scan);
  comm->coll_fns->Exscan                    = COLLCAST(Exscan,exscan);

  /* Neighborhood collectives */
  comm->coll_fns->Neighbor_allgather        = NULL; /* MPIR_Neighbor_allgather_default  */
  comm->coll_fns->Neighbor_allgatherv       = NULL; /* MPIR_Neighbor_allgatherv_default */
  comm->coll_fns->Neighbor_alltoall         = NULL; /* MPIR_Neighbor_alltoall_default;  */
  comm->coll_fns->Neighbor_alltoallv        = NULL; /* MPIR_Neighbor_alltoallv_default; */
  comm->coll_fns->Neighbor_alltoallw        = NULL; /* MPIR_Neighbor_alltoallw_default; */

  /* These don't have suitable overrides in MPICH yet */
  comm->coll_fns->Ineighbor_allgather       = NULL; /* MPIR_Ineighbor_allgather_default;  */
  comm->coll_fns->Ineighbor_allgatherv      = NULL; /* MPIR_Ineighbor_allgatherv_default; */
  comm->coll_fns->Ineighbor_alltoall        = NULL; /* MPIR_Ineighbor_alltoall_default;   */
  comm->coll_fns->Ineighbor_alltoallv       = NULL; /* MPIR_Ineighbor_alltoallv_default;  */
  comm->coll_fns->Ineighbor_alltoallw       = NULL; /* MPIR_Ineighbor_alltoallw_default;  */

  /* These are implemented by default using the _sched variants */
  comm->coll_fns->Ibarrier_req              = NULL;
  comm->coll_fns->Ibcast_req                = NULL;
  comm->coll_fns->Iallgather_req            = NULL;
  comm->coll_fns->Iallgatherv_req           = NULL;
  comm->coll_fns->Iallreduce_req            = NULL;
  comm->coll_fns->Ialltoall_req             = NULL;
  comm->coll_fns->Ialltoallv_req            = NULL;
  comm->coll_fns->Ialltoallw_req            = NULL;
  comm->coll_fns->Iexscan_req               = NULL;
  comm->coll_fns->Igather_req               = NULL;
  comm->coll_fns->Igatherv_req              = NULL;
  comm->coll_fns->Ireduce_scatter_block_req = NULL;
  comm->coll_fns->Ireduce_scatter_req       = NULL;
  comm->coll_fns->Ireduce_req               = NULL;
  comm->coll_fns->Iscan_req                 = NULL;
  comm->coll_fns->Iscatter_req              = NULL;
  comm->coll_fns->Iscatterv_req             = NULL;
#endif
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_free_hook)
__ADI_INLINE__ int MPIDI_Comm_free_hook(MPIR_Comm *comm)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  MPIDI_Map_erase(MPIDI_Global.comm_map,comm->context_id);
  MPIDI_Map_destroy(COMM_OFI(comm)->huge_send_counters);
  MPIDI_Map_destroy(COMM_OFI(comm)->huge_recv_counters);

  mpi_errno = MPIDI_VCRT_Release(COMM_OFI(comm)->vcrt);
  if (mpi_errno) MPIR_ERR_POP(mpi_errno);

  if (comm->comm_kind == MPIR_COMM_KIND__INTERCOMM) {
    mpi_errno = MPIDI_VCRT_Release(COMM_OFI(comm)->local_vcrt);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
  }
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Intercomm_exchange_map)
__ADI_INLINE__ int MPIDI_Intercomm_exchange_map(MPIR_Comm  *local_comm,
                                                int         local_leader,
                                                MPIR_Comm  *peer_comm,
                                                int         remote_leader,
                                                int        *remote_size,
                                                int       **remote_lupids,
                                                int        *is_low_group)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);
  assert(0);
  END_FUNC(FCNAME);
  return mpi_errno;
}
