/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#include "../../coll.h"
#include <assert.h>

__COLL_INLINE__ int COLLI_Init(TransportFunc *fcn)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Comm_init(COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Kick(void)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Allgather(const void *sendbuf,
                                    int sendcount,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    int recvcount,
                                    COLL_DT recvtype,
                                    COLL_COMM comm,
                                    int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Allgatherv(const void *sendbuf,
                                     int sendcount,
                                     COLL_DT sendtype,
                                     void *recvbuf,
                                     const int *recvcounts,
                                     const int *displs,
                                     COLL_DT recvtype,
                                     COLL_COMM comm,
                                     int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Allreduce(const void *sendbuf,
                                    void *recvbuf,
                                    int count,
                                    COLL_DT datatype,
                                    COLL_OP op,
                                    COLL_COMM comm,
                                    int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Alltoall(const void *sendbuf,
                                   int sendcount,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   int recvcount,
                                   COLL_DT recvtype,
                                   COLL_COMM comm,
                                   int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Alltoallv(const void *sendbuf,
                                    const int *sendcnts,
                                    const int *sdispls,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    const int *recvcnts,
                                    const int *rdispls,
                                    COLL_DT recvtype,
                                    COLL_COMM comm,
                                    int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Alltoallw(const void *sendbuf,
                                    const int *sendcnts,
                                    const int *sdispls,
                                    const COLL_DT *sendtypes,
                                    void *recvbuf,
                                    const int *recvcnts,
                                    const int *rdispls,
                                    const COLL_DT *recvtypes,
                                    COLL_COMM comm,
                                    int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Bcast(void *buffer,
                                int count,
                                COLL_DT datatype,
                                int
                                root,
                                COLL_COMM comm,
                                int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Exscan(const void *sendbuf,
                                 void *recvbuf,
                                 int count,
                                 COLL_DT datatype,
                                 COLL_OP op,
                                 COLL_COMM comm,
                                 int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Gather(const void *sendbuf,
                                 int sendcnt,
                                 COLL_DT sendtype,
                                 void *recvbuf,
                                 int recvcnt,
                                 COLL_DT recvtype,
                                 int root,
                                 COLL_COMM comm,
                                 int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Gatherv(const void *sendbuf,
                                  int sendcnt,
                                  COLL_DT sendtype,
                                  void *recvbuf,
                                  const int *recvcnts,
                                  const int *displs,
                                  COLL_DT recvtype,
                                  int root,
                                  COLL_COMM comm,
                                  int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Reduce_scatter(const void *sendbuf,
                                         void *recvbuf,
                                         const int *recvcnts,
                                         COLL_DT datatype,
                                         COLL_OP op,
                                         COLL_COMM comm,
                                         int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Reduce_scatter_block(const void *sendbuf,
                                               void *recvbuf,
                                               int recvcount,
                                               COLL_DT datatype,
                                               COLL_OP op,
                                               COLL_COMM comm,
                                               int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Reduce(const void *sendbuf,
                                 void *recvbuf,
                                 int count,
                                 COLL_DT datatype,
                                 COLL_OP op,
                                 int root,
                                 COLL_COMM comm,
                                 int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Scan(const void *sendbuf,
                               void *recvbuf,
                               int count,
                               COLL_DT datatype,
                               COLL_OP op,
                               COLL_COMM comm,
                               int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Scatter(const void *sendbuf,
                                  int sendcnt,
                                  COLL_DT sendtype,
                                  void *recvbuf,
                                  int recvcnt,
                                  COLL_DT recvtype,
                                  int root,
                                  COLL_COMM comm,
                                  int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Scatterv(const void *sendbuf,
                                   const int *sendcnts,
                                   const int *displs,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   int recvcnt,
                                   COLL_DT recvtype,
                                   int root,
                                   COLL_COMM comm,
                                   int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Barrier(COLL_COMM comm,
                                  int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iallgather(const void *sendbuf,
                                     int sendcount,
                                     COLL_DT sendtype,
                                     void *recvbuf,
                                     int recvcount,
                                     COLL_DT recvtype,
                                     COLL_COMM comm,
                                     COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iallgatherv(const void *sendbuf,
                                      int sendcount,
                                      COLL_DT sendtype,
                                      void *recvbuf,
                                      const int *recvcounts,
                                      const int *displs,
                                      COLL_DT recvtype,
                                      COLL_COMM comm,
                                      COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iallreduce(const void *sendbuf,
                                     void *recvbuf,
                                     int count,
                                     COLL_DT datatype,
                                     COLL_OP op,
                                     COLL_COMM comm,
                                     COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ialltoall(const void *sendbuf,
                                    int sendcount,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    int recvcount,
                                    COLL_DT recvtype,
                                    COLL_COMM comm,
                                    COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ialltoallv(const void *sendbuf,
                                     const int *sendcnts,
                                     const int *sdispls,
                                     COLL_DT sendtype,
                                     void *recvbuf,
                                     const int *recvcnts,
                                     const int *rdispls,
                                     COLL_DT recvtype,
                                     COLL_COMM comm,
                                     COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ialltoallw(const void *sendbuf,
                                     const int *sendcnts,
                                     const int *sdispls,
                                     const COLL_DT *sendtypes,
                                     void *recvbuf,
                                     const int *recvcnts,
                                     const int *rdispls,
                                     const COLL_DT *recvtypes,
                                     COLL_COMM comm,
                                     COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ibcast(void *buffer,
                                 int count,
                                 COLL_DT datatype,
                                 int
                                 root,
                                 COLL_COMM comm,
                                 COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iexscan(const void *sendbuf,
                                  void *recvbuf,
                                  int count,
                                  COLL_DT datatype,
                                  COLL_OP op,
                                  COLL_COMM comm,
                                  COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Igather(const void *sendbuf,
                                  int sendcnt,
                                  COLL_DT sendtype,
                                  void *recvbuf,
                                  int recvcnt,
                                  COLL_DT recvtype,
                                  int root,
                                  COLL_COMM comm,
                                  COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Igatherv(const void *sendbuf,
                                   int sendcnt,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   const int *recvcnts,
                                   const int *displs,
                                   COLL_DT recvtype,
                                   int root,
                                   COLL_COMM comm,
                                   COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ireduce_scatter(const void *sendbuf,
                                          void *recvbuf,
                                          const int *recvcnts,
                                          COLL_DT datatype,
                                          COLL_OP op,
                                          COLL_COMM comm,
                                          COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ireduce_scatter_block(const void *sendbuf,
                                                void *recvbuf,
                                                int recvcount,
                                                COLL_DT datatype,
                                                COLL_OP op,
                                                COLL_COMM comm,
                                                COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ireduce(const void *sendbuf,
                                  void *recvbuf,
                                  int count,
                                  COLL_DT datatype,
                                  COLL_OP op,
                                  int root,
                                  COLL_COMM comm,
                                  COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iscan(const void *sendbuf,
                                void *recvbuf,
                                int count,
                                COLL_DT datatype,
                                COLL_OP op,
                                COLL_COMM comm,
                                COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iscatter(const void *sendbuf,
                                   int sendcnt,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   int recvcnt,
                                   COLL_DT recvtype,
                                   int root,
                                   COLL_COMM comm,
                                   COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iscatterv(const void *sendbuf,
                                    const int *sendcnts,
                                    const int *displs,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    int recvcnt,
                                    COLL_DT recvtype,
                                    int root,
                                    COLL_COMM comm,
                                    COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ibarrier(COLL_COMM comm,
                                   COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_allgather(const void *sendbuf,
                                             int sendcount,
                                             COLL_DT sendtype,
                                             void *recvbuf,
                                             int recvcount,
                                             COLL_DT recvtype,
                                             COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_allgatherv(const void *sendbuf,
                                              int sendcount,
                                              COLL_DT sendtype,
                                              void *recvbuf,
                                              const int recvcounts[],
                                              const int displs[],
                                              COLL_DT recvtype,
                                              COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_alltoall(const void *sendbuf,
                                            int sendcount,
                                            COLL_DT sendtype,
                                            void *recvbuf,
                                            int recvcount,
                                            COLL_DT recvtype,
                                            COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_alltoallv(const void *sendbuf,
                                             const int sendcounts[],
                                             const int sdispls[],
                                             COLL_DT sendtype,
                                             void *recvbuf,
                                             const int recvcounts[],
                                             const int rdispls[],
                                             COLL_DT recvtype,
                                             COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_alltoallw(const void *sendbuf,
                                             const int sendcounts[],
                                             const COLL_Aint sdispls[],
                                             const COLL_DT sendtypes[],
                                             void *recvbuf,
                                             const int recvcounts[],
                                             const COLL_Aint rdispls[],
                                             const COLL_DT recvtypes[],
                                             COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_allgather(const void *sendbuf,
                                              int sendcount,
                                              COLL_DT sendtype,
                                              void *recvbuf,
                                              int recvcount,
                                              COLL_DT recvtype,
                                              COLL_COMM comm,
                                              COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_allgatherv(const void *sendbuf,
                                               int sendcount,
                                               COLL_DT sendtype,
                                               void *recvbuf,
                                               const int recvcounts[],
                                               const int displs[],
                                               COLL_DT recvtype,
                                               COLL_COMM comm,
                                               COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_alltoall(const void *sendbuf,
                                             int sendcount,
                                             COLL_DT sendtype,
                                             void *recvbuf,
                                             int recvcount,
                                             COLL_DT recvtype,
                                             COLL_COMM comm,
                                             COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_alltoallv(const void *sendbuf,
                                              const int sendcounts[],
                                              const int sdispls[],
                                              COLL_DT sendtype,
                                              void *recvbuf,
                                              const int recvcounts[],
                                              const int rdispls[],
                                              COLL_DT recvtype,
                                              COLL_COMM comm,
                                              COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_alltoallw(const void *sendbuf,
                                              const int sendcounts[],
                                              const COLL_Aint sdispls[],
                                              const COLL_DT sendtypes[],
                                              void *recvbuf,
                                              const int recvcounts[],
                                              const COLL_Aint rdispls[],
                                              const COLL_DT recvtypes[],
                                              COLL_COMM comm,
                                              COLL_SCHED s)
{
  assert(0);
  return 0;
}
