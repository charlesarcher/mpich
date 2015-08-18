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
#include "../../../mpid_types.h"
#include "../../coll.h"

#define DTCAST(x) ((MPI_Datatype)(uint64_t) x)
#define DTSCAST(x) ((MPI_Datatype*) x)
#define OPCAST(x) ((MPI_Op)(uint64_t) x)
#define COMMCAST(x) ((MPIR_Comm*) x)

#define CALL_THROUGH(rc,wrapper_name,target_name,ARGS1,ARGS2)   \
  rc COLLI_##wrapper_name (ARGS1, MPIR_Errflag_t *errflag)      \
  {                                                             \
    return MPIR_##target_name (ARGS2, errflag);                 \
  }

#define ICALL_THROUGH(rc,wrapper_name,target_name,ARGS1,ARGS2)  \
  rc COLLI_##wrapper_name (ARGS1,COLL_REQ *request)              \
  {                                                             \
    MPIR_Assert(0);                                             \
    return 0;                                                   \
    /*return MPIR_##target_name (ARGS2,COLL_REQ *request);*/    \
  }

#define NCALL_THROUGH(rc,wrapper_name,target_name,ARGS1,ARGS2)  \
  rc COLLI_Neighbor_##wrapper_name (ARGS1)                       \
  {                                                             \
    MPIR_Assert(0);                                             \
    return 0;                                                   \
    /*return MPIR_##target_name (ARGS2,COLL_REQ *request);*/    \
  }
#define INCALL_THROUGH(rc,wrapper_name,target_name,ARGS1,ARGS2) \
  rc COLLI_Ineighbor_##wrapper_name (ARGS1,COLL_SCHED s)         \
  {                                                             \
    MPIR_Assert(0);                                             \
    return 0;                                                   \
    /*return MPIR_##target_name (ARGS2,COLL_REQ *request);*/    \
  }

int COLLI_Init(TransportFunc *fcn)
{
  return 0;
}

int COLLI_Comm_init(COLL_COMM comm)
{
  return 0;
}

int COLLI_Kick(void)
{
  return 0;
}

#define BARRIER_CALL    COMMCAST(comm)
#define ALLGATHER_CALL sendbuf,sendcount,DTCAST(sendtype),      \
    recvbuf,recvcount,DTCAST(recvtype),                         \
    COMMCAST(comm)
#define ALLGATHERV_CALL sendbuf,sendcount,DTCAST(sendtype),     \
    recvbuf,recvcounts,displs, DTCAST(recvtype),                \
    COMMCAST(comm)
#define ALLREDUCE_CALL sendbuf,recvbuf,count,DTCAST(datatype),OPCAST(op), \
    COMMCAST(comm)
#define ALLTOALL_CALL sendbuf,sendcount,DTCAST(sendtype),       \
    recvbuf,recvcount,DTCAST(recvtype),                         \
    COMMCAST(comm)
#define ALLTOALLV_CALL sendbuf,sendcnts,sdispls,DTCAST(sendtype),       \
    recvbuf,recvcnts,rdispls,DTCAST(recvtype),                          \
    COMMCAST(comm)
#define ALLTOALLW_CALL sendbuf,sendcnts,sdispls,DTSCAST(sendtypes),     \
    recvbuf,recvcnts,rdispls,DTSCAST(recvtypes),                        \
    COMMCAST(comm)
#define GATHER_CALL sendbuf,sendcnt,DTCAST(sendtype),   \
    recvbuf,recvcnt,DTCAST(recvtype),                   \
    root,COMMCAST(comm)
#define GATHERV_CALL sendbuf,sendcnt,DTCAST(sendtype),  \
    recvbuf,recvcnts,displs,DTCAST(recvtype),           \
    root,COMMCAST(comm)
#define REDUCE_SCATTER_CALL sendbuf,recvbuf,recvcnts,DTCAST(datatype),  \
    OPCAST(op),COMMCAST(comm)
#define REDUCE_SCATTER_BLOCK_CALL sendbuf,recvbuf,recvcount,DTCAST(datatype), \
    OPCAST(op),COMMCAST(comm)
#define REDUCE_CALL sendbuf,recvbuf,count,DTCAST(datatype),     \
    OPCAST(op),root,COMMCAST(comm)
#define SCAN_CALL sendbuf,recvbuf,count,DTCAST(datatype),       \
    OPCAST(op),COMMCAST(comm)
#define SCATTERV_CALL sendbuf,sendcnts,displs,DTCAST(sendtype), \
    recvbuf,recvcnt,DTCAST(recvtype),root,                      \
    COMMCAST(comm)
#define SCATTER_CALL sendbuf,sendcnt,DTCAST(sendtype),recvbuf,recvcnt,  \
    DTCAST(recvtype),root,COMMCAST(comm)

CALL_THROUGH(int,Allgather,Allgather_intra,ALLGATHER_PARAMS,ALLGATHER_CALL);
CALL_THROUGH(int,Allgatherv,Allgatherv_intra,ALLGATHERV_PARAMS,ALLGATHERV_CALL);
CALL_THROUGH(int,Allreduce,Allreduce_intra,ALLREDUCE_PARAMS,ALLREDUCE_CALL);
CALL_THROUGH(int,Alltoall,Alltoall_intra,ALLTOALL_PARAMS,ALLTOALL_CALL);
CALL_THROUGH(int,Alltoallv,Alltoallv_intra,ALLTOALLV_PARAMS,ALLTOALLV_CALL);
CALL_THROUGH(int,Alltoallw,Alltoallw_intra,ALLTOALLW_PARAMS,ALLTOALLW_CALL);
CALL_THROUGH(int,Bcast,Bcast_intra,BCAST_PARAMS,BCAST_CALL);
CALL_THROUGH(int,Exscan,Exscan,EXSCAN_PARAMS,EXSCAN_CALL);
CALL_THROUGH(int,Gather,Gather_intra,GATHER_PARAMS,GATHER_CALL);
CALL_THROUGH(int,Gatherv,Gatherv,GATHERV_PARAMS,GATHERV_CALL);
CALL_THROUGH(int,Reduce_scatter,Reduce_scatter_intra,
             REDUCE_SCATTER_PARAMS,REDUCE_SCATTER_CALL);
CALL_THROUGH(int,Reduce_scatter_block,Reduce_scatter_block_intra,
             REDUCE_SCATTER_BLOCK_PARAMS,REDUCE_SCATTER_BLOCK_CALL);
CALL_THROUGH(int,Reduce,Reduce_intra,REDUCE_PARAMS,REDUCE_CALL);
CALL_THROUGH(int,Scan,Scan,SCAN_PARAMS,SCAN_CALL);
CALL_THROUGH(int,Scatterv,Scatterv,SCATTERV_PARAMS,SCATTERV_CALL);
CALL_THROUGH(int,Scatter,Scatter,SCATTER_PARAMS,SCATTER_CALL);
CALL_THROUGH(int,Barrier,Barrier_intra,BARRIER_PARAMS,BARRIER_CALL);

ICALL_THROUGH(int,Iallgather,Iallgather_sched,ALLGATHER_PARAMS,ALLGATHER_CALL);
ICALL_THROUGH(int,Iallgatherv,Iallgatherv_sched,ALLGATHERV_PARAMS,ALLGATHERV_CALL);
ICALL_THROUGH(int,Iallreduce,Iallreduce_sched,ALLREDUCE_PARAMS,ALLREDUCE_CALL);
ICALL_THROUGH(int,Ialltoall,Ialltoall_sched,ALLTOALL_PARAMS,ALLTOALL_CALL);
ICALL_THROUGH(int,Ialltoallv,Ialltoallv_sched,ALLTOALLV_PARAMS,ALLTOALLV_CALL);
ICALL_THROUGH(int,Ialltoallw,Ialltoallw_sched,ALLTOALLW_PARAMS,ALLTOALLW_CALL);
ICALL_THROUGH(int,Ibcast,Ibcast_sched,BCAST_PARAMS,BCAST_CALL);
ICALL_THROUGH(int,Iexscan,Iexscan_sched,EXSCAN_PARAMS,EXSCAN_CALL);
ICALL_THROUGH(int,Igather,Igather_sched,GATHER_PARAMS,GATHER_CALL);
ICALL_THROUGH(int,Igatherv,Igatherv_sched,GATHERV_PARAMS,GATHERV_CALL);
ICALL_THROUGH(int,Ireduce_scatter,Ireduce_scatter_sched,
              REDUCE_SCATTER_PARAMS,REDUCE_SCATTER_CALL);
ICALL_THROUGH(int,Ireduce_scatter_block,Ireduce_scatter_block_sched,
              REDUCE_SCATTER_BLOCK_PARAMS,REDUCE_SCATTER_BLOCK_CALL);
ICALL_THROUGH(int,Ireduce,Ireduce_sched,REDUCE_PARAMS,REDUCE_CALL);
ICALL_THROUGH(int,Iscan,Iscan_sched,SCAN_PARAMS,SCAN_CALL);
ICALL_THROUGH(int,Iscatterv,Iscatterv_sched,SCATTERV_PARAMS,SCATTERV_CALL);
ICALL_THROUGH(int,Iscatter,Iscatter_sched,SCATTER_PARAMS,SCATTER_CALL);
ICALL_THROUGH(int,Ibarrier,Ibarrier_sched,BARRIER_PARAMS,BARRIER_CALL);

NCALL_THROUGH(int,allgather,allgather,ALLGATHER_PARAMS,ALLGATHER_CALL);
NCALL_THROUGH(int,allgatherv,allgatherv,ALLGATHERV_PARAMS,ALLGATHERV_CALL);
NCALL_THROUGH(int,alltoall,alltoall,ALLTOALL_PARAMS,ALLTOALL_CALL);
NCALL_THROUGH(int,alltoallv,alltoallv,ALLTOALLV_PARAMS,ALLTOALLV_CALL);
NCALL_THROUGH(int,alltoallw,alltoallw,NALLTOALLW_PARAMS,ALLTOALLW_CALL);

INCALL_THROUGH(int,allgather,allgather,ALLGATHER_PARAMS,ALLGATHER_CALL);
INCALL_THROUGH(int,allgatherv,allgatherv,ALLGATHERV_PARAMS,ALLGATHERV_CALL);
INCALL_THROUGH(int,alltoall,alltoall,ALLTOALL_PARAMS,ALLTOALL_CALL);
INCALL_THROUGH(int,alltoallv,alltoallv,ALLTOALLV_PARAMS,ALLTOALLV_CALL);
INCALL_THROUGH(int,alltoallw,alltoallw,NALLTOALLW_PARAMS,ALLTOALLW_CALL);
