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

__ADI_INLINE__ int MPIDI_Probe(int         source,
                               int         tag,
                               MPIR_Comm  *comm,
                               int         context_offset,
                               MPI_Status *status)
{
  int mpi_errno = MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}


__ADI_INLINE__ int MPIDI_Mprobe(int            source,
                                int            tag,
                                MPIR_Comm     *comm,
                                int            context_offset,
                                MPIR_Request **message,
                                MPI_Status    *status)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Improbe(int            source,
                                 int            tag,
                                 MPIR_Comm     *comm,
                                 int            context_offset,
                                 int           *flag,
                                 MPIR_Request **message,
                                 MPI_Status    *status)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Iprobe(int         source,
                                int         tag,
                                MPIR_Comm  *comm,
                                int         context_offset,
                                int        *flag,
                                MPI_Status *status)
{
  assert(0);
  return MPI_SUCCESS;
}
