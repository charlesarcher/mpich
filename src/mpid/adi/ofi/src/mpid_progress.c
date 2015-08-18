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

#define NUM_CQ_ENTRIES 8

__SI__ int coll_progress(int needs_progress)
{
  if(needs_progress) {
    int made_progress = 0;
    if(MPIDI_Global.coll_progress)
    {
      made_progress++;
      COLL_Kick();
    }
    return made_progress;
  }
  return 0;
}

__SI__ MPIR_Request *devreq_to_req(void *context)
{
  char *base = (char *)context;
  return (MPIR_Request *) container_of(base,MPIR_Request,dev.pad);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(ofi_poll)
__SI__ int ofi_poll(int in_blocking_poll)
{
  int                mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  int                ret, need_coll_progress=1;
  cq_tagged_entry_t  wc[NUM_CQ_ENTRIES];
  MPIR_Request      *req;
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);

  do {
  do_progress:
    MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
    ret = fi_cq_read(MPIDI_Global.p2p_cq,(void *)wc,NUM_CQ_ENTRIES);
    MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
    if(ret > 0) {
      int count;
      for(count=0; count < ret; count++)
      {
        req = devreq_to_req(wc[count].op_context);
        mpi_errno = REQ_OFI(req,callback)(&wc[count],req);
      }
      coll_progress(1);  need_coll_progress=0;
    } else if (ret == -FI_EAGAIN) {
      if(coll_progress(need_coll_progress))
        goto do_progress;
    }
    else if(ret < 0) {
      if(ret == -FI_EAVAIL) {
        cq_err_entry_t e;
        MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
        fi_cq_readerr(MPIDI_Global.p2p_cq,&e,0);
        MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
        if(e.err == FI_ETRUNC) {
          /* This error message should only be delivered on send
           * events.  We want to ignore truncation errors
           * on the sender side, but complete the request anyways
           * Other kinds of requests, this is fatal.
           */
          req = devreq_to_req(e.op_context);

          if(req->kind == MPIR_REQUEST_KIND__SEND)
            mpi_errno = REQ_OFI(req,callback)(NULL,req);
          else if(req->kind == MPIR_REQUEST_KIND__RECV) {
            mpi_errno = REQ_OFI(req,callback)((cq_tagged_entry_t *)&e,req);
            req->status.MPI_ERROR = MPI_ERR_TRUNCATE;
          } else
            MPIR_ERR_SETFATALANDJUMP4(mpi_errno,MPI_ERR_OTHER,"**ofid_poll",
                                      "**ofid_poll %s %d %s %s",__SHORT_FILE__,__LINE__,
                                      FCNAME,fi_strerror(e.err));
        } else if (e.err == FI_ECANCELED) {
          req = devreq_to_req(e.op_context);
          MPIR_STATUS_SET_CANCEL_BIT(req->status, TRUE);
        } else
          MPIR_ERR_SETFATALANDJUMP4(mpi_errno,MPI_ERR_OTHER,"**ofid_poll",
                                    "**ofid_poll %s %d %s %s",__SHORT_FILE__,__LINE__,
                                    FCNAME,fi_strerror(e.err));
      } else
        MPIR_ERR_SETFATALANDJUMP4(mpi_errno,MPI_ERR_OTHER,"**ofid_poll",
                                  "**ofid_poll %s %d %s %s",__SHORT_FILE__,__LINE__,
                                  FCNAME,fi_strerror(errno));
    }
  } while (ret > 0);
  int i;
  for (i = 0; i < MAX_PROGRESS_HOOKS; i++) {
    if (MPIDI_Global.progress_hooks[i].active == TRUE) {
      int made_progress;
      MPIR_Assert(MPIDI_Global.progress_hooks[i].func_ptr != NULL);
      mpi_errno = MPIDI_Global.progress_hooks[i].func_ptr(&made_progress);
      if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }
  }


fn_exit:
  MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
  MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);

  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_test)
__ADI_INLINE__ int MPIDI_Progress_test(void)
{
  int mpi_errno;
  BEGIN_FUNC(FCNAME);
  mpi_errno = ofi_poll(0);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_poke)
__ADI_INLINE__ int MPIDI_Progress_poke(void)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  ofi_poll(0);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_start)
__ADI_INLINE__ void MPIDI_Progress_start(MPID_Progress_state *state)
{
  /* Do Nothing */
  return;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_end)
__ADI_INLINE__ void MPIDI_Progress_end(MPID_Progress_state *state)
{
  /* Do Nothing */
  return;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_wait)
__ADI_INLINE__ int MPIDI_Progress_wait(MPID_Progress_state *state)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  ofi_poll(0);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_register)
__ADI_INLINE__ int MPIDI_Progress_register(int (*progress_fn)(int*), int *id)
{
  int mpi_errno = MPI_SUCCESS;
  int i;
  BEGIN_FUNC(FCNAME);
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);
  for (i = 0; i < MAX_PROGRESS_HOOKS; i++) {
    if (MPIDI_Global.progress_hooks[i].func_ptr == NULL) {
      MPIDI_Global.progress_hooks[i].func_ptr = progress_fn;
      MPIDI_Global.progress_hooks[i].active = FALSE;
      break;
    }
  }

  if (i >= MAX_PROGRESS_HOOKS)
    goto fn_fail;

  (*id) = i;

fn_exit:
  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  return MPIR_Err_create_code( MPI_SUCCESS, MPIR_ERR_RECOVERABLE,
                               "MPIDI_CH3I_Progress_register_hook", __LINE__,
                               MPI_ERR_INTERN, "**progresshookstoomany", 0 );
  goto fn_exit;
}
#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_deregister)
__ADI_INLINE__ int MPIDI_Progress_deregister(int id)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);

  MPIR_Assert(id >= 0 &&
              id < MAX_PROGRESS_HOOKS &&
              MPIDI_Global.progress_hooks[id].func_ptr != NULL);
  MPIDI_Global.progress_hooks[id].func_ptr = NULL;
  MPIDI_Global.progress_hooks[id].active = FALSE;

  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_activate)
__ADI_INLINE__ int MPIDI_Progress_activate(int id)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);

  MPIR_Assert(id >= 0 && id < MAX_PROGRESS_HOOKS &&
              MPIDI_Global.progress_hooks[id].active == FALSE &&
              MPIDI_Global.progress_hooks[id].func_ptr != NULL);
  MPIDI_Global.progress_hooks[id].active = TRUE;

  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;
}


#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_deactivate)
__ADI_INLINE__ int MPIDI_Progress_deactivate(int id)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);

  MPIR_Assert(id >= 0 && id < MAX_PROGRESS_HOOKS &&
              MPIDI_Global.progress_hooks[id].active == TRUE &&
              MPIDI_Global.progress_hooks[id].func_ptr != NULL);
  MPIDI_Global.progress_hooks[id].active = FALSE;

  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_PROGRESS_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;
}
