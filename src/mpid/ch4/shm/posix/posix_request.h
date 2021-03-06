/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef SHM_POSIX_REQUEST_H_INCLUDED
#define SHM_POSIX_REQUEST_H_INCLUDED

#include "posix_impl.h"

static inline void MPIDI_SHM_am_request_init(MPIR_Request * req)
{
    MPIR_Assert(0);
}

static inline void MPIDI_SHM_am_request_finalize(MPIR_Request * req)
{
    MPIR_Assert(0);
}

#endif /* SHM_POSIX_REQUEST_H_INCLUDED */
