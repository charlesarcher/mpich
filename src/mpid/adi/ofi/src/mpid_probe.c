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

#undef  FCNAME
#define FCNAME DECL_FUNC(peek_callback)
__SI__ int peek_callback(cq_tagged_entry_t *wc,
                         MPIR_Request      *rreq)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  REQ_OFI(rreq,util_id)       = 1;
  rreq->status.MPI_SOURCE     = get_source(wc->tag);
  rreq->status.MPI_TAG        = get_tag(wc->tag);
  size_t count                = wc->len;
  rreq->status.MPI_ERROR      = MPI_SUCCESS;
  MPIR_STATUS_SET_COUNT(rreq->status, count);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(do_iprobe)
__SI__ int do_iprobe(int            source,
                     int            tag,
                     MPIR_Comm     *comm,
                     int            context_offset,
                     int           *flag,
                     MPI_Status    *status,
                     MPIR_Request **message,
                     uint64_t       peek_flags)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  int          ret;
  fi_addr_t    remote_proc;
  uint64_t     match_bits, mask_bits;
  MPIR_Request r,*rreq;  /* don't need to init request, output only */
  msg_tagged_t msg;

  if(unlikely(source == MPI_PROC_NULL)) {
    MPIR_Status_set_procnull(status);
    *flag                = true;

    if(message) *message = NULL;

    goto fn_exit;
  }
  else if(unlikely(MPI_ANY_SOURCE == source))
    remote_proc = FI_ADDR_UNSPEC;
  else
    remote_proc = _comm_to_phys(comm,source,MPIDI_API_TAG);

  if(message)
    REQ_CREATE(rreq);
  else
    rreq = &r;

  match_bits = init_recvtag(&mask_bits,
                            comm->context_id+context_offset,
                            source,tag);

  REQ_OFI(rreq,callback) = peek_callback;
  REQ_OFI(rreq,util_id)  = 0;

  msg.msg_iov   = NULL;
  msg.desc      = NULL;
  msg.iov_count = 0;
  msg.addr      = remote_proc;
  msg.tag       = match_bits;
  msg.ignore    = mask_bits;
  msg.context   = (void *) &(REQ_OFI(rreq,context));
  msg.data      = 0;

  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
  ret = fi_trecvmsg(G_RXC_TAG(0),&msg,peek_flags|FI_PEEK|FI_COMPLETION);
  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
  if(ret == -ENOMSG) {
    *flag   = 0;
    if(message) MPIDI_Request_tls_free(rreq);
    goto fn_exit;
  }
  MPIR_ERR_CHKANDJUMP4((ret<0), mpi_errno,MPI_ERR_OTHER,"**ofid_peek",
                       "**ofid_peek %s %d %s %s",__SHORT_FILE__,
                       __LINE__,FCNAME,fi_strerror(-ret));
  PROGRESS_WHILE(0 == REQ_OFI(rreq,util_id));
  if(REQ_OFI(rreq,util_id) == 1) {
    MPIR_Request_extract_status(rreq,status);
    *flag   = 1;
    if(message) *message = rreq;
  } else {
    *flag   = 0;
    if(message) MPIDI_Request_tls_free(rreq);
  }
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Mprobe)
__ADI_INLINE__ int MPIDI_Mprobe(int            source,
                                int            tag,
                                MPIR_Comm     *comm,
                                int            context_offset,
                                MPIR_Request **message,
                                MPI_Status    *status)
{
  int mpi_errno = MPI_SUCCESS;
  int flag      = 0;
  BEGIN_FUNC(FCNAME);

  while(flag == 0) {
    /* Set flags for mprobe peek, when ready */
    mpi_errno = MPIDI_Improbe(source,tag,comm,context_offset,
                              &flag,message,status);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    PROGRESS();
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Improbe)
__ADI_INLINE__ int MPIDI_Improbe(int            source,
                                 int            tag,
                                 MPIR_Comm     *comm,
                                 int            context_offset,
                                 int           *flag,
                                 MPIR_Request **message,
                                 MPI_Status    *status)
{
  BEGIN_FUNC(FCNAME);

  /* Set flags for mprobe peek, when ready */
  int mpi_errno = do_iprobe(source,tag,comm,context_offset,
                            flag,status,message,FI_CLAIM|FI_COMPLETION);

  if(*flag && *message) {
    (*message)->kind  = MPIR_REQUEST_KIND__MPROBE;
    (*message)->comm  = comm;
    MPIR_Object_add_ref(comm);
  }

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Probe)
__ADI_INLINE__ int MPIDI_Probe(int         source,
                               int         tag,
                               MPIR_Comm  *comm,
                               int         context_offset,
                               MPI_Status *status)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  int flag = 0;

  while(!flag) {
    mpi_errno = MPIDI_Iprobe(source,tag,comm,context_offset,
                             &flag,status);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    PROGRESS();
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Iprobe)
__ADI_INLINE__ int MPIDI_Iprobe(int         source,
                                int         tag,
                                MPIR_Comm  *comm,
                                int         context_offset,
                                int        *flag,
                                MPI_Status *status)
{
  int mpi_errno;

  BEGIN_FUNC(FCNAME);
  mpi_errno = do_iprobe(source,tag,comm,context_offset,
                        flag, status,NULL,0ULL);
  END_FUNC(FCNAME);
  return mpi_errno;
}
