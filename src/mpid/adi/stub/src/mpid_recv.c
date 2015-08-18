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

__ADI_INLINE__ int MPIDI_Recv(void          *buf,
                              int            count,
                              MPI_Datatype   datatype,
                              int            rank,
                              int            tag,
                              MPIR_Comm     *comm,
                              int            context_offset,
                              MPI_Status    *status,
                              MPIR_Request **request)
{
  int err = MPI_SUCCESS;
  assert(0);
  return err;
}

__ADI_INLINE__ int MPIDI_Recv_init(void *buf,
                                   int            count,
                                   MPI_Datatype   datatype,
                                   int            rank,
                                   int            tag,
                                   MPIR_Comm     *comm,
                                   int            context_offset,
                                   MPIR_Request **request)
{
  assert(0);
  return MPI_SUCCESS;
}


__ADI_INLINE__ int MPIDI_Mrecv(void         *buf,
                               int           count,
                               MPI_Datatype  datatype,
                               MPIR_Request *message,
                               MPI_Status   *status)
{
  assert(0);
  return MPI_SUCCESS;
}


__ADI_INLINE__ int MPIDI_Imrecv(void        *buf,
                                int          count,
                                MPI_Datatype datatype,
                                MPIR_Request *message,
                                MPIR_Request **rreqp)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Irecv(void          *buf,
                               int            count,
                               MPI_Datatype   datatype,
                               int            rank,
                               int            tag,
                               MPIR_Comm     *comm,
                               int            context_offset,
                               MPIR_Request **request)
{
  int err = MPI_SUCCESS;
  assert(0);
  return err;
}

__ADI_INLINE__ int MPIDI_Cancel_recv(MPIR_Request *rreq)
{
  assert(0);
  return MPI_SUCCESS;
}
