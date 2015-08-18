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

enum recv_mode {
  ON_HEAP, USE_EXISTING
};

#undef  FCNAME
#define FCNAME DECL_FUNC(recv_callback)
__SI__ int recv_callback(cq_tagged_entry_t *wc,
                         MPIR_Request      *rreq)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPI_Aint last;
  rreq->status.MPI_ERROR  = MPI_SUCCESS;
  rreq->status.MPI_SOURCE = get_source(wc->tag);
  rreq->status.MPI_TAG    = get_tag(wc->tag);
  size_t count            = wc->len;

  MPIR_STATUS_SET_COUNT(rreq->status, count);

  if(REQ_OFI(rreq,pack_buffer)) {
    last = count;
    MPIDU_Segment_unpack(REQ_OFI(rreq,segment_ptr), 0, &last,
                         REQ_OFI(rreq,pack_buffer));
    MPL_free(REQ_OFI(rreq,pack_buffer));
    MPID_Segment_free(REQ_OFI(rreq,segment_ptr));

    if(last != count) {
      mpi_errno = MPIR_Err_create_code(MPI_SUCCESS,MPIR_ERR_RECOVERABLE,
                                       __FUNCTION__, __LINE__,
                                       MPI_ERR_TYPE, "**dtypemismatch",
                                       0);
      rreq->status.MPI_ERROR = mpi_errno;
    }
  }

  dtype_release_if_not_builtin(REQ_OFI(rreq,datatype));

  /* If syncronous, ack and complete when the ack is done */
  if(unlikely(is_tag_sync(wc->tag))) {
    uint64_t ss_bits = init_sendtag(REQ_OFI(rreq,util_id),
                                    REQ_OFI(rreq,util_comm->rank),
                                    rreq->status.MPI_TAG,
                                    MPID_SYNC_SEND_ACK);
    MPIR_Comm *c   = REQ_OFI(rreq,util_comm);
    int        r   = rreq->status.MPI_SOURCE;
    FI_RC_RETRY(fi_tinject(G_TXC_TAG(0),NULL,0,_comm_to_phys(c,r,MPIDI_API_TAG),
                           ss_bits),tsendsync);

  }
  MPIDI_Request_complete(rreq);

  /* Polling loop will check for truncation */
fn_exit:
  END_FUNC(FCNAME);

  return mpi_errno;
fn_fail:
  rreq->status.MPI_ERROR = mpi_errno;
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(recv_callback)
__SI__ int recv_callback_huge(cq_tagged_entry_t *wc,
                              MPIR_Request      *rreq)
{
  int                 mpi_errno = MPI_SUCCESS;
  MPIDI_Huge_recv_t  *recv;
  MPIDI_Huge_chunk_t *hc;
  MPIR_Comm          *comm_ptr;

  BEGIN_FUNC(FCNAME);
  /* Look up the receive sequence number and chunk queue */
  comm_ptr = REQ_OFI(rreq,util_comm);
  recv     = MPIDI_Map_lookup(COMM_OFI(comm_ptr)->huge_recv_counters,
                              get_source(wc->tag));

  if(recv == MPIDI_MAP_NOT_FOUND) {
    recv        = (MPIDI_Huge_recv_t *)MPL_malloc(sizeof(*recv));
    recv->seqno = 0;
    MPIDI_Map_create(&recv->chunk_q);
    MPIDI_Map_set(COMM_OFI(comm_ptr)->huge_recv_counters,
                  get_source(wc->tag),recv);
  }

  /* Look up the receive in the chunk queue */
  hc   = (MPIDI_Huge_chunk_t *)MPIDI_Map_lookup(recv->chunk_q,recv->seqno);

  if(hc == MPIDI_MAP_NOT_FOUND) {
    hc = (MPIDI_Huge_chunk_t *)MPL_malloc(sizeof(*hc));
    memset(hc,0, sizeof(*hc));
    hc->callback = MPIDI_Gethuge_callback;
    MPIDI_Map_set(recv->chunk_q,recv->seqno,hc);
  }

  recv->seqno++;
  hc->localreq       = rreq;
  hc->done_fn        = recv_callback;
  hc->wc             = *wc;
  MPIDI_Gethuge_callback(NULL, (MPIR_Request *)hc);

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_irecv)
__SI__ int do_irecv(void          *buf,
                    int            count,
                    MPI_Datatype   datatype,
                    int            rank,
                    int            tag,
                    MPIR_Comm     *comm,
                    int            context_offset,
                    MPIR_Request **request,
                    enum recv_mode mode,
                    uint64_t       flags)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPIR_Request      *rreq        = NULL;
  uint64_t           match_bits, mask_bits;
  MPIR_Context_id_t  context_id  = comm->recvcontext_id+context_offset;
  MPIDI_msg_sz_t     data_sz;
  int                dt_contig;
  MPI_Aint           dt_true_lb;
  MPIR_Datatype     *dt_ptr;

  if(mode == ON_HEAP)  /* Branch should compile out */
    REQ_CREATE(rreq);
  else if(mode == USE_EXISTING)
    rreq=*request;

  rreq->kind = MPIR_REQUEST_KIND__RECV;
  *request   = rreq;
  if(unlikely(rank == MPI_PROC_NULL)) {
    rreq->status.MPI_ERROR  = MPI_SUCCESS;
    rreq->status.MPI_SOURCE = rank;
    rreq->status.MPI_TAG    = tag;
    MPIDI_Request_complete(rreq);
    goto fn_exit;
  }

  match_bits = init_recvtag(&mask_bits,context_id,rank,tag);

  MPIDI_Datatype_get_info(count,datatype,dt_contig,
                          data_sz, dt_ptr, dt_true_lb);
  REQ_OFI(rreq,datatype)       = datatype;
  dtype_add_ref_if_not_builtin(datatype);

  char *recv_buf = (char *)buf+dt_true_lb;

  if(!dt_contig) {
    REQ_OFI(rreq,segment_ptr) = MPID_Segment_alloc();
    MPIR_ERR_CHKANDJUMP1(REQ_OFI(rreq,segment_ptr) == NULL, mpi_errno,
                         MPI_ERR_OTHER, "**nomem",
                         "**nomem %s",
                         "Recv MPID_Segment_alloc");
    MPID_Segment_init(buf, count,datatype,
                      REQ_OFI(rreq,segment_ptr),
                      0);

    REQ_OFI(rreq,pack_buffer) = (char *)MPL_malloc(data_sz);
    MPIR_ERR_CHKANDJUMP1(REQ_OFI(rreq,pack_buffer) == NULL, mpi_errno,
                         MPI_ERR_OTHER, "**nomem",
                         "**nomem %s",
                         "Recv Pack Buffer alloc");
    recv_buf = REQ_OFI(rreq,pack_buffer);
  } else
    REQ_OFI(rreq,pack_buffer) = NULL;

  REQ_OFI(rreq,util_comm) = comm;
  REQ_OFI(rreq,util_id)   = context_id;

  if(unlikely(data_sz > MPIDI_Global.max_send)) {
    REQ_OFI(rreq,callback) = recv_callback_huge;
    data_sz = MPIDI_Global.max_send;
  } else
    REQ_OFI(rreq,callback) = recv_callback;

  if(!flags) /* Branch should compile out */
    FI_RC_RETRY(fi_trecv(G_RXC_TAG(0),
                         recv_buf,
                         data_sz,
                         MPIDI_Global.mr,
                         (MPI_ANY_SOURCE == rank)?FI_ADDR_UNSPEC:_comm_to_phys(comm,rank,MPIDI_API_TAG),
                         match_bits,
                         mask_bits,
                         (void *) &(REQ_OFI(rreq,context))),trecv);
  else {
    iovec_t      iov;
    iov.iov_base = recv_buf;
    iov.iov_len  = data_sz;

    msg_tagged_t msg;
    msg.msg_iov   = &iov;
    msg.desc      = (void *)MPIDI_Global.mr;
    msg.iov_count = 1;
    msg.tag       = match_bits;
    msg.ignore    = mask_bits;
    msg.context   = (void *) &(REQ_OFI(rreq,context));
    msg.data      = 0;
    msg.addr      = FI_ADDR_UNSPEC;

    MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
    FI_RC_RETRY(fi_trecvmsg(G_RXC_TAG(0),&msg,flags),trecv);
    MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Recv)
__ADI_INLINE__ int MPIDI_Recv(void          *buf,
                              int            count,
                              MPI_Datatype   datatype,
                              int            rank,
                              int            tag,
                              MPIR_Comm     *comm,
                              int            context_offset,
                              MPI_Status    *status,
                              MPIR_Request **request)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  mpi_errno = do_irecv(buf,count,datatype,rank,tag,comm,
                       context_offset,request,ON_HEAP,0ULL);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Recv_init)
__ADI_INLINE__ int MPIDI_Recv_init(void          *buf,
                                   int            count,
                                   MPI_Datatype   datatype,
                                   int            rank,
                                   int            tag,
                                   MPIR_Comm     *comm,
                                   int            context_offset,
                                   MPIR_Request **request)
{
  BEGIN_FUNC(FCNAME);
  MPIR_Request *rreq;
  REQ_CREATE((rreq));

  *request   = rreq;
  rreq->kind = MPIR_REQUEST_KIND__RECV;
  rreq->comm = comm;
  MPIR_Comm_add_ref(comm);

  REQ_OFI(rreq,p_buf)          = (void *)buf;
  REQ_OFI(rreq,p_count)        = count;
  REQ_OFI(rreq,datatype)       = datatype;
  REQ_OFI(rreq,p_rank)         = rank;
  REQ_OFI(rreq,p_tag)          = tag;
  REQ_OFI(rreq,util_comm)      = comm;
  REQ_OFI(rreq,util_id)        = comm->context_id+context_offset;
  rreq->u.persist.real_request = NULL;

  MPIDI_Request_complete(rreq);

  REQ_OFI(rreq,p_type)     = MPIDI_PTYPE_RECV;

  if(HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN) {
    MPIR_Datatype  *dt_ptr;
    MPID_Datatype_get_ptr(datatype, dt_ptr);
    MPID_Datatype_add_ref(dt_ptr);
  }

  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Mrecv)
__ADI_INLINE__ int MPIDI_Mrecv(void         *buf,
                               int           count,
                               MPI_Datatype  datatype,
                               MPIR_Request *message,
                               MPI_Status   *status)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  MPI_Request   req_handle;
  int           active_flag;
  MPIR_Request *rreq = NULL;

  MPI_RC_POP(MPIDI_Imrecv(buf, count, datatype, message, &rreq));

  if(!MPIR_Request_is_complete(rreq))
    PROGRESS_WHILE(!MPIR_Request_is_complete(rreq));

  MPIR_Request_extract_status(rreq,status);
  MPI_RC_POP(MPIR_Request_complete(&req_handle, rreq, status, &active_flag));

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Imrecv)
__ADI_INLINE__ int MPIDI_Imrecv(void        *buf,
                                int          count,
                                MPI_Datatype datatype,
                                MPIR_Request *message,
                                MPIR_Request **rreqp)
{

  int           mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPIR_Request *rreq;

  if(message == NULL) {
    MPIDI_Request_create_null_rreq(rreq, mpi_errno,fn_fail);
    *rreqp = rreq;
    goto fn_exit;
  }

  MPIR_Assert(message != NULL);
  MPIR_Assert(message->kind == MPIR_REQUEST_KIND__MPROBE);

  *rreqp = rreq = message;

  mpi_errno = do_irecv(buf,count,datatype,message->status.MPI_SOURCE,
                       message->status.MPI_TAG,rreq->comm,0,
                       &rreq,USE_EXISTING,FI_CLAIM|FI_COMPLETION);

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Irecv)
__ADI_INLINE__ int MPIDI_Irecv(void          *buf,
                               int            count,
                               MPI_Datatype   datatype,
                               int            rank,
                               int            tag,
                               MPIR_Comm     *comm,
                               int            context_offset,
                               MPIR_Request **request)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  mpi_errno = do_irecv(buf,count,datatype,rank,tag,comm,
                       context_offset,request,ON_HEAP,0ULL);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Cancel_recv)
__ADI_INLINE__ int MPIDI_Cancel_recv(MPIR_Request *rreq)
{

  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  PROGRESS();
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
  ssize_t ret = fi_cancel((fid_t)G_RXC_TAG(0),
                          &(REQ_OFI(rreq,context)));
  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);

  if(ret==0) {
    while(!MPIR_STATUS_GET_CANCEL_BIT(rreq->status)) {
      if ((mpi_errno = MPIDI_Progress_test()) != MPI_SUCCESS)
        goto fn_exit;
    }
    MPIR_STATUS_SET_CANCEL_BIT(rreq->status, TRUE);
    MPIR_STATUS_SET_COUNT(rreq->status,0);
    MPIDI_Request_complete(rreq);
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}
