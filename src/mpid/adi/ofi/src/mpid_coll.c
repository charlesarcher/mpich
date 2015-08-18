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
#include "mpidimpl.h"
#include "mpid_types.h"

__ADI_INLINE__ int MPIDI_Barrier(MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Barrier_impl(comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Bcast(void * buffer, int count, MPI_Datatype datatype, int root,
                               MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Bcast_impl(buffer, count, datatype, root, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Allreduce(const void * sendbuf, void * recvbuf, int count,
                                   MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                   MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Allreduce_impl(sendbuf, recvbuf, count, datatype, op, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                   void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                   MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Allgather_impl(sendbuf, sendcount, sendtype, recvbuf,
                                  recvcount, recvtype, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                    void * recvbuf, const int * recvcounts, const int * displs,
                                    MPI_Datatype recvtype, MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Allgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcounts, displs, recvtype, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Scatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                 void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                 int root, MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Scatter_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                recvtype, root, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Scatterv(const void * sendbuf, const int * sendcounts, const int * displs,
                                  MPI_Datatype sendtype, void * recvbuf, int recvcount,
                                  MPI_Datatype recvtype, int root, MPIR_Comm * comm,
                                  MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Scatterv_impl(sendbuf, sendcounts, displs, sendtype, recvbuf,
                                 recvcount, recvtype, root, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Gather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                int root, MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Gather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                               recvtype, root, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Gatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                 void * recvbuf, const int * recvcounts, const int * displs,
                                 MPI_Datatype recvtype, int root, MPIR_Comm * comm,
                                 MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Gatherv_impl(sendbuf, sendcount, sendtype, recvbuf, recvcounts,
                                displs, recvtype, root, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                  void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                  MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Alltoall_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                 recvtype, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls,
                                   MPI_Datatype sendtype, void * recvbuf, const int * recvcounts,
                                   const int * rdispls, MPI_Datatype recvtype, MPIR_Comm * comm,
                                   MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype, recvbuf,
                                  recvcounts, rdispls, recvtype, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Alltoallw(const void *sendbuf, const int sendcounts[], const int sdispls[],
                                   const MPI_Datatype sendtypes[], void *recvbuf, const int recvcounts[],
                                   const int rdispls[], const MPI_Datatype recvtypes[],
                                   MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Alltoallw_impl(sendbuf, sendcounts, sdispls,
                                  sendtypes, recvbuf, recvcounts,
                                  rdispls, recvtypes, comm_ptr, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Reduce(const void * sendbuf, void * recvbuf, int count,
                                MPI_Datatype datatype, MPI_Op op, int root,
                                MPIR_Comm * comm, MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Reduce_impl(sendbuf, recvbuf, count, datatype, op, root, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Reduce_scatter(const void *sendbuf, void *recvbuf, const int recvcounts[],
                                        MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Reduce_scatter_impl(sendbuf, recvbuf, recvcounts,
                                       datatype, op, comm_ptr, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Reduce_scatter_block(const void *sendbuf, void *recvbuf,
                                              int recvcount, MPI_Datatype datatype,
                                              MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Reduce_scatter_block_impl(sendbuf, recvbuf, recvcount,
                                             datatype, op, comm_ptr, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Scan(const void * sendbuf, void * recvbuf, int count,
                              MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                              MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Scan_impl(sendbuf, recvbuf, count, datatype, op, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Exscan(const void * sendbuf, void * recvbuf, int count,
                                MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                MPIR_Errflag_t * errflag)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Exscan_impl(sendbuf, recvbuf, count, datatype, op, comm, errflag);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Neighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                            void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                            MPIR_Comm * comm)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Neighbor_allgather_impl(sendbuf, sendcount, sendtype, recvbuf,
                                           recvcount, recvtype, comm);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Neighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                             void * recvbuf, const int recvcounts[], const int displs[],
                                             MPI_Datatype recvtype, MPIR_Comm * comm)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Neighbor_allgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
                                            recvcounts, displs, recvtype, comm);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Neighbor_alltoallv(const void * sendbuf, const int sendcounts[],
                                            const int sdispls[], MPI_Datatype sendtype,
                                            void * recvbuf, const int recvcounts[],
                                            const int rdispls[], MPI_Datatype recvtype,
                                            MPIR_Comm * comm)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Neighbor_alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype,
                                           recvbuf, recvcounts, rdispls, recvtype, comm);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Neighbor_alltoallw(const void * sendbuf, const int sendcounts[],
                                            const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                                            void * recvbuf, const int recvcounts[],
                                            const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                                            MPIR_Comm * comm)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Neighbor_alltoallw_impl(sendbuf, sendcounts, sdispls,
                                           sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Neighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                           void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                           MPIR_Comm * comm)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Neighbor_alltoall_impl(sendbuf, sendcount, sendtype, recvbuf,
                                          recvcount, recvtype, comm);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ineighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                             void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                             MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ineighbor_allgather_impl(sendbuf, sendcount, sendtype, recvbuf,
                                            recvcount, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ineighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                              void * recvbuf, const int recvcounts[], const int displs[], MPI_Datatype recvtype,
                                              MPIR_Comm * comm, MPI_Request *request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ineighbor_allgatherv_impl(sendbuf, sendcount, sendtype,
                                             recvbuf, recvcounts, displs, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ineighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                            void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                            MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ineighbor_alltoall_impl(sendbuf, sendcount, sendtype, recvbuf,
                                           recvcount, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ineighbor_alltoallv(const void * sendbuf, const int sendcounts[],
                                             const int sdispls[], MPI_Datatype sendtype,
                                             void * recvbuf, const int recvcounts[],
                                             const int rdispls[], MPI_Datatype recvtype,
                                             MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ineighbor_alltoallv_impl(sendbuf, sendcounts, sdispls,
                                            sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ineighbor_alltoallw(const void * sendbuf, const int sendcounts[],
                                             const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                                             void * recvbuf, const int recvcounts[],
                                             const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                                             MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ineighbor_alltoallw_impl(sendbuf, sendcounts, sdispls,
                                            sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ibarrier(MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ibarrier_impl(comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ibcast(void * buffer, int count, MPI_Datatype datatype, int root,
                                MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ibcast_impl(buffer, count, datatype, root, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Iallgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                    void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                    MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Iallgather_impl(sendbuf, sendcount, sendtype, recvbuf,
                                   recvcount, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Iallgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                     void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype,
                                     MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Iallgatherv_impl(sendbuf, sendcount, sendtype, recvbuf,
                                    recvcounts, displs, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Iallreduce(const void * sendbuf, void * recvbuf, int count,
                                    MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                    MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Iallreduce_impl(sendbuf, recvbuf, count, datatype, op, comm,
                                   request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ialltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                   void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                   MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ialltoall_impl(sendbuf, sendcount, sendtype, recvbuf,
                                  recvcount, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ialltoallv(const void * sendbuf, const int sendcounts[],
                                    const int sdispls[], MPI_Datatype sendtype,
                                    void * recvbuf, const int recvcounts[],
                                    const int rdispls[], MPI_Datatype recvtype,
                                    MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ialltoallv_impl(sendbuf, sendcounts, sdispls, sendtype,
                                   recvbuf, recvcounts, rdispls, recvtype, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ialltoallw(const void * sendbuf, const int sendcounts[],
                                    const int sdispls[], const MPI_Datatype sendtypes[],
                                    void * recvbuf, const int recvcounts[],
                                    const int rdispls[], const MPI_Datatype recvtypes[],
                                    MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ialltoallw_impl(sendbuf, sendcounts, sdispls, sendtypes,
                                   recvbuf, recvcounts, rdispls, recvtypes, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Iexscan(const void * sendbuf, void * recvbuf, int count,
                                 MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                 MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Iexscan_impl(sendbuf, recvbuf, count, datatype, op, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Igather(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                 void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                 int root, MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Igather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                recvtype, root, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Igatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                  void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype,
                                  int root, MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Igatherv_impl(sendbuf, sendcount, sendtype, recvbuf, recvcounts,
                                 displs, recvtype, root, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ireduce_scatter_block(const void * sendbuf, void * recvbuf, int recvcount,
                                               MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                               MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ireduce_scatter_block_impl(sendbuf, recvbuf, recvcount,
                                              datatype, op, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ireduce_scatter(const void * sendbuf, void * recvbuf, const int recvcounts[],
                                         MPI_Datatype datatype, MPI_Op op, MPIR_Comm * comm,
                                         MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ireduce_scatter_impl(sendbuf, recvbuf, recvcounts, datatype,
                                        op, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Ireduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype,
                                 MPI_Op op, int root, MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Ireduce_impl(sendbuf, recvbuf, count, datatype, op, root, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Iscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype,
                               MPI_Op op, MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Iscan_impl(sendbuf, recvbuf, count, datatype, op, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Iscatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype,
                                  void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                  int root, MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Iscatter_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                 recvtype, root, comm, request);

  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Iscatterv(const void * sendbuf, const int * sendcounts,
                                   const int * displs, MPI_Datatype sendtype,
                                   void * recvbuf, int recvcount, MPI_Datatype recvtype,
                                   int root, MPIR_Comm * comm, MPI_Request * request)
{
  int mpi_errno = MPI_SUCCESS;

  mpi_errno = MPIR_Iscatterv_impl(sendbuf, sendcounts, displs, sendtype, recvbuf,
                                  recvcount, recvtype, root, comm, request);

  return mpi_errno;
}
