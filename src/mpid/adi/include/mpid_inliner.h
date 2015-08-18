/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef __cplusplus

#ifdef ADI_DEVICE_OFI
#include "../ofi/src/mpid_inline_helper.c"
#include "../ofi/src/mpid_init.c"
#include "../ofi/src/mpid_probe.c"
#include "../ofi/src/mpid_send.c"
#include "../ofi/src/mpid_recv.c"
#include "../ofi/src/mpid_request.c"
#include "../ofi/src/mpid_comm.c"
#include "../ofi/src/mpid_coll.c"
#include "../ofi/src/mpid_win.c"
#include "../ofi/src/mpid_progress.c"
#include "../ofi/src/mpid_dynproc.c"
#endif

#ifdef ADI_DEVICE_STUB
#include "../stub/src/mpid_init.c"
#include "../stub/src/mpid_probe.c"
#include "../stub/src/mpid_send.c"
#include "../stub/src/mpid_recv.c"
#include "../stub/src/mpid_request.c"
#include "../stub/src/mpid_comm.c"
#include "../stub/src/mpid_coll.c"
#include "../stub/src/mpid_win.c"
#include "../stub/src/mpid_progress.c"
#include "../stub/src/mpid_unimpl.cc"
#endif

#endif

#ifdef MPIDI_USE_DYNAMIC_ADI
#error "Fatal : Cannot use the dynamic adi loader with the inliner"
#endif
