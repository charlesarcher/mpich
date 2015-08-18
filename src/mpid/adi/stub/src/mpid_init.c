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

__ADI_INLINE__ int MPIDI_Init(int    *argc,
                              char ***argv,
                              int     requested,
                              int    *provided,
                              int    *has_args,
                              int    *has_env)
{
  int mpi_errno = MPI_SUCCESS;


  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_InitCompleted(void)
{
  int mpi_errno = MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Finalize(void)
{
  int mpi_errno = MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Get_universe_size(int *universe_size)
{
  int err = MPI_SUCCESS;
  assert(0);
  return err;
}

__ADI_INLINE__ int MPIDI_Get_processor_name(char *name,
                                            int   namelen,
                                            int  *resultlen)
{
  int mpi_errno = MPI_SUCCESS;
  assert(0);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Abort(MPIR_Comm  *comm,
                               int         mpi_errno,
                               int         exit_code,
                               const char *error_msg)
{
  int err = MPI_SUCCESS;
  assert(0);
  return err;
}

__ADI_INLINE__ void *MPIDI_Alloc_mem(size_t size, MPIR_Info *info_ptr)
{
  assert(0);
  return NULL;
}
__ADI_INLINE__ int MPIDI_Free_mem(void *ptr)
{
  assert(0);
  return MPI_SUCCESS;
}
__ADI_INLINE__ int MPIDI_Comm_get_lpid(MPIR_Comm *comm_ptr,
                                       int        idx,
                                       int       *lpid_ptr,
                                       MPL_bool  is_remote)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Get_node_id(MPIR_Comm      *comm,
                                     int             rank,
                                     MPID_Node_id_t *id_p)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Get_max_node_id(MPIR_Comm      *comm,
                                         MPID_Node_id_t *max_id_p)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Create_intercomm_from_lpids(MPIR_Comm *newcomm_ptr,
                                                     int        size,
                                                     const int  lpids[])
{
  assert(0);
  return MPI_SUCCESS;
}


__ADI_INLINE__ MPI_Aint MPIDI_Aint_add(MPI_Aint base, MPI_Aint disp)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ MPI_Aint MPIDI_Aint_diff(MPI_Aint addr1, MPI_Aint addr2)
{
  assert(0);
  return MPI_SUCCESS;
}
