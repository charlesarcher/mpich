#include "transport.h"
#include <assert.h>

/* Collective Plugin Layer Adapter Code */

__CSI__ int TRANSPORTI_t_init()
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_pack(COLL_DT dt)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_unpack(COLL_DT dt)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_opinfo(COLL_OP  operation,
                                int     *is_commutative)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_isinplace(void *buf)
{
  assert(0);
  return 0;
}
__CSI__ int TRANSPORTI_t_dtinfo(COLL_DT  datatype,
                                int     *iscontig,
                                size_t  *out_extent,
                                size_t  *lower_bound)
{
  assert(0);
  return 0;
}

__CSI__ void TRANSPORTI_t_addref_dt(COLL_DT datatype,int up)
{
  assert(0);
}

__CSI__ void TRANSPORTI_t_addref_op(COLL_OP operation, int up)
{
  assert(0);
}

__CSI__ int TRANSPORTI_t_send(const void *buf,
                              int         count,
                              COLL_DT     datatype,
                              int         dest,
                              int         tag,
                              COLL_COMM   comm_ptr,
                              COLL_REQ   *req_ptr)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_recv(void      *buf,
                              int        count,
                              COLL_DT    datatype,
                              int        source,
                              int        tag,
                              COLL_COMM  comm_ptr,
                              COLL_REQ  *req_ptr)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_freefcn(void *data)
{
  assert(0);
  return 0;
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
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_test(int         count,
                              COLL_REQ   *req_ptr)
{
  assert(0);
  return 0;
}

__CSI__ void TRANSPORTI_t_reqalloc(COLL_REQ   *req_ptr)
{
  assert(0);
}

__CSI__ void TRANSPORTI_t_reqcomplete(COLL_REQ   *req_ptr)
{
  assert(0);
}

__CSI__ int TRANSPORTI_t_rank(COLL_COMM  comm_ptr)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_size(COLL_COMM comm_ptr)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_reduce_local(const void *inbuf,
                                      void       *inoutbuf,
                                      int         count,
                                      COLL_DT     datatype,
                                      COLL_OP     operation)
{
  assert(0);
  return 0;
}

__CSI__ int TRANSPORTI_t_dtcopy(void       *tobuf,
                                int         tocount,
                                COLL_DT     totype,
                                const void *frombuf,
                                int         fromcount,
                                COLL_DT     fromtype)
{
  assert(0);
  return 0;
}

__CSI__ void *TRANSPORTI_t_malloc(size_t size)
{
  assert(0);
  return NULL;
}

__CSI__ void TRANSPORTI_t_free(void *ptr)
{
  assert(0);
}
