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

__ADI_INLINE__ int MPIDI_Comm_spawn_multiple(int         count,
                                             char       *array_of_commands[],
                                             char      **array_of_argv[],
                                             const int   array_of_maxprocs[],
                                             MPIR_Info  *array_of_info_ptrs[],
                                             int         root,
                                             MPIR_Comm  *comm_ptr,
                                             MPIR_Comm **intercomm,
                                             int         array_of_errcodes[])
{
  int mpi_errno = MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

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

__ADI_INLINE__ int MPIDI_Comm_disconnect(MPIR_Comm *comm_ptr)
{
  int mpi_errno = MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}
__ADI_INLINE__ int MPIDI_Open_port(MPIR_Info *info_ptr,
                                   char      *port_name)
{
  int mpi_errno=MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Close_port(const char *port_name)
{
  int mpi_errno=MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Comm_accept(const char  *port_name,
                                     MPIR_Info   *info,
                                     int          root,
                                     MPIR_Comm   *comm,
                                     MPIR_Comm  **newcomm_ptr)
{
  int mpi_errno=MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Comm_connect(const char *port_name,
                                      MPIR_Info  *info,
                                      int         root,
                                      MPIR_Comm  *comm,
                                      MPIR_Comm **newcomm_ptr)
{
  int mpi_errno=MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Comm_create(MPIR_Comm *comm)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Comm_destroy(MPIR_Comm *comm)
{
  assert(0);
  return MPI_SUCCESS;
}
