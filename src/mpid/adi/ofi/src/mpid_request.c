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
#include <mpidimpl.h>
#include "mpid_types.h"

__ADI_INLINE__ int MPIDI_Request_is_pending_failure(MPIR_Request *req)
{
  return FALSE;
}

__ADI_INLINE__ int MPIDI_Request_is_anysource(MPIR_Request *req)
{
  assert(0);
  return MPI_SUCCESS;
}

__ADI_INLINE__ void MPIDI_Request_free_hook(MPIR_Request *req)
{
  int count;
  MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPIR_REQUEST);
  MPIR_Object_release_ref(req, &count);
  MPIR_Assert(count >= 0);

  if(count == 0) {
    MPIR_Assert(MPIR_cc_is_complete(&req->cc));

    if(req->comm)              MPIR_Comm_release(req->comm);

    if(req->u.ureq.greq_fns)          MPL_free(req->u.ureq.greq_fns);

    MPIDI_Request_tls_free(req);
  }

  return;
}

__ADI_INLINE__ void MPIDI_Request_destroy_hook(MPIR_Request *req)
{
  int count;
  MPIR_Assert(0);
  MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPIR_REQUEST);
  MPIR_Object_release_ref(req, &count);
  MPIR_Assert(count >= 0);

  if(count == 0) {
    MPIR_Assert(MPIR_cc_is_complete(&req->cc));

    if(req->comm)              MPIR_Comm_release(req->comm);

    if(req->u.ureq.greq_fns)          MPL_free(req->u.ureq.greq_fns);

    MPIDI_Request_tls_free(req);
  }

  return;
}

__ADI_INLINE__ int MPIDI_Request_complete(MPIR_Request *req)
{
  int count;
  MPIR_cc_decr(req->cc_ptr, &count);
  MPIR_Assert(count >= 0);
  MPIDI_Request_free_hook(req);
  return MPI_SUCCESS;
}

/* ADI request:  These two routines are for MPID grequests */
__ADI_INLINE__ void MPIDI_Request_create_hook(MPIR_Request *req)
{
  MPIDI_Request_alloc_and_init(req,1);
}

__ADI_INLINE__ void MPIDI_Request_init(MPIR_Request *req)
{
  return;
}

__ADI_INLINE__ void MPIDI_Request_finalize(MPIR_Request * req)
{
  return;
}

__ADI_INLINE__ void MPIDI_Request_set_completed(MPIR_Request *req)
{
  MPIR_cc_set(&req->cc, 0);
  return;
}
