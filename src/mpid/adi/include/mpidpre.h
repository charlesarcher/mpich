/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef MPIDPRE_H_INCLUDED
#define MPIDPRE_H_INCLUDED

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

typedef size_t MPIDI_msg_sz_t;
typedef int    MPID_Progress_state;

#include "mpidu_dataloop.h"
#include "mpid_thread.h"
#include "mpid_sched.h"
#include "mpid_timers_fallback.h"

#define MPID_INTERCOMM_NO_DYNPROC(x) (0)

/* A compile-time assertion macro.  It should cause a compilation error if (expr_) is false. */
#define ADI_COMPILE_TIME_ASSERT(expr_)                                  \
  do { switch(0) { case 0: case (expr_): default: break; } } while (0)

typedef struct {
  uint64_t pad[112/8];
} MPIDI_Devreq_t;

typedef struct {
  uint64_t pad[192/8];
} MPIDI_Devwin_t;

typedef struct {
  uint64_t pad[256/8];
} MPIDI_Devcomm_t;

typedef struct {
  uint32_t pad[4/4];
} MPIDI_Devdt_t;

typedef struct {
  uint64_t pad[32/8];
} MPIDI_Devgpid_t;


#define MPID_DEV_REQUEST_DECL    MPIDI_Devreq_t  dev;
#define MPID_DEV_WIN_DECL        MPIDI_Devwin_t  dev;
#define MPID_DEV_COMM_DECL       MPIDI_Devcomm_t dev;
#define MPID_DEV_DATATYPE_DECL   MPIDI_Devdt_t   dev;
#define MPID_DEV_GPID_DECL       MPIDI_Devgpid_t dev;
#define MPID_REQUEST_KIND_DECL   MPID_SSENDACK_REQUEST

#include "mpid_datatype_fallback.h"

#define MPID_Progress_register_hook(fn_,id_) MPID_Progress_register(fn_,id_)
#define MPID_Progress_deregister_hook(id_) MPID_Progress_deregister(id_)
#define MPID_Progress_activate_hook(id_) MPID_Progress_activate(id_)
#define MPID_Progress_deactivate_hook(id_) MPID_Progress_deactivate(id_)

#define HAVE_DEV_COMM_HOOK
#define MPID_Dev_comm_create_hook(a)  (MPID_Comm_create(a))
#define MPID_Dev_comm_destroy_hook(a) (MPID_Comm_destroy(a))

#define MPID_USE_NODE_IDS
typedef int16_t MPID_Node_id_t;

#ifndef HAVE_GETHOSTNAME
#define HAVE_GETHOSTNAME
#endif

#include "mpidu_pre.h"

#endif /* MPIDPRE_H_INCLUDED */
