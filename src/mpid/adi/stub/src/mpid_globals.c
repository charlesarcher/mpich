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


int MPIDI_Put_Global_Data_here;

#ifndef MPIR_REQUEST_PREALLOC
#define MPIR_REQUEST_PREALLOC 16
#endif

MPIR_Request MPIDI_Request_direct[MPIR_REQUEST_PREALLOC] = {{0}};
MPIR_Object_alloc_t MPIDI_Request_mem = {
  0, 0, 0, 0,
  MPIR_REQUEST,
  sizeof(MPIR_Request),
  MPIDI_Request_direct,
  MPIR_REQUEST_PREALLOC
};
