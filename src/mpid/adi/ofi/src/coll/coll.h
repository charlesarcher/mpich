/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#if !defined(COLL_H_INCLUDED)
#define COLL_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#define ADD_SUFFIX(x) x##_fn

#ifdef USE_DYNAMIC_COLL

#define __COLL_INLINE__
#define __COLL_SI__  static inline
#define COLL_API(rc,ptrname,fcnname,...)        \
  typedef rc (*ptrname)(__VA_ARGS__);           \
  rc COLL_##fcnname(__VA_ARGS__)

#define __TRANSPORT_SI__  static inline
#define TRANSPORT_API(rc,fcnname,...)        \
  typedef rc (*ADD_SUFFIX(fcnname))(__VA_ARGS__);           \
  rc TRANSPORT_##fcnname(__VA_ARGS__)

#else  /* USE_DYNAMIC_COLL*/

#define __COLL_INLINE__ static inline
#define __COLL_SI__ __attribute__((always_inline)) static inline
#define COLL_API(rc,ptrname,fcnname,...)                \
  typedef rc (*ptrname)(__VA_ARGS__);                   \
  __COLL_INLINE__ rc COLL_##fcnname(__VA_ARGS__)        \
  __attribute__((always_inline))

#define __TRANSPORT_INLINE__ static inline
#define __TRANSPORT_SI__ __attribute__((always_inline)) static inline
#define TRANSPORT_API(rc,fcnname,...)                \
  typedef rc (*ADD_SUFFIX(fcnname))(__VA_ARGS__);                   \
  __TRANSPORT_INLINE__ rc TRANSPORT_##fcnname(__VA_ARGS__)        \
  __attribute__((always_inline))

#endif

/* This macro is to strongly type the collective API */
#define NEW_TYPE(name) struct name##__ { uint64_t dummy; }; \
  typedef struct name##__ *name

NEW_TYPE(COLL_DT);
NEW_TYPE(COLL_OP);
NEW_TYPE(COLL_COMM);
NEW_TYPE(COLL_SCHED);
NEW_TYPE(COLL_REQ);

typedef ssize_t COLL_Aint;
TRANSPORT_API(int,t_pack,COLL_DT dt);
TRANSPORT_API(int,t_unpack,COLL_DT dt);
TRANSPORT_API(int,t_dtinfo,COLL_DT dt,int *is_contig,size_t *extent,size_t *lower_bound);
TRANSPORT_API(int,t_opinfo,COLL_OP op,int *is_commutative);
TRANSPORT_API(int,t_rank,COLL_COMM comm);
TRANSPORT_API(int,t_size,COLL_COMM comm);
TRANSPORT_API(int,t_isinplace,void *buffer);
TRANSPORT_API(int,t_reduce_local,const void *inbuf,void *inoutbuf,int count,COLL_DT datatype,COLL_OP op);
TRANSPORT_API(int,t_dtcopy,void *tobuf,int tocount,COLL_DT totype,const void *frombuf,int fromcount,COLL_DT fromtype);
TRANSPORT_API(int,t_send,const void *buf,int count,COLL_DT datatype,int dest,int tag,COLL_COMM comm,COLL_REQ *request);
TRANSPORT_API(int,t_recv,void *buf,int count,COLL_DT datatype,int source,int tag,COLL_COMM comm,COLL_REQ *request);
TRANSPORT_API(int,t_recvreduce,void *buf,int count,COLL_DT datatype,COLL_OP op,int source,int tag,COLL_COMM comm,COLL_REQ *request);
TRANSPORT_API(int,t_test,int count,COLL_REQ *request);
TRANSPORT_API(void,t_reqalloc,COLL_REQ *req);
TRANSPORT_API(void,t_reqcomplete,COLL_REQ *req);
TRANSPORT_API(void,t_addref_op,COLL_OP op,int up);
TRANSPORT_API(void,t_addref_dt,COLL_DT dt,int up);
TRANSPORT_API(void *,t_malloc,size_t size);
TRANSPORT_API(void,t_free,void *ptr);
TRANSPORT_API(int,t_init);

#ifdef USE_DYNAMIC_COLL
#define TRANSPORT_F(x) x##_fn x
typedef struct TransportFunc {
  TRANSPORT_F(t_init);
  TRANSPORT_F(t_pack);
  TRANSPORT_F(t_unpack);
  TRANSPORT_F(t_isinplace);
  TRANSPORT_F(t_send);
  TRANSPORT_F(t_recv);
  TRANSPORT_F(t_recvreduce);
  TRANSPORT_F(t_test);
  TRANSPORT_F(t_rank);
  TRANSPORT_F(t_size);
  TRANSPORT_F(t_reduce_local);
  TRANSPORT_F(t_dtinfo);
  TRANSPORT_F(t_dtcopy);
  TRANSPORT_F(t_opinfo);
  TRANSPORT_F(t_addref_op);
  TRANSPORT_F(t_addref_dt);
  TRANSPORT_F(t_malloc);
  TRANSPORT_F(t_free);
  TRANSPORT_F(t_reqcomplete);
  TRANSPORT_F(t_reqalloc);
} TransportFunc;
#else
typedef struct TransportFunc {} TransportFunc;
#endif

#define BARRIER_PARAMS  COLL_COMM comm
#define ALLGATHER_PARAMS const void *sendbuf,int sendcount,COLL_DT sendtype, \
    void *recvbuf,int recvcount,COLL_DT recvtype,                       \
    COLL_COMM comm
#define ALLGATHERV_PARAMS const void *sendbuf,int sendcount,COLL_DT sendtype, \
    void *recvbuf,const int *recvcounts,const int *displs,              \
    COLL_DT recvtype, COLL_COMM comm
#define ALLREDUCE_PARAMS const void *sendbuf,void *recvbuf,int count,   \
    COLL_DT datatype,COLL_OP op,COLL_COMM comm
#define ALLTOALL_PARAMS const void *sendbuf,int sendcount,COLL_DT sendtype, \
    void *recvbuf,int recvcount,COLL_DT recvtype,                       \
    COLL_COMM comm
#define ALLTOALLV_PARAMS const void *sendbuf,const int *sendcnts,       \
    const int *sdispls,COLL_DT sendtype,void *recvbuf,                  \
    const int *recvcnts,const int *rdispls,                             \
    COLL_DT recvtype,COLL_COMM comm
#define NALLTOALLW_PARAMS const void *sendbuf,const int sendcounts[],   \
    const COLL_Aint sdispls[],const COLL_DT sendtypes[],                \
    void *recvbuf,const int recvcounts[],const COLL_Aint rdispls[],     \
    const COLL_DT recvtypes[],                                          \
    COLL_COMM comm
#define ALLTOALLW_PARAMS const void *sendbuf,const int *sendcnts,       \
    const int *sdispls,const COLL_DT *sendtypes,                        \
    void *recvbuf,const int *recvcnts,const int *rdispls,               \
    const COLL_DT *recvtypes,COLL_COMM comm
#define BCAST_PARAMS void *buffer,int count,COLL_DT datatype,int root,  \
    COLL_COMM comm
#define BCAST_CALL buffer,count,DTCAST(datatype),       \
    root,COMMCAST(comm)
#define EXSCAN_PARAMS const void *sendbuf,void *recvbuf,int count,      \
    COLL_DT datatype,COLL_OP op,COLL_COMM comm
#define EXSCAN_CALL sendbuf,recvbuf,count,DTCAST(datatype),     \
    OPCAST(op),COMMCAST(comm)
#define GATHER_PARAMS const void *sendbuf,int sendcnt,COLL_DT sendtype, \
    void *recvbuf,int recvcnt,COLL_DT recvtype,int root,                \
    COLL_COMM comm
#define GATHERV_PARAMS const void *sendbuf,int sendcnt,COLL_DT sendtype, \
    void *recvbuf,const int *recvcnts,const int *displs,                \
    COLL_DT recvtype,int root,COLL_COMM comm
#define REDUCE_SCATTER_PARAMS const void *sendbuf,void *recvbuf,        \
    const int *recvcnts,COLL_DT datatype,COLL_OP op,                    \
    COLL_COMM comm
#define REDUCE_SCATTER_BLOCK_PARAMS const void *sendbuf,void *recvbuf,  \
    int recvcount,COLL_DT datatype,COLL_OP op,                          \
    COLL_COMM comm
#define REDUCE_PARAMS const void *sendbuf,void *recvbuf,int count,COLL_DT datatype, \
    COLL_OP op,int root,COLL_COMM comm
#define SCAN_PARAMS const void *sendbuf,void *recvbuf,int count,        \
    COLL_DT datatype,COLL_OP op,COLL_COMM comm
#define SCATTERV_PARAMS const void *sendbuf,const int *sendcnts,const int *displs, \
    COLL_DT sendtype,void *recvbuf,int recvcnt,COLL_DT recvtype,        \
    int root,COLL_COMM comm
#define SCATTER_PARAMS const void *sendbuf,int sendcnt,COLL_DT sendtype,void *recvbuf, \
    int recvcnt,COLL_DT recvtype,int root,COLL_COMM comm

/* Initialization Routines */
COLL_API(int,Init_fn,Init,TransportFunc *input,size_t offset, size_t size,
         size_t req_offset, size_t req_sizeof,int *do_progress,
         COLL_DT control_dt);

COLL_API(int,Comm_init_fn,Comm_init,
         COLL_COMM comm);

/* Blocking Collectives */
COLL_API(int,Allgather_fn,Allgather,ALLGATHER_PARAMS,int *errflag);
COLL_API(int,Allgatherv_fn,Allgatherv,ALLGATHERV_PARAMS,int *errflag);
COLL_API(int,Allreduce_fn,Allreduce,ALLREDUCE_PARAMS,int *errflag);
COLL_API(int,Alltoall_fn,Alltoall,ALLTOALL_PARAMS,int *errflag);
COLL_API(int,Alltoallv_fn,Alltoallv,ALLTOALLV_PARAMS,int *errflag);
COLL_API(int,Alltoallw_fn,Alltoallw,ALLTOALLW_PARAMS,int *errflag);
COLL_API(int,Bcast_fn,Bcast,BCAST_PARAMS,int *errflag);
COLL_API(int,Exscan_fn,Exscan,EXSCAN_PARAMS,int *errflag);
COLL_API(int,Gather_fn,Gather,GATHER_PARAMS,int *errflag);
COLL_API(int,Gatherv_fn,Gatherv,GATHERV_PARAMS,int *errflag);
COLL_API(int,Reduce_scatter_fn,Reduce_scatter,REDUCE_SCATTER_PARAMS,int *errflag);
COLL_API(int,Reduce_scatter_block_fn,Reduce_scatter_block,REDUCE_SCATTER_BLOCK_PARAMS,int *errflag);
COLL_API(int,Reduce_fn,Reduce,REDUCE_PARAMS,int *errflag);
COLL_API(int,Scan_fn,Scan,SCAN_PARAMS,int *errflag);
COLL_API(int,Scatter_fn,Scatter,SCATTER_PARAMS,int *errflag);
COLL_API(int,Scatterv_fn,Scatterv,SCATTERV_PARAMS,int *errflag);
COLL_API(int,Barrier_fn,Barrier,BARRIER_PARAMS,int *errflag);

/* Non-blocking Collectives */
COLL_API(int,Iallgather_fn,Iallgather,ALLGATHER_PARAMS,COLL_REQ *request);
COLL_API(int,Iallgatherv_fn,Iallgatherv,ALLGATHERV_PARAMS,COLL_REQ *request);
COLL_API(int,Iallreduce_fn,Iallreduce,ALLREDUCE_PARAMS,COLL_REQ *request);
COLL_API(int,Ialltoall_fn,Ialltoall,ALLTOALL_PARAMS,COLL_REQ *request);
COLL_API(int,Ialltoallv_fn,Ialltoallv,ALLTOALLV_PARAMS,COLL_REQ *request);
COLL_API(int,Ialltoallw_fn,Ialltoallw,ALLTOALLW_PARAMS,COLL_REQ *request);
COLL_API(int,Ibcast_fn,Ibcast,BCAST_PARAMS,COLL_REQ *request);
COLL_API(int,Iexscan_fn,Iexscan,EXSCAN_PARAMS,COLL_REQ *request);
COLL_API(int,Igather_fn,Igather,GATHER_PARAMS,COLL_REQ *request);
COLL_API(int,Igatherv_fn,Igatherv,GATHERV_PARAMS,COLL_REQ *request);
COLL_API(int,Ireduce_scatter_fn,Ireduce_scatter,REDUCE_SCATTER_PARAMS,COLL_REQ *request);
COLL_API(int,Ireduce_scatter_block_fn,Ireduce_scatter_block,REDUCE_SCATTER_BLOCK_PARAMS,COLL_REQ *request);
COLL_API(int,Ireduce_fn,Ireduce,REDUCE_PARAMS,COLL_REQ *request);
COLL_API(int,Iscan_fn,Iscan,SCAN_PARAMS,COLL_REQ *request);
COLL_API(int,Iscatter_fn,Iscatter,SCATTER_PARAMS,COLL_REQ *request);
COLL_API(int,Iscatterv_fn,Iscatterv,SCATTERV_PARAMS,COLL_REQ *request);
COLL_API(int,Ibarrier_fn,Ibarrier,BARRIER_PARAMS,COLL_REQ *request);

/* Neighborhood Collectives */
COLL_API(int,Neighbor_allgather_fn,Neighbor_allgather,ALLGATHER_PARAMS);
COLL_API(int,Neighbor_allgatherv_fn,Neighbor_allgatherv,ALLGATHERV_PARAMS);
COLL_API(int,Neighbor_alltoall_fn,Neighbor_alltoall,ALLTOALL_PARAMS);
COLL_API(int,Neighbor_alltoallv_fn,Neighbor_alltoallv,ALLTOALLV_PARAMS);
COLL_API(int,Neighbor_alltoallw_fn,Neighbor_alltoallw,NALLTOALLW_PARAMS);

/* Nonblocking Neighborhood Collectives */
COLL_API(int,Ineighbor_allgather_fn,Ineighbor_allgather,ALLGATHER_PARAMS,COLL_SCHED s);
COLL_API(int,Ineighbor_allgatherv_fn,Ineighbor_allgatherv,ALLGATHERV_PARAMS,COLL_SCHED s);
COLL_API(int,Ineighbor_alltoall_fn,Ineighbor_alltoall,ALLTOALL_PARAMS,COLL_SCHED s);
COLL_API(int,Ineighbor_alltoallv_fn,Ineighbor_alltoallv,ALLTOALLV_PARAMS,COLL_SCHED s);
COLL_API(int,Ineighbor_alltoallw_fn,Ineighbor_alltoallw,NALLTOALLW_PARAMS,COLL_SCHED s);

/* Progress function for nonblocking */
COLL_API(int,Kick_fn,Kick,void);

#ifdef USE_DYNAMIC_COLL
/* Collectives Function table layout */
#define COLL_F(x) x##_fn x
struct CollFunc {
  COLL_F(Init);
  COLL_F(Comm_init);

  COLL_F(Allgather);
  COLL_F(Allgatherv);
  COLL_F(Allreduce);
  COLL_F(Alltoall);
  COLL_F(Alltoallv);
  COLL_F(Alltoallw);
  COLL_F(Bcast);
  COLL_F(Exscan);

  COLL_F(Gather);
  COLL_F(Gatherv);
  COLL_F(Reduce_scatter);
  COLL_F(Reduce);
  COLL_F(Scan);
  COLL_F(Reduce_scatter_block);
  COLL_F(Scatter);
  COLL_F(Scatterv);
  COLL_F(Barrier);

  COLL_F(Iallgather);
  COLL_F(Iallgatherv);
  COLL_F(Iallreduce);
  COLL_F(Ialltoall);
  COLL_F(Ialltoallv);
  COLL_F(Ialltoallw);
  COLL_F(Ibcast);
  COLL_F(Iexscan);

  COLL_F(Igather);
  COLL_F(Igatherv);
  COLL_F(Ireduce_scatter);
  COLL_F(Ireduce);
  COLL_F(Iscan);
  COLL_F(Ireduce_scatter_block);
  COLL_F(Iscatter);
  COLL_F(Iscatterv);
  COLL_F(Ibarrier);

  COLL_F(Neighbor_allgather);
  COLL_F(Neighbor_allgatherv);
  COLL_F(Neighbor_alltoall);
  COLL_F(Neighbor_alltoallv);
  COLL_F(Neighbor_alltoallw);

  COLL_F(Ineighbor_allgather);
  COLL_F(Ineighbor_allgatherv);
  COLL_F(Ineighbor_alltoall);
  COLL_F(Ineighbor_alltoallv);
  COLL_F(Ineighbor_alltoallw);

  COLL_F(Kick);
};

#define COLL_FN(TABLE,FCN) TABLE.FCN
#define TRANSPORT_FN(TABLE,FCN) TABLE.FCN

#define COLL_Init                         COLL_FN(COLL_OBJ,Init)
#define COLL_Comm_init                    COLL_FN(COLL_OBJ,Comm_init)
#define COLL_Allgather                    COLL_FN(COLL_OBJ,Allgather)
#define COLL_Allgatherv                   COLL_FN(COLL_OBJ,Allgatherv)
#define COLL_Allreduce                    COLL_FN(COLL_OBJ,Allreduce)
#define COLL_Alltoall                     COLL_FN(COLL_OBJ,Alltoall)
#define COLL_Alltoallv                    COLL_FN(COLL_OBJ,Alltoallv)
#define COLL_Alltoallw                    COLL_FN(COLL_OBJ,Alltoallw)
#define COLL_Bcast                        COLL_FN(COLL_OBJ,Bcast)
#define COLL_Exscan                       COLL_FN(COLL_OBJ,Exscan)
#define COLL_Gather                       COLL_FN(COLL_OBJ,Gather)
#define COLL_Gatherv                      COLL_FN(COLL_OBJ,Gatherv)
#define COLL_Reduce_scatter               COLL_FN(COLL_OBJ,Reduce_scatter)
#define COLL_Reduce                       COLL_FN(COLL_OBJ,Reduce)
#define COLL_Scan                         COLL_FN(COLL_OBJ,Scan)
#define COLL_Reduce_scatter_block         COLL_FN(COLL_OBJ,Reduce_scatter_block)
#define COLL_Scatter                      COLL_FN(COLL_OBJ,Scatter)
#define COLL_Scatterv                     COLL_FN(COLL_OBJ,Scatterv)
#define COLL_Barrier                      COLL_FN(COLL_OBJ,Barrier)
#define COLL_Iallgather                   COLL_FN(COLL_OBJ,Iallgather)
#define COLL_Iallgatherv                  COLL_FN(COLL_OBJ,Iallgatherv)
#define COLL_Iallreduce                   COLL_FN(COLL_OBJ,Iallreduce)
#define COLL_Ialltoall                    COLL_FN(COLL_OBJ,Ialltoall)
#define COLL_Ialltoallv                   COLL_FN(COLL_OBJ,Ialltoallv)
#define COLL_Ialltoallw                   COLL_FN(COLL_OBJ,Ialltoallw)
#define COLL_Ibcast                       COLL_FN(COLL_OBJ,Ibcast)
#define COLL_Iexscan                      COLL_FN(COLL_OBJ,Iexscan)
#define COLL_Igather                      COLL_FN(COLL_OBJ,Igather)
#define COLL_Igatherv                     COLL_FN(COLL_OBJ,Igatherv)
#define COLL_Ireduce_scatter              COLL_FN(COLL_OBJ,Ireduce_scatter)
#define COLL_Ireduce                      COLL_FN(COLL_OBJ,Ireduce)
#define COLL_Iscan                        COLL_FN(COLL_OBJ,Iscan)
#define COLL_Ireduce_scatter_block        COLL_FN(COLL_OBJ,Ireduce_scatter_block)
#define COLL_Iscatter                     COLL_FN(COLL_OBJ,Iscatter)
#define COLL_Iscatterv                    COLL_FN(COLL_OBJ,Iscatterv)
#define COLL_Ibarrier                     COLL_FN(COLL_OBJ,Ibarrier)
#define COLL_Neighbor_allgather           COLL_FN(COLL_OBJ,Neighbor_allgather)
#define COLL_Neighbor_allgatherv          COLL_FN(COLL_OBJ,Neighbor_allgatherv)
#define COLL_Neighbor_alltoall            COLL_FN(COLL_OBJ,Neighbor_alltoall)
#define COLL_Neighbor_alltoallv           COLL_FN(COLL_OBJ,Neighbor_alltoallv)
#define COLL_Neighbor_alltoallw           COLL_FN(COLL_OBJ,Neighbor_alltoallw)
#define COLL_Ineighbor_allgather          COLL_FN(COLL_OBJ,Ineighbor_allgather)
#define COLL_Ineighbor_allgatherv         COLL_FN(COLL_OBJ,Ineighbor_allgatherv)
#define COLL_Ineighbor_alltoall           COLL_FN(COLL_OBJ,Ineighbor_alltoall)
#define COLL_Ineighbor_alltoallv          COLL_FN(COLL_OBJ,Ineighbor_alltoallv)
#define COLL_Ineighbor_alltoallw          COLL_FN(COLL_OBJ,Ineighbor_alltoallw)
#define COLL_Kick                         COLL_FN(COLL_OBJ,Kick)

#define TRANSPORT_t_init          TRANSPORT_FN(TRANSPORT_OBJ,t_init)
#define TRANSPORT_t_pack          TRANSPORT_FN(TRANSPORT_OBJ,t_pack)
#define TRANSPORT_t_unpack        TRANSPORT_FN(TRANSPORT_OBJ,t_unpack)
#define TRANSPORT_t_isinplace     TRANSPORT_FN(TRANSPORT_OBJ,t_isinplace)
#define TRANSPORT_t_send          TRANSPORT_FN(TRANSPORT_OBJ,t_send)
#define TRANSPORT_t_recv          TRANSPORT_FN(TRANSPORT_OBJ,t_recv)
#define TRANSPORT_t_recvreduce    TRANSPORT_FN(TRANSPORT_OBJ,t_recvreduce)
#define TRANSPORT_t_test          TRANSPORT_FN(TRANSPORT_OBJ,t_test)
#define TRANSPORT_t_rank          TRANSPORT_FN(TRANSPORT_OBJ,t_rank)
#define TRANSPORT_t_size          TRANSPORT_FN(TRANSPORT_OBJ,t_size)
#define TRANSPORT_t_reduce_local  TRANSPORT_FN(TRANSPORT_OBJ,t_reduce_local)
#define TRANSPORT_t_dtinfo        TRANSPORT_FN(TRANSPORT_OBJ,t_dtinfo)
#define TRANSPORT_t_dtcopy        TRANSPORT_FN(TRANSPORT_OBJ,t_dtcopy)
#define TRANSPORT_t_opinfo        TRANSPORT_FN(TRANSPORT_OBJ,t_opinfo)
#define TRANSPORT_t_addref_op     TRANSPORT_FN(TRANSPORT_OBJ,t_addref_op)
#define TRANSPORT_t_addref_dt     TRANSPORT_FN(TRANSPORT_OBJ,t_addref_dt)
#define TRANSPORT_t_malloc        TRANSPORT_FN(TRANSPORT_OBJ,t_malloc)
#define TRANSPORT_t_free          TRANSPORT_FN(TRANSPORT_OBJ,t_free)
#define TRANSPORT_t_reqcomplete   TRANSPORT_FN(TRANSPORT_OBJ,t_reqcomplete)
#define TRANSPORT_t_reqalloc      TRANSPORT_FN(TRANSPORT_OBJ,t_reqalloc)

#else

struct CollFunc {};


#define COLL_Init                         COLLI_Init
#define COLL_Comm_init                    COLLI_Comm_init
#define COLL_Allgather                    COLLI_Allgather
#define COLL_Allgatherv                   COLLI_Allgatherv
#define COLL_Allreduce                    COLLI_Allreduce
#define COLL_Alltoall                     COLLI_Alltoall
#define COLL_Alltoallv                    COLLI_Alltoallv
#define COLL_Alltoallw                    COLLI_Alltoallw
#define COLL_Bcast                        COLLI_Bcast
#define COLL_Exscan                       COLLI_Exscan
#define COLL_Gather                       COLLI_Gather
#define COLL_Gatherv                      COLLI_Gatherv
#define COLL_Reduce_scatter               COLLI_Reduce_scatter
#define COLL_Reduce                       COLLI_Reduce
#define COLL_Scan                         COLLI_Scan
#define COLL_Reduce_scatter_block         COLLI_Reduce_scatter_block
#define COLL_Scatter                      COLLI_Scatter
#define COLL_Scatterv                     COLLI_Scatterv
#define COLL_Barrier                      COLLI_Barrier
#define COLL_Iallgather                   COLLI_Iallgather
#define COLL_Iallgatherv                  COLLI_Iallgatherv
#define COLL_Iallreduce                   COLLI_Iallreduce
#define COLL_Ialltoall                    COLLI_Ialltoall
#define COLL_Ialltoallv                   COLLI_Ialltoallv
#define COLL_Ialltoallw                   COLLI_Ialltoallw
#define COLL_Ibcast                       COLLI_Ibcast
#define COLL_Iexscan                      COLLI_Iexscan
#define COLL_Igather                      COLLI_Igather
#define COLL_Igatherv                     COLLI_Igatherv
#define COLL_Ireduce_scatter              COLLI_Ireduce_scatter
#define COLL_Ireduce                      COLLI_Ireduce
#define COLL_Iscan                        COLLI_Iscan
#define COLL_Ireduce_scatter_block        COLLI_Ireduce_scatter_block
#define COLL_Iscatter                     COLLI_Iscatter
#define COLL_Iscatterv                    COLLI_Iscatterv
#define COLL_Ibarrier                     COLLI_Ibarrier
#define COLL_Neighbor_allgather           COLLI_Neighbor_allgather
#define COLL_Neighbor_allgatherv          COLLI_Neighbor_allgatherv
#define COLL_Neighbor_alltoall            COLLI_Neighbor_alltoall
#define COLL_Neighbor_alltoallv           COLLI_Neighbor_alltoallv
#define COLL_Neighbor_alltoallw           COLLI_Neighbor_alltoallw
#define COLL_Ineighbor_allgather          COLLI_Ineighbor_allgather
#define COLL_Ineighbor_allgatherv         COLLI_Ineighbor_allgatherv
#define COLL_Ineighbor_alltoall           COLLI_Ineighbor_alltoall
#define COLL_Ineighbor_alltoallv          COLLI_Ineighbor_alltoallv
#define COLL_Ineighbor_alltoallw          COLLI_Ineighbor_alltoallw
#define COLL_Kick                         COLLI_Kick

#define TRANSPORT_t_init                  TRANSPORTI_t_init
#define TRANSPORT_t_pack                  TRANSPORTI_t_pack
#define TRANSPORT_t_unpack                TRANSPORTI_t_unpack
#define TRANSPORT_t_isinplace             TRANSPORTI_t_isinplace
#define TRANSPORT_t_send                  TRANSPORTI_t_send
#define TRANSPORT_t_recv                  TRANSPORTI_t_recv
#define TRANSPORT_t_recvreduce            TRANSPORTI_t_recvreduce
#define TRANSPORT_t_test                  TRANSPORTI_t_test
#define TRANSPORT_t_rank                  TRANSPORTI_t_rank
#define TRANSPORT_t_size                  TRANSPORTI_t_size
#define TRANSPORT_t_reduce_local          TRANSPORTI_t_reduce_local
#define TRANSPORT_t_dtinfo                TRANSPORTI_t_dtinfo
#define TRANSPORT_t_dtcopy                TRANSPORTI_t_dtcopy
#define TRANSPORT_t_opinfo                TRANSPORTI_t_opinfo
#define TRANSPORT_t_addref_op             TRANSPORTI_t_addref_op
#define TRANSPORT_t_addref_dt             TRANSPORTI_t_addref_dt
#define TRANSPORT_t_malloc                TRANSPORTI_t_malloc
#define TRANSPORT_t_free                  TRANSPORTI_t_free
#define TRANSPORT_t_reqcomplete           TRANSPORTI_t_reqcomplete
#define TRANSPORT_t_reqalloc              TRANSPORTI_t_reqalloc

#define COLL_FN(TABLE,FCN) COLL_##FCN
#define TRANSPORT_FN(TABLE,FCN) TRANSPORT_##FCN

#endif


#endif /* !defined(COLL_H_INCLUDED) */
