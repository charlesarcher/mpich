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

__ADI_INLINE__ int MPIDI_Send(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Rsend(const void    *buf,
                               int            count,
                               MPI_Datatype   datatype,
                               int            dest,
                               int            tag,
                               MPIR_Comm     *comm,
                               int            context_offset,
                               MPIR_Request **request)
{
  int err = MPI_SUCCESS;
  assert(0);
  return err;
}



__ADI_INLINE__ int MPIDI_Irsend(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Ssend(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Startall(int           count,
                                  MPIR_Request *requests[])
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Send_init(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Ssend_init(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Bsend_init(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Rsend_init(const void    *buf,
                                    int            count,
                                    MPI_Datatype   datatype,
                                    int            dest,
                                    int            tag,
                                    MPIR_Comm     *comm,
                                    int            context_offset,
                                    MPIR_Request **request)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Isend(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Issend(const void    *buf,
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

__ADI_INLINE__ int MPIDI_Cancel_send(MPIR_Request *sreq)
{
  assert(0);
  return MPI_SUCCESS;
}
