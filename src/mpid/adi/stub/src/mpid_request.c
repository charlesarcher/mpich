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
#include "./mpid_types.h"

__ADI_INLINE__ int MPIDI_Request_is_anysource(MPIR_Request *req)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ int MPIDI_Request_is_pending_failure(MPIR_Request *req)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ void MPIDI_Request_set_completed(MPIR_Request *req)
{
  assert(0);
  return;
}

__ADI_INLINE__ void MPIDI_Request_free_hook(MPIR_Request *req)
{
  assert(0);
  return;
}

__ADI_INLINE__ int MPIDI_Request_complete(MPIR_Request *req)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ MPIR_Request *MPIDI_Request_create(void)
{
  assert(0);
  return ((MPIR_Request *)0);
}
