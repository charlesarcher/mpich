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

__ADI_INLINE__ int MPIDI_Progress_test(void)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Progress_poke(void)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ void MPIDI_Progress_start(MPID_Progress_state *state)
{
  assert(0);
  return;
}

__ADI_INLINE__ void MPIDI_Progress_end(MPID_Progress_state *state)
{
  assert(0);
  return;
}

__ADI_INLINE__ int MPIDI_Progress_wait(MPID_Progress_state *state)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Progress_register(int (*progress_fn)(int*), int *id)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Progress_deregister(int id)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Progress_activate(int id)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Progress_deactivate(int id)
{
  assert(0);
  return MPI_SUCCESS;
}
