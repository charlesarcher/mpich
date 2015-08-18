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
#include <../mpi/pt2pt/bsendutil.h>

#define SENDPARMS const void *buf,int count,MPI_Datatype datatype,  \
    int rank,int tag,MPIR_Comm *comm,                               \
    int context_offset,MPIR_Request **request

typedef struct MPIDI_Hugecntr {
  uint16_t counter;
  uint16_t outstanding;
} MPIDI_Hugecntr;

#define DO_SEND(NOREQ,SYNCFLAG)                                     \
  ({                                                                \
    int             mpi_errno,dt_contig;                            \
    MPIDI_msg_sz_t  data_sz;                                        \
    MPI_Aint        dt_true_lb;                                     \
    MPIR_Datatype  *dt_ptr;                                         \
    BEGIN_FUNC(FCNAME);                                             \
    if(unlikely(rank == MPI_PROC_NULL)) {                           \
      mpi_errno = MPI_SUCCESS;                                      \
      if(!NOREQ) {                                                  \
        REQ_CREATE((*request));                                     \
        (*request)->kind = MPIR_REQUEST_KIND__SEND;                 \
        MPIDI_Request_complete((*request));                         \
      }                                                             \
      goto fn_exit;                                                 \
    }                                                               \
    MPIDI_Datatype_get_info(count,datatype,dt_contig,               \
                            data_sz,dt_ptr,dt_true_lb);             \
    if(NOREQ&&dt_contig&&(data_sz<=MPIDI_Global.max_buffered_send)) \
      mpi_errno=do_send_lw((char*)buf+dt_true_lb,data_sz,           \
                           rank,tag,comm,context_offset);           \
    else                                                            \
      mpi_errno=do_send(buf,count,datatype,rank,tag,comm,           \
                        context_offset,request,dt_contig,           \
                        data_sz,dt_ptr,dt_true_lb,SYNCFLAG);        \
  fn_exit:                                                          \
    END_FUNC(FCNAME);                                               \
    return mpi_errno;                                               \
  })

#define DO_PSEND(type)                                          \
  ({                                                            \
    BEGIN_FUNC(FCNAME);                                         \
    int mpi_errno = do_psend(buf,count,datatype,rank,tag,       \
                               comm,context_offset,request);    \
    if (mpi_errno != MPI_SUCCESS)                               \
      goto fn_fail;                                             \
    REQ_OFI((*request),p_type) = type;                          \
fn_exit:                                                        \
END_FUNC(FCNAME);                                               \
return mpi_errno;                                               \
fn_fail:                                                        \
goto fn_exit;                                                   \
})

#undef  FCNAME
#define FCNAME DECL_FUNC(send_callback)
__SI__ int send_callback(cq_tagged_entry_t *wc,
                         MPIR_Request      *sreq)
{
  int mpi_errno = MPI_SUCCESS;
  int c;
  BEGIN_FUNC(FCNAME);
  MPIR_cc_decr(sreq->cc_ptr, &c);
  MPIR_Assert(c >= 0);

  if(c == 0) {
    if(REQ_OFI(sreq,pack_buffer))
      MPL_free(REQ_OFI(sreq,pack_buffer));

    dtype_release_if_not_builtin(REQ_OFI(sreq,datatype));
    MPIDI_Request_free_hook(sreq);
  }/* c != 0, ssend */

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(send_callback_huge)
__SI__ int send_callback_huge(cq_tagged_entry_t *wc,
                              MPIR_Request      *sreq)
{
  int        mpi_errno = MPI_SUCCESS;
  int        c;
  BEGIN_FUNC(FCNAME);

  MPIR_cc_decr(sreq->cc_ptr, &c);
  MPIR_Assert(c >= 0);

  if(c == 0) {
    MPIR_Comm      *comm;
    void           *ptr;
    MPIDI_Hugecntr *cntr;
    comm = REQ_OFI(sreq,util_comm);
    ptr  = MPIDI_Map_lookup(COMM_OFI(comm)->huge_send_counters,
                            REQ_OFI(sreq,util_id));
    MPIR_Assert(ptr != MPIDI_MAP_NOT_FOUND);
    cntr = (MPIDI_Hugecntr *)ptr;
    cntr->outstanding--;

    if(cntr->outstanding == 0) {
      MPIDI_Send_control_t ctrl;
      MPIDI_Map_erase(COMM_OFI(comm)->huge_send_counters,
                      REQ_OFI(sreq,util_id));
      MPL_free(ptr);
      ctrl.type  = MPIDI_CTRL_HUGE_CLEANUP;
      MPI_RC_POP(do_control_send(&ctrl,NULL,0,
                                 REQ_OFI(sreq,util_id),
                                 comm,NULL));
    }

    if(REQ_OFI(sreq,pack_buffer))
      MPL_free(REQ_OFI(sreq,pack_buffer));

    dtype_release_if_not_builtin(REQ_OFI(sreq,datatype));
    MPIDI_Request_free_hook(sreq);
  }/* c != 0, ssend */

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(send_callback)
__SI__ int ssend_ack_callback(cq_tagged_entry_t *wc,
                              MPIR_Request      *sreq)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPIDI_Ssendack_request *req = (MPIDI_Ssendack_request *)sreq;
  send_callback(NULL, req->signal_req);
  MPIDI_Ssendack_request_tls_free(req);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(do_send_lw)
__SI__ int do_send_lw(const void     *buf,
                      MPIDI_msg_sz_t  data_sz,
                      int             rank,
                      int             tag,
                      MPIR_Comm      *comm,
                      int             context_offset)
{
  int      mpi_errno = MPI_SUCCESS;
  uint64_t match_bits;
  BEGIN_FUNC(FCNAME);
  match_bits = init_sendtag(comm->context_id+context_offset,
                            comm->rank,tag,0);
  FI_RC_RETRY(fi_tinject(G_TXC_TAG(0),buf,data_sz,
                         _comm_to_phys(comm,rank,MPIDI_API_TAG),match_bits),tinject);
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(do_send)
__SI__ int do_send(const void      *buf,
                   int              count,
                   MPI_Datatype     datatype,
                   int              rank,
                   int              tag,
                   MPIR_Comm       *comm,
                   int              context_offset,
                   MPIR_Request   **request,
                   int              dt_contig,
                   MPIDI_msg_sz_t   data_sz,
                   MPIR_Datatype   *dt_ptr,
                   MPI_Aint         dt_true_lb,
                   uint64_t         type)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  MPIR_Request   *sreq        = NULL;
  MPI_Aint        last;
  char           *send_buf;
  uint64_t        match_bits;

  REQ_CREATE(sreq);
  sreq->kind = MPIR_REQUEST_KIND__SEND;
  *request   = sreq;
  match_bits = init_sendtag(comm->context_id+context_offset,
                            comm->rank,tag,type);
  REQ_OFI(sreq,callback) = send_callback;
  REQ_OFI(sreq,datatype) = datatype;
  dtype_add_ref_if_not_builtin(datatype);

  if(type == MPID_SYNC_SEND) { /* Branch should compile out */
    int c = 1;
    uint64_t ssend_match,ssend_mask;
    MPIDI_Ssendack_request *ackreq;
    SSENDACKREQ_CREATE(ackreq);
    ackreq->callback       = ssend_ack_callback;
    ackreq->signal_req     = sreq;
    MPIR_cc_incr(sreq->cc_ptr, &c);
    ssend_match = init_recvtag(&ssend_mask,
                               comm->context_id+context_offset,
                               rank,
                               tag);
    ssend_match |= MPID_SYNC_SEND_ACK;
    FI_RC_RETRY(fi_trecv(G_RXC_TAG(0),          /* Socket      */
                         NULL,                     /* recvbuf     */
                         0,                        /* data sz     */
                         MPIDI_Global.mr,          /* data descr  */
                         _comm_to_phys(comm,rank,MPIDI_API_TAG),  /* remote proc */
                         ssend_match,              /* match bits  */
                         0ULL,                     /* mask bits   */
                         (void *) &(ackreq->context)),trecvsync);
  }

  send_buf = (char *)buf+dt_true_lb;

  if(!dt_contig) {
    MPIDI_msg_sz_t  segment_first;
    MPID_Segment   *segment_ptr;
    segment_ptr = MPIDU_Segment_alloc();
    MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                         MPI_ERR_OTHER, "**nomem","**nomem %s",
                         "Send MPID_Segment_alloc");
    MPIDU_Segment_init(buf, count, datatype, segment_ptr, 0);
    segment_first = 0;
    last          = data_sz;
    REQ_OFI(sreq,pack_buffer) = (char *)MPL_malloc(data_sz);
    MPIR_ERR_CHKANDJUMP1(REQ_OFI(sreq,pack_buffer) == NULL, mpi_errno,
                         MPI_ERR_OTHER, "**nomem","**nomem %s",
                         "Send Pack buffer alloc");
    MPIDU_Segment_pack(segment_ptr,segment_first,&last,
                       REQ_OFI(sreq,pack_buffer));
    MPIDU_Segment_free(segment_ptr);
    send_buf = REQ_OFI(sreq,pack_buffer);
  } else
    REQ_OFI(sreq,pack_buffer) = NULL;

  if(data_sz <= MPIDI_Global.max_buffered_send) {
    FI_RC_RETRY(fi_tinject(G_TXC_TAG(0),send_buf,data_sz,
                           _comm_to_phys(comm,rank,MPIDI_API_TAG),match_bits),tinject);
    send_callback(NULL, sreq);
  } else if(data_sz <= MPIDI_Global.max_send)
    FI_RC_RETRY(fi_tsend(G_TXC_TAG(0),send_buf,
                         data_sz,MPIDI_Global.mr,_comm_to_phys(comm,rank,MPIDI_API_TAG),
                         match_bits,(void *) &(REQ_OFI(sreq,context))),tsend);
  else if(unlikely(1)) {
    MPIDI_Send_control_t ctrl;
    int                  c;
    MPIDI_Hugecntr      *cntr;
    void                *ptr;
    c                             = 1;
    REQ_OFI(sreq,callback) = send_callback_huge;
    MPIR_cc_incr(sreq->cc_ptr, &c);
    ptr = MPIDI_Map_lookup(COMM_OFI(comm)->huge_send_counters,rank);

    if(ptr == MPIDI_MAP_NOT_FOUND) {
      ptr   = MPL_malloc(sizeof(int));
      cntr  = (MPIDI_Hugecntr *) ptr;
      cntr->outstanding = 0;
      cntr->counter     = 0;
      MPIDI_Map_set(COMM_OFI(comm)->huge_send_counters,rank,ptr);
    }

    cntr  = (MPIDI_Hugecntr *) ptr;
    cntr->outstanding++;
    cntr->counter++;
    MPIR_Assert(cntr->outstanding != USHRT_MAX);
    MPIR_Assert(cntr->counter != USHRT_MAX);
    REQ_OFI(sreq,util_comm) = comm;
    REQ_OFI(sreq,util_id)   = rank;
    FI_RC_RETRY(fi_tsend(G_TXC_TAG(0),send_buf,
                         MPIDI_Global.max_send,MPIDI_Global.mr,_comm_to_phys(comm,rank,MPIDI_API_TAG),
                         match_bits,(void *) &(REQ_OFI(sreq,context))),tsend);
    ctrl.type  = MPIDI_CTRL_HUGE;
    ctrl.seqno = cntr->counter-1;
    MPI_RC_POP(do_control_send(&ctrl,send_buf,data_sz,rank,comm,sreq));
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(do_psend)
__SI__ int do_psend(const void     *buf,
                    int             count,
                    MPI_Datatype    datatype,
                    int             rank,
                    int             tag,
                    MPIR_Comm      *comm,
                    int             context_offset,
                    MPIR_Request **request)
{
  BEGIN_FUNC(FCNAME);
  MPIR_Request *sreq;
  REQ_CREATE(sreq);
  *request = sreq;

  MPIR_Comm_add_ref(comm);
  sreq->kind                   = MPIR_REQUEST_KIND__PREQUEST_SEND;
  sreq->comm                   = comm;
  REQ_OFI(sreq,p_buf)          = (void *)buf;
  REQ_OFI(sreq,p_count)        = count;
  REQ_OFI(sreq,datatype)       = datatype;
  REQ_OFI(sreq,p_rank)         = rank;
  REQ_OFI(sreq,p_tag)          = tag;
  REQ_OFI(sreq,util_comm)      = comm;
  REQ_OFI(sreq,util_id)        = comm->context_id+context_offset;
  sreq->u.persist.real_request = NULL;
  MPIDI_Request_complete(sreq);

  if(HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN) {
    MPIR_Datatype  *dt_ptr;
    MPID_Datatype_get_ptr(datatype, dt_ptr);
    MPID_Datatype_add_ref(dt_ptr);
  }

  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}

#define STARTALL_CASE(CASELABEL,FUNC,CONTEXTID) \
  case CASELABEL:                               \
  {                                             \
    rc = FUNC(REQ_OFI(preq,p_buf),              \
              REQ_OFI(preq,p_count),            \
              REQ_OFI(preq,datatype),           \
              REQ_OFI(preq,p_rank),             \
              REQ_OFI(preq,p_tag),              \
              preq->comm,                       \
              REQ_OFI(preq,util_id) -           \
              CONTEXTID,                        \
              &preq->u.persist.real_request);   \
    break;                                      \
  }

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Startall)
__ADI_INLINE__ int MPIDI_Startall(int           count,
                                  MPIR_Request *requests[])
{
  BEGIN_FUNC(FCNAME);
  int rc=MPI_SUCCESS, i;

  for(i = 0; i < count; i++) {
    MPIR_Request *const preq = requests[i];

    switch(REQ_OFI(preq,p_type)) {
      STARTALL_CASE(MPIDI_PTYPE_RECV,MPIDI_Irecv,preq->comm->recvcontext_id);
      STARTALL_CASE(MPIDI_PTYPE_SEND,MPIDI_Isend,preq->comm->context_id);
      STARTALL_CASE(MPIDI_PTYPE_SSEND,MPIDI_Issend,preq->comm->context_id);

    case MPIDI_PTYPE_BSEND: {
      rc = MPIR_Bsend_isend(REQ_OFI(preq,p_buf),
                            REQ_OFI(preq,p_count),
                            REQ_OFI(preq,datatype),
                            REQ_OFI(preq,p_rank),
                            REQ_OFI(preq,p_tag),
                            preq->comm,
                            BSEND_INIT,
                            &preq->u.persist.real_request);

      if(preq->u.persist.real_request != NULL)
        MPIR_Object_add_ref(preq->u.persist.real_request);

      break;
    }

    default:
      rc = MPIR_Err_create_code(MPI_SUCCESS,MPIR_ERR_FATAL,__FUNCTION__,
                                __LINE__,MPI_ERR_INTERN,"**ch3|badreqtype",
                                "**ch3|badreqtype %d",REQ_OFI(preq,p_type));
    }

    if(rc == MPI_SUCCESS) {
      preq->status.MPI_ERROR = MPI_SUCCESS;

      if(REQ_OFI(preq,p_type) == MPIDI_PTYPE_BSEND) {
        preq->cc_ptr = &preq->cc;
        MPIDI_Request_set_completed(preq);
      } else
        preq->cc_ptr = &preq->u.persist.real_request->cc;
    } else {
      preq->u.persist.real_request = NULL;
      preq->status.MPI_ERROR       = rc;
      preq->cc_ptr                 = &preq->cc;
      MPIDI_Request_set_completed(preq);
    }
  }

  END_FUNC(FCNAME);
  return rc;
}

                            __ADI_INLINE__ int MPIDI_Cancel_send(MPIR_Request *sreq)
                            {
                              int mpi_errno = MPI_SUCCESS;
                              BEGIN_FUNC(FCNAME);
                              /* Sends cannot be cancelled */
                              END_FUNC(FCNAME);
                              return mpi_errno;
                            }

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Send)
__ADI_INLINE__ int MPIDI_Send(SENDPARMS)
{
  DO_SEND(1,0ULL);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Rsend)
__ADI_INLINE__ int MPIDI_Rsend(SENDPARMS)
{
  DO_SEND(1,0ULL);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Irsend)
__ADI_INLINE__ int MPIDI_Irsend(SENDPARMS)
{
  DO_SEND(0,0ULL);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Ssend)
__ADI_INLINE__ int MPIDI_Ssend(SENDPARMS)
{
  DO_SEND(0,MPID_SYNC_SEND);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Isend)
__ADI_INLINE__ int MPIDI_Isend(SENDPARMS)
{
  DO_SEND(0,0);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Issend)
__ADI_INLINE__ int MPIDI_Issend(SENDPARMS)
{
  DO_SEND(0,MPID_SYNC_SEND);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Send_init)
__ADI_INLINE__ int MPIDI_Send_init(SENDPARMS)
{
  DO_PSEND(MPIDI_PTYPE_SEND);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Ssend_init)
__ADI_INLINE__ int MPIDI_Ssend_init(SENDPARMS)
{
  DO_PSEND(MPIDI_PTYPE_SSEND);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Bsend_init)
__ADI_INLINE__ int MPIDI_Bsend_init(SENDPARMS)
{
  DO_PSEND(MPIDI_PTYPE_BSEND);
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_Rsend_init)
__ADI_INLINE__ int MPIDI_Rsend_init(SENDPARMS)
{
  DO_PSEND(MPIDI_PTYPE_SEND);
}
