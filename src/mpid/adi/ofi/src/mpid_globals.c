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

/* All global ADI data structures need to go in this file */
/* reference them with externs from other files           */

#include <mpidimpl.h>
#include "mpid_types.h"


MPIDI_Global_t       MPIDI_Global;
MPIDI_Addr_table_t  *MPIDI_Addr_table;


#ifndef MPIR_REQUEST_PREALLOC
#define MPIR_REQUEST_PREALLOC 32
#endif
MPIR_Request        MPIDI_Request_direct[MPIR_REQUEST_PREALLOC] = {{0}};
MPIR_Object_alloc_t MPIDI_Request_mem = {
  0, 0, 0, 0,
  MPIR_REQUEST,
  sizeof(MPIR_Request),
  MPIDI_Request_direct,
  MPIR_REQUEST_PREALLOC
};

#ifndef MPID_SSENDACKREQUEST_PREALLOC
#define MPID_SSENDACKREQUEST_PREALLOC 16
#endif
MPIDI_Ssendack_request   MPIDI_Ssendack_request_direct[MPID_SSENDACKREQUEST_PREALLOC] = {{{0}}};
MPIR_Object_alloc_t MPIDI_Ssendack_request_mem = {
  0, 0, 0, 0,
  (MPII_Object_kind)MPID_SSENDACK_REQUEST,
  sizeof(MPIDI_Ssendack_request),
  MPIDI_Ssendack_request_direct,
  MPID_SSENDACKREQUEST_PREALLOC
};
