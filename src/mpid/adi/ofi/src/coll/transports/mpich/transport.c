#include "transport.h"
void MPIR_Grequest_complete_impl(MPIR_Request *request_ptr);
void MPIR_Type_get_true_extent_impl(MPI_Datatype datatype, MPI_Aint *true_lb, MPI_Aint *true_extent);
/* Collective Plugin Layer Adapter Code */

__CSI__ int TRANSPORTI_t_init()
{
  return 0;
}

__CSI__ int TRANSPORTI_t_pack(COLL_DT dt)
{
  MPIR_Assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_unpack(COLL_DT dt)
{
  MPIR_Assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_opinfo(COLL_OP  operation,
                                int     *is_commutative)
{
  MPI_Op op = (MPI_Op)(uint64_t)operation;
  MPIR_Op *op_ptr;

  if(HANDLE_GET_KIND(op) == HANDLE_KIND_BUILTIN)
    *is_commutative=1;
  else {
    MPIR_Op_get_ptr(op, op_ptr);

    if(op_ptr->kind == MPIR_OP_KIND__USER_NONCOMMUTE)
      *is_commutative = 0;
    else
      *is_commutative = 1 ;
  }

  return 0;
}

__CSI__ int TRANSPORTI_t_isinplace(void *buf)
{
  if(buf == MPI_IN_PLACE) return 1;

  return 0;
}
__CSI__ int TRANSPORTI_t_dtinfo(COLL_DT  datatype,
                                int     *iscontig,
                                size_t  *out_extent,
                                size_t  *lower_bound)
{
  MPI_Aint true_lb,extent,true_extent;
  MPI_Datatype dt = (MPI_Datatype)(uint64_t)datatype;
  MPIR_Type_get_true_extent_impl(dt,&true_lb,&true_extent);
  MPID_Datatype_get_extent_macro(dt,extent);

  MPIDI_Datatype_check_contig(dt,*iscontig);
  *out_extent  = MPL_MAX(extent,true_extent);
  *lower_bound = true_lb;
  return 0;
}

__CSI__ void TRANSPORTI_t_addref_dt(COLL_DT datatype,int up)
{
  MPI_Datatype dt = (MPI_Datatype)(uint64_t)datatype;
  MPIR_Datatype *dt_ptr;

  if(HANDLE_GET_KIND(dt) != HANDLE_KIND_BUILTIN) {
    MPID_Datatype_get_ptr(dt, dt_ptr);

    if(up)
      MPID_Datatype_add_ref(dt_ptr);
    else
      MPID_Datatype_release(dt_ptr);
  }
}

__CSI__ void TRANSPORTI_t_addref_op(COLL_OP operation, int up)
{
  MPI_Op       op = (MPI_Op)(uint64_t)operation;
  MPIR_Op     *op_ptr;

  if(HANDLE_GET_KIND(op) != HANDLE_KIND_BUILTIN) {
    MPIR_Op_get_ptr(op, op_ptr);

    if(up)
      MPIR_Op_add_ref(op_ptr);
    else
      MPIR_Op_release(op_ptr);
  }
}

__CSI__ int TRANSPORTI_t_send(const void *buf,
                              int         count,
                              COLL_DT     datatype,
                              int         dest,
                              int         tag,
                              COLL_COMM   comm_ptr,
                              COLL_REQ   *req_ptr)
{
  MPIR_Comm      *mpi_comm = (MPIR_Comm *)comm_ptr;
  MPI_Request    *mpi_req  = (MPI_Request *)req_ptr;
  MPIR_Request   *request_ptr;
  MPIR_Errflag_t  errflag  = MPIR_ERR_NONE;
  int             rc       = MPIC_Isend(buf,count,(MPI_Datatype)(uint64_t)datatype,
                                        dest,tag,mpi_comm,
                                        &request_ptr,
                                        &errflag);
  MPIR_OBJ_PUBLISH_HANDLE(mpi_req[0], request_ptr->handle);
  mpi_req[1] = MPI_REQUEST_NULL;
  return rc;
}

__CSI__ int TRANSPORTI_t_recv(void      *buf,
                              int        count,
                              COLL_DT    datatype,
                              int        source,
                              int        tag,
                              COLL_COMM  comm_ptr,
                              COLL_REQ  *req_ptr)
{
  MPIR_Comm      *mpi_comm = (MPIR_Comm *)comm_ptr;
  MPI_Request    *mpi_req  = (MPI_Request *)req_ptr;
  MPIR_Request   *request_ptr;
  int             rc       = MPIC_Irecv(buf,
                                        count,
                                        (MPI_Datatype)(uint64_t)datatype,
                                        source,
                                        tag,
                                        mpi_comm,
                                        &request_ptr);
  MPIR_OBJ_PUBLISH_HANDLE(mpi_req[0], request_ptr->handle);
  mpi_req[1] = MPI_REQUEST_NULL;
  return rc;
}

typedef struct TRANSPORTI_t_recvreduce_t {
  void      *inbuf;
  void      *inoutbuf;
  int        count;
  COLL_DT    datatype;
  COLL_OP    op;
  int        source;
  int        tag;
  COLL_COMM  comm_ptr;
  COLL_REQ  *req_ptr;
  int        done;
} TRANSPORTI_t_recvreduce_t;

__CSI__ int TRANSPORTI_t_freefcn(void *data)
{
  TRANSPORTI_t_recvreduce_t *rr       = (TRANSPORTI_t_recvreduce_t *)data;
  MPL_free(rr);
  return 0;
}

__CSI__ int TRANSPORTI_t_queryfcn(void *data, MPI_Status *status)
{
  TRANSPORTI_t_recvreduce_t *rr       = (TRANSPORTI_t_recvreduce_t *)data;
  MPI_Request             *mpi_req = (MPI_Request *)rr->req_ptr;

  if(mpi_req[0] == MPI_REQUEST_NULL && !rr->done) {
    MPIR_Request            *mpid_req;
    MPIR_Request_get_ptr(mpi_req[1],mpid_req);
    MPI_Datatype dt = (MPI_Datatype)(uint64_t)rr->datatype;
    MPI_Op       op = (MPI_Op)(uint64_t)rr->op;
    MPIR_Reduce_local_impl(rr->inbuf,rr->inoutbuf,rr->count,dt,op);
    MPL_free(rr->inbuf);
    MPIR_Grequest_complete_impl(mpid_req);
    rr->done = 1;
  }

  status->MPI_SOURCE = MPI_UNDEFINED;
  status->MPI_TAG    = MPI_UNDEFINED;
  MPI_Status_set_cancelled(status, 0);
  MPI_Status_set_elements(status, MPI_BYTE, 0);
  return MPI_SUCCESS;
}

__CSI__ int TRANSPORTI_t_recvreduce(void      *buf,
                                    int        count,
                                    COLL_DT    datatype,
                                    COLL_OP    op,
                                    int        source,
                                    int        tag,
                                    COLL_COMM  comm_ptr,
                                    COLL_REQ  *req_ptr)
{
  MPIR_Comm              *mpi_comm    = (MPIR_Comm *)comm_ptr;
  MPI_Request            *mpi_req     = (MPI_Request *)req_ptr;
  TRANSPORTI_t_recvreduce_t *state = (TRANSPORTI_t_recvreduce_t *)MPL_malloc(sizeof(*state));
  MPIR_Request           *request_ptr;
  int                     iscontig, rc;
  size_t                  out_extent, lower_bound;

  TRANSPORTI_t_dtinfo(datatype,&iscontig,&out_extent,&lower_bound);
  state->inbuf    = MPL_malloc(count*out_extent);
  state->inoutbuf = buf;
  state->count    = count;
  state->datatype = datatype;
  state->op       = op;
  state->source   = source;
  state->tag      = tag;
  state->comm_ptr = comm_ptr;
  state->req_ptr  = req_ptr;
  state->done     = 0;

  rc       = MPIC_Irecv(state->inbuf,
                        count,
                        (MPI_Datatype)(uint64_t)datatype,
                        source,
                        tag,
                        mpi_comm,
                        &request_ptr);
  MPIR_OBJ_PUBLISH_HANDLE(mpi_req[0], request_ptr->handle);
  MPIR_Grequest_start_impl(TRANSPORTI_t_queryfcn,TRANSPORTI_t_freefcn,
                           NULL,state,&request_ptr);
  MPIR_OBJ_PUBLISH_HANDLE(mpi_req[1], request_ptr->handle);

  return rc;
}

__CSI__ int TRANSPORTI_t_test(int         count,
                              COLL_REQ   *req_ptr)
{
  MPI_Request  *mpi_req  = (MPI_Request *)req_ptr;
  MPIR_Request *mpid_req;
  int outcount           = 0;
  int realcount          = 2*count;
  int i;

  for(i=0; i<realcount; i++) {
    MPIR_Request_get_ptr(mpi_req[i], mpid_req);

    if(mpi_req[i] == MPI_REQUEST_NULL)
      outcount++;
    else {
      MPI_Status status;

      if(mpid_req->u.ureq.greq_fns != NULL &&
         mpid_req->u.ureq.greq_fns->query_fn != NULL)
        (mpid_req->u.ureq.greq_fns->query_fn)
                (mpid_req->u.ureq.greq_fns->grequest_extra_state, &status);

      if(MPIR_Request_is_complete(mpid_req)) {
        int        active_flag=1;
        outcount++;
        MPIR_Request_complete(&mpi_req[i], mpid_req, &status,&active_flag);
        mpi_req[i] = MPI_REQUEST_NULL;
      }
    }
  }

  MPIDI_Progress_test();

  if(count == 0 || outcount == realcount)
    return 1;
  else
    return 0;
}

__CSI__ void TRANSPORTI_t_reqalloc(COLL_REQ   *req_ptr)
{
  MPIR_Request *mpid_req;
  REQ_CREATE(mpid_req);
  mpid_req->kind = MPIR_REQUEST_KIND__COLL;
  *req_ptr       = (COLL_REQ)mpid_req;
}

__CSI__ void TRANSPORTI_t_reqcomplete(COLL_REQ   *req_ptr)
{
  MPIR_Request *mpid_req  = (MPIR_Request *)*req_ptr;
  int c;
  MPIR_cc_decr(mpid_req->cc_ptr, &c);
  MPIR_Assert(c >= 0);

  if(c == 0) {
    MPIDI_Request_free_hook(mpid_req);
  }
}

__CSI__ int TRANSPORTI_t_rank(COLL_COMM  comm_ptr)
{
  MPIR_Comm *mpi_comm = (MPIR_Comm *)comm_ptr;
  return mpi_comm->rank;
}

__CSI__ int TRANSPORTI_t_size(COLL_COMM comm_ptr)
{
  MPIR_Comm *mpi_comm = (MPIR_Comm *)comm_ptr;
  return mpi_comm->local_size;
}

__CSI__ int TRANSPORTI_t_reduce_local(const void *inbuf,
                                      void       *inoutbuf,
                                      int         count,
                                      COLL_DT     datatype,
                                      COLL_OP     operation)
{
  MPI_Datatype dt = (MPI_Datatype)(uint64_t)datatype;
  MPI_Op       op = (MPI_Op)(uint64_t)operation;
  return MPIR_Reduce_local_impl(inbuf,inoutbuf,count,dt,op);
}

__CSI__ int TRANSPORTI_t_dtcopy(void       *tobuf,
                                int         tocount,
                                COLL_DT     totype,
                                const void *frombuf,
                                int         fromcount,
                                COLL_DT     fromtype)
{
  MPI_Datatype tdt = (MPI_Datatype)(uint64_t)totype;
  MPI_Datatype fdt = (MPI_Datatype)(uint64_t)fromtype;
  return MPIR_Localcopy(frombuf,   /* yes, parameters are reversed        */
                        fromcount, /* MPICH forgot what memcpy looks like */
                        fdt,
                        tobuf,
                        tocount,
                        tdt);
}

__CSI__ void *TRANSPORTI_t_malloc(size_t size)
{
  return MPL_malloc(size);
}

__CSI__ void TRANSPORTI_t_free(void *ptr)
{
  MPL_free(ptr);
}
