/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef MPIDOFI_TYPES_H_INCLUDED
#define MPIDOFI_TYPES_H_INCLUDED
#include <rdma/fabric.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_tagged.h>
#include <rdma/fi_rma.h>
#include <rdma/fi_atomic.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_errno.h>
#include <netdb.h>
#include <stddef.h>
#include <inttypes.h>
EXTERN_C_BEGIN

#define TRANSPORT_OBJ MPIDI_Global.transportfuncs
#define COLL_OBJ      MPIDI_Global.collfuncs
#include "coll/coll.h"

#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y))?(x):(y))
#endif

#define COMM_TO_INDEX(comm,rank) COMM_OFI(comm)->vcrt->vcr_table[rank].addr_idx

#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
#define COMM_TO_EP(comm,rank)  COMM_OFI(comm)->vcrt->vcr_table[rank].ep_idx
#define MPIDI_MAX_ENDPOINTS 256
#define MPIDI_MAX_ENDPOINTS_BITS 8
#define G_TXC_TAG(x) MPIDI_Global.ctx[x].tx_tag
#define G_TXC_RMA(x) MPIDI_Global.ctx[x].tx_rma
#define G_TXC_MSG(x) MPIDI_Global.ctx[x].tx_msg
#define G_TXC_CTR(x) MPIDI_Global.ctx[x].tx_ctr
#define G_RXC_TAG(x) MPIDI_Global.ctx[x].rx_tag
#define G_RXC_RMA(x) MPIDI_Global.ctx[x].rx_rma
#define G_RXC_MSG(x) MPIDI_Global.ctx[x].rx_msg
#define G_RXC_CTR(x) MPIDI_Global.ctx[x].rx_ctr
#else
#define COMM_TO_EP(comm,rank) 0
#define MPIDI_MAX_ENDPOINTS 0
#define MPIDI_MAX_ENDPOINTS_BITS 0
#define G_TXC_TAG(x) MPIDI_Global.ep
#define G_TXC_RMA(x) MPIDI_Global.ep
#define G_TXC_MSG(x) MPIDI_Global.ep
#define G_TXC_CTR(x) MPIDI_Global.ep
#define G_RXC_TAG(x) MPIDI_Global.ep
#define G_RXC_RMA(x) MPIDI_Global.ep
#define G_RXC_MSG(x) MPIDI_Global.ep
#define G_RXC_CTR(x) MPIDI_Global.ep
#endif

#ifdef MPIDI_USE_AV_TABLE
#define COMM_TO_PHYS(comm,rank)  ((fi_addr_t)(uintptr_t)COMM_TO_INDEX(comm,rank))
#define TO_PHYS(rank)            ((fi_addr_t)(uintptr_t)rank)
#else
#define COMM_TO_PHYS(comm,rank)  MPIDI_Addr_table->table[COMM_TO_INDEX(comm,rank)].dest
#define TO_PHYS(rank)            MPIDI_Addr_table->table[rank].dest
#endif


typedef int (*progress_func_ptr_t) (int *made_progress);
typedef struct progress_hook_slot {
  progress_func_ptr_t func_ptr;
  int active;
} progress_hook_slot_t;
#define MAX_PROGRESS_HOOKS 8

#define MPIDI_PARENT_PORT_KVSKEY "PARENT_ROOT_PORT_NAME"
#define MPIDI_MAX_KVS_VALUE_LEN  4096
#define MPIDI_MAP_NOT_FOUND      ((void*)(-1UL))

#define MPIDI_FI_MAJOR_VERSION 1
#define MPIDI_FI_MINOR_VERSION 0

/* Macros and inlines */
/* match/ignore bit manipulation
 *
 * 0123 4567 01234567 0123 4567 01234567 0123 4567 01234567 01234567 01234567
 *     |                  |                  |
 * ^   |    context id    |       source     |       message tag
 * |   |                  |                  |
 * +---- protocol
 */
#define MPID_PROTOCOL_MASK (0x9000000000000000ULL)
#define MPID_CONTEXT_MASK  (0x0FFFF00000000000ULL)
#define MPID_SOURCE_MASK   (0x00000FFFF0000000ULL)
#define MPID_TAG_MASK      (0x000000000FFFFFFFULL)
#define MPID_SYNC_SEND     (0x1000000000000000ULL)
#define MPID_SYNC_SEND_ACK (0x2000000000000000ULL)
#define MPID_DYNPROC_SEND  (0x4000000000000000ULL)
#define MPID_TAG_SHIFT     (28)
#define MPID_SOURCE_SHIFT  (16)

#define MPIDI_CACHELINE_SIZE            64
#define MPIR_THREAD_POBJ_UTIL_MUTEX     MPIDI_Global.mutexes[0].m
#define MPIR_THREAD_POBJ_PROGRESS_MUTEX MPIDI_Global.mutexes[1].m
#define MPIR_THREAD_POBJ_FI_MUTEX       MPIDI_Global.mutexes[2].m
#define MPIR_THREAD_POBJ_SPAWN_MUTEX    MPIDI_Global.mutexes[3].m

typedef union {
    MPID_Thread_mutex_t m;
    char cacheline[MPIDI_CACHELINE_SIZE];
} cacheline_mutex_t
__attribute__ ((aligned(MPIDI_CACHELINE_SIZE)));


/* Typedefs */
typedef struct fid_ep            *fid_base_ep_t;
typedef struct fid_ep            *fid_ep_t;
typedef struct fid_fabric        *fid_fabric_t;
typedef struct fid_domain        *fid_domain_t;
typedef struct fid_cq            *fid_cq_t;
typedef struct fid_av            *fid_av_t;
typedef struct fid_mr            *fid_mr_t;
typedef struct fid_cntr          *fid_cntr_t;
typedef struct fi_fabric          fabric_t;
typedef struct fi_msg             msg_t;
typedef struct fi_info            info_t;
typedef struct fi_cq_attr         cq_attr_t;
typedef struct fi_cntr_attr       cntr_attr_t;
typedef struct fi_av_attr         av_attr_t;
typedef struct fi_domain_attr     domain_attr_t;
typedef struct fi_tx_attr         tx_attr_t;
typedef struct fi_rx_attr         rx_attr_t;
typedef struct fi_cq_tagged_entry cq_tagged_entry_t;
typedef struct fi_cq_err_entry    cq_err_entry_t;
typedef struct fi_context         context_t;
typedef struct fi_info_addr       info_addr_t;
typedef struct fi_msg_tagged      msg_tagged_t;
typedef struct iovec              iovec_t;
typedef struct fi_rma_ioc         rma_ioc_t;
typedef struct fi_rma_iov         rma_iov_t;
typedef struct fi_ioc             ioc_t;
typedef struct fi_msg_rma         msg_rma_t;
typedef struct fi_msg_atomic      msg_atomic_t;
typedef enum fi_op                fi_op_t;
typedef enum fi_datatype          fi_datatype_t;
typedef int (*event_callback_fn)(cq_tagged_entry_t *wc, MPIR_Request *);
typedef int (*control_callback_fn)(void *buf);

typedef enum {
  MPIDI_PTYPE_RECV,
  MPIDI_PTYPE_SEND,
  MPIDI_PTYPE_BSEND,
  MPIDI_PTYPE_SSEND,
} MPIDI_ptype;

typedef enum {
  MPIDI_ACCU_ORDER_RAR = 1,
  MPIDI_ACCU_ORDER_RAW = 2,
  MPIDI_ACCU_ORDER_WAR = 4,
  MPIDI_ACCU_ORDER_WAW = 8
} MPIDI_Win_info_accumulate_ordering;

typedef enum {
  MPIDI_ACCU_SAME_OP,
  MPIDI_ACCU_SAME_OP_NO_OP
} MPIDI_Win_info_accumulate_ops;

enum {
  MPID_EPOTYPE_NONE      = 0,       /**< No epoch in affect */
  MPID_EPOTYPE_LOCK      = 1,       /**< MPI_Win_lock access epoch */
  MPID_EPOTYPE_START     = 2,       /**< MPI_Win_start access epoch */
  MPID_EPOTYPE_POST      = 3,       /**< MPI_Win_post exposure epoch */
  MPID_EPOTYPE_FENCE     = 4,       /**< MPI_Win_fence access/exposure epoch */
  MPID_EPOTYPE_REFENCE   = 5,       /**< MPI_Win_fence possible access/exposure epoch */
  MPID_EPOTYPE_LOCK_ALL  = 6,       /**< MPI_Win_lock_all access epoch */
};

enum {
  MPIDI_CTRL_ASSERT,      /**< Lock acknowledge      */
  MPIDI_CTRL_LOCKACK,     /**< Lock acknowledge      */
  MPIDI_CTRL_LOCKALLACK,  /**< Lock all acknowledge  */
  MPIDI_CTRL_LOCKREQ,     /**< Lock window           */
  MPIDI_CTRL_LOCKALLREQ,  /**< Lock all window       */
  MPIDI_CTRL_UNLOCK,      /**< Unlock window         */
  MPIDI_CTRL_UNLOCKACK,   /**< Unlock window         */
  MPIDI_CTRL_UNLOCKALL,   /**< Unlock window         */
  MPIDI_CTRL_UNLOCKALLACK,/**< Unlock window         */
  MPIDI_CTRL_COMPLETE,    /**< End a START epoch     */
  MPIDI_CTRL_POST,        /**< Begin POST epoch      */
  MPIDI_CTRL_HUGE,        /**< Huge message          */
  MPIDI_CTRL_HUGEACK,     /**< Huge message ack      */
  MPIDI_CTRL_HUGE_CLEANUP,/**< Huge message cleanup  */
};

enum {
  MPIDI_REQUEST_LOCK,
  MPIDI_REQUEST_LOCKALL,
};

/* VCR Table Data */
typedef struct MPIDI_VCR {
  unsigned is_local : 1;
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
  unsigned ep_idx   : MPIDI_MAX_ENDPOINTS_BITS;
#endif
  unsigned addr_idx :(31-MPIDI_MAX_ENDPOINTS_BITS);
} MPIDI_VCR;
#define VCR_OFI(vcr)   ((MPIDI_VCR*)(vcr)->pad)

struct MPIDI_VCRT {
  MPIR_OBJECT_HEADER;
  unsigned         size;          /**< Number of entries in the table */
  MPIDI_VCR        vcr_table[0];  /**< Array of virtual connection references */
};
#define MPIDI_VCRT_HDR_SIZE offsetof(struct MPIDI_VCRT, vcr_table)
typedef struct MPIDI_VCRT*  MPID_VCRT;


/* Physical address table data */
typedef struct {
  fi_addr_t dest; /**< A single physical address */
} MPID_Addr_entry_t;
typedef struct {
  int               size;
  MPID_Addr_entry_t table[0];  /**< Array of physical addresses */
} MPIDI_Addr_table_t;

#define MPIDI_REQUEST_HDR_SIZE offsetof(struct MPIR_Request, dev.pad)
typedef struct {
  char              pad[MPIDI_REQUEST_HDR_SIZE];
  context_t         context;  /* fixed field, do not move */
  event_callback_fn callback; /* fixed field, do not move */
} MPIDI_Ctrl_req;

typedef struct {
  char               pad[MPIDI_REQUEST_HDR_SIZE];
  context_t          context;  /* fixed field, do not move */
  event_callback_fn  callback; /* fixed field, do not move */
  MPIR_Request      *signal_req;
} MPIDI_Ssendack_request;

typedef struct {
  char              pad[MPIDI_REQUEST_HDR_SIZE];
  context_t         context;  /* fixed field, do not move */
  event_callback_fn callback; /* fixed field, do not move */
  int               done;
  uint32_t          tag;
  uint32_t          source;
  uint64_t          msglen;
} MPIDI_Dynproc_req;

typedef struct atomic_valid {
  uint8_t  op;
  uint8_t  dt;
  unsigned atomic_valid:2;
  unsigned fetch_atomic_valid:2;
  unsigned compare_atomic_valid:2;
  unsigned dtsize:10;
  uint64_t max_atomic_count;
  uint64_t max_compare_atomic_count;
  uint64_t max_fetch_atomic_count;
} atomic_valid_t;
#define DT_SIZES 62
#define OP_SIZES 15

#define MPIDI_API_TAG 0
#define MPIDI_API_RMA 1
#define MPIDI_API_MSG 2
#define MPIDI_API_CTR 3

typedef struct {
  fid_ep_t tx_tag;
  fid_ep_t rx_tag;

  fid_ep_t tx_rma;
  fid_ep_t rx_rma;

  fid_ep_t tx_msg;
  fid_ep_t rx_msg;

  fid_ep_t tx_ctr;
  fid_ep_t rx_ctr;

  int        ctx_offset;
} MPIDI_Context_t;

/* Global state data */
#define MPIDI_KVSAPPSTRLEN 1024
typedef struct {
  int                  jobid;
  char                 addrname[FI_NAME_MAX];
  size_t               addrnamelen;
  fid_domain_t         domain;
  fid_fabric_t         fabric;
  fid_base_ep_t        ep;
  cacheline_mutex_t    mutexes[4];
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
  MPIDI_Context_t      ctx[MPIDI_MAX_ENDPOINTS];
#endif
  fid_cq_t             p2p_cq;
  fid_cntr_t           rma_ctr;
  fid_mr_t             mr;
  fid_av_t             av;
  iovec_t             *iov;
  msg_t               *msg;
  MPIDI_Ctrl_req      *control_req;
  uint64_t             cntr;
  uint64_t             max_buffered_send;
  uint64_t             max_buffered_write;
  uint64_t             max_send;
  uint64_t             max_write;
  int                  cur_ctrlblock;
  int                  num_ctrlblock;
  uint64_t             lkey;
  int                  control_init;
  control_callback_fn  control_fn[16];
  MPID_Node_id_t      *node_map;
  MPID_Node_id_t       max_node_id;
  void                *win_map;
  void                *comm_map;
  atomic_valid_t       win_op_table[DT_SIZES][OP_SIZES];
  MPIR_Commops         MPIR_Comm_fns_store;
  progress_hook_slot_t progress_hooks[MAX_PROGRESS_HOOKS];
  struct TransportFunc transportfuncs;
  struct CollFunc      collfuncs;
  int                  coll_progress;
  int                  pname_set;
  int                  pname_len;
  char                 kvsname[MPIDI_KVSAPPSTRLEN];
  char                 pname[MPI_MAX_PROCESSOR_NAME];
  int                  port_name_tag_mask[MPIR_MAX_CONTEXT_MASK];
} MPIDI_Global_t;

typedef struct {
  context_t            context;        /* fixed field, do not move */
  event_callback_fn    callback;       /* fixed field, do not move */

  /* Re-usable fields for ssend, probe,persistent */
  int                  util_id;
  MPIR_Comm           *util_comm;

  /* Datatype fields */
  MPI_Datatype         datatype;
  MPID_Segment        *segment_ptr;
  char                *pack_buffer;

  /* persistent send fields */
  MPIDI_ptype          p_type;
  int                  p_rank;
  int                  p_tag;
  int                  p_count;
  void                *p_buf;
} MPIDI_OFIReq_t;
#define REQ_OFI(req,field) (((MPIDI_OFIReq_t*)(req)->dev.pad)->field)

typedef struct {
  MPID_VCRT   vcrt;
  MPID_VCRT   local_vcrt;
  uint32_t    window_instance;
  void       *huge_send_counters; /* Map of per destination send queues      */
  void       *huge_recv_counters; /* Map of per destination receive queues   */
  char        coll_blob[232];
} MPIDI_OFIComm_t;
#define COMM_OFI(comm) ((MPIDI_OFIComm_t*)(comm)->dev.pad)
#define COMMBLOBOFFSET() (offsetof(struct MPIR_Comm,dev.pad) + \
                          offsetof(MPIDI_OFIComm_t,coll_blob))
#define COMMBLOBSIZE() sizeof((MPIDI_OFIComm_t*)0)->coll_blob

typedef struct {
  uint32_t index;
} MPIDI_OFIdt_t;
#define DT_OFI(comm) ((MPIDI_OFIdt_t*)(comm)->dev.pad)

struct MPIDI_Win_lock {
  struct MPIDI_Win_lock *next;
  unsigned               rank;
  uint16_t               mtype;
  uint16_t               type;
};

struct MPIDI_Win_queue {
  struct MPIDI_Win_lock *head;
  struct MPIDI_Win_lock *tail;
};

typedef struct MPIDI_WinLock_info {
  unsigned            peer;
  int                 lock_type;
  struct MPIR_Win    *win;
  volatile unsigned   done;
} MPIDI_WinLock_info;

/* These control structures have to be the same size */
typedef struct {
  int16_t   type;
  int16_t   lock_type;
  int       origin_rank;
  uint64_t  win_id;
  int       dummy[5];
} MPIDI_Win_control_t;
#define MPID_MIN_CTRL_MSG_SZ (sizeof(MPIDI_Win_control_t))

typedef struct {
  int16_t       type;
  int16_t       seqno;
  int           origin_rank;
  char         *send_buf;
  size_t        msgsize;
  int           comm_id;
  int           endpoint_id;
  MPIR_Request *ackreq;
} MPIDI_Send_control_t;

typedef struct {
  MPIR_Datatype *pointer;
  MPI_Datatype    type;
  int             count;
  int             contig;
  MPI_Aint        true_lb;
  MPIDI_msg_sz_t  size;
  int             num_contig;
  DLOOP_VECTOR   *map;
  DLOOP_VECTOR    __map;
} MPIDI_Win_dt;

typedef struct {
  void          *addr;
  void          *result_addr;
  void          *req;
  MPIR_Win      *win;
  MPI_Datatype   type;
  MPI_Op         op;
  int            origin_endpoint;
  size_t         len;
} MPIDI_Win_MsgInfo;

typedef struct MPIDI_dummy {
  MPIR_OBJECT_HEADER;
  void *pad;
} MPIDI_dummy_t;
#define MPIDI_OBJECT_HEADER_SZ offsetof(struct MPIDI_dummy,  pad)

typedef struct MPIDI_Iovec_state {
  uintptr_t  target_base_addr;
  uintptr_t  origin_base_addr;
  uintptr_t  result_base_addr;
  size_t     target_count;
  size_t     origin_count;
  size_t     result_count;
  iovec_t   *target_iov;
  iovec_t   *origin_iov;
  iovec_t   *result_iov;
  size_t     target_idx;
  uintptr_t  target_addr;
  uintptr_t  target_size;
  size_t     origin_idx;
  uintptr_t  origin_addr;
  uintptr_t  origin_size;
  size_t     result_idx;
  uintptr_t  result_addr;
  uintptr_t  result_size;
  size_t     buf_limit;
}MPIDI_Iovec_state_t;

typedef struct MPIDI_Win_noncontig {
  MPIDI_Iovec_state_t       iovs;
  MPIDI_Win_dt              origin_dt;
  MPIDI_Win_dt              result_dt;
  MPIDI_Win_dt              target_dt;
}MPIDI_Win_noncontig;

typedef struct MPIDI_Win_request {
  MPIR_OBJECT_HEADER;
  char                      pad[MPIDI_REQUEST_HDR_SIZE-MPIDI_OBJECT_HEADER_SZ];
  context_t                 context;  /* fixed field, do not move */
  event_callback_fn         callback; /* fixed field, do not move */
  struct MPIDI_Win_request *next;
  int                       target_rank;
  MPIDI_Win_noncontig      *noncontig;
} MPIDI_Win_request;

typedef struct {
  char               pad[MPIDI_REQUEST_HDR_SIZE];
  context_t          context;        /* fixed field, do not move */
  event_callback_fn  callback;       /* fixed field, do not move */
  MPIR_Request      *parent;         /* Parent request           */
} MPIDI_Chunk_request;

typedef struct {
  char                  pad[MPIDI_REQUEST_HDR_SIZE];
  context_t             context;  /* fixed field, do not move */
  event_callback_fn     callback; /* fixed field, do not move */
  event_callback_fn     done_fn;
  MPIDI_Send_control_t  remote_info;
  size_t                cur_offset;
  MPIR_Comm            *comm_ptr;
  MPIR_Request         *localreq;
  cq_tagged_entry_t     wc;
} MPIDI_Huge_chunk_t;

typedef struct {
  uint16_t  seqno;
  void     *chunk_q;
} MPIDI_Huge_recv_t;

typedef struct MPIDI_Win_info_args {
  int                                no_locks;
  MPIDI_Win_info_accumulate_ordering accumulate_ordering;
  MPIDI_Win_info_accumulate_ops      accumulate_ops;
  int                                same_size;
  int                                alloc_shared_noncontig;
} MPIDI_Win_info_args;

typedef struct MPIDI_Win_info {
  void              *base_addr;
  uint32_t           disp_unit;
}  __attribute__((packed)) MPIDI_Win_info;

typedef struct MPIDI_Win_sync_lock {
  struct {
    volatile unsigned locked;
    volatile unsigned allLocked;
  } remote;
  struct {
    struct MPIDI_Win_queue requested;
    int                    type;
    unsigned               count;
  } local;
} MPIDI_Win_sync_lock;

typedef struct MPIDI_Win_sync_pscw {
  struct MPIR_Group *group;
  volatile unsigned  count;
} MPIDI_Win_sync_pscw;

typedef struct MPIDI_Win_sync_t {
  volatile int        origin_epoch_type;
  volatile int        target_epoch_type;
  MPIDI_Win_sync_pscw sc, pw;
  MPIDI_Win_sync_lock lock;
} MPIDI_Win_sync_t;

typedef struct {
  void                *winfo;
  MPIDI_Win_info_args  info_args;
  MPIDI_Win_sync_t     sync;
  uint64_t             win_id;
  void                *mmap_addr;
  int64_t              mmap_sz;
  MPI_Aint            *sizes;
  MPIDI_Win_request   *syncQ;
  void                *msgQ;
  int                  count;
} MPIDI_OFIWin_t;
#define WIN_OFI(win) ((MPIDI_OFIWin_t*)(win)->dev.pad)

/*
 * Helper routines and macros for request completion
 */
#define MPIDI_Request_tls_alloc(req)                                     \
  ({                                                                     \
    (req) = (MPIR_Request*)MPIR_Handle_obj_alloc(&MPIDI_Request_mem);    \
    if (req == NULL)                                                     \
      MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request"); \
  })

#define MPIDI_Request_tls_free(req) \
  MPIR_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Request_alloc_and_init(req,count)         \
  ({                                                    \
    MPIDI_Request_tls_alloc(req);                       \
    MPIR_Assert(req != NULL);                           \
    MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
                == MPIR_REQUEST);                       \
    MPIR_cc_set(&req->cc, 1);                           \
    req->cc_ptr = &req->cc;                             \
    MPIR_Object_set_ref(req, count);                    \
    req->u.ureq.greq_fns          = NULL;               \
    MPIR_STATUS_SET_COUNT(req->status, 0);              \
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);     \
    req->status.MPI_SOURCE    = MPI_UNDEFINED;          \
    req->status.MPI_TAG       = MPI_UNDEFINED;          \
    req->status.MPI_ERROR     = MPI_SUCCESS;            \
    req->comm                 = NULL;                   \
  })

#define MPIDI_Win_request_tls_alloc(req)                                \
  ({                                                                           \
  (req) = (MPIDI_Win_request*)MPIR_Handle_obj_alloc(&MPIDI_Request_mem); \
  if (req == NULL)                                                           \
    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Win Request");   \
  })

#define MPIDI_Win_request_tls_free(req) \
  MPIR_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Win_request_complete(req)                 \
  ({                                                    \
  int count;                                          \
  MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
              == MPIR_REQUEST);                       \
  MPIR_Object_release_ref(req, &count);               \
  MPIR_Assert(count >= 0);                            \
  if (count == 0)                                     \
    {                                                 \
      MPL_free(req->noncontig);                      \
      MPIDI_Win_request_tls_free(req);                \
    }                                                 \
  })

#define MPIDI_Win_request_alloc_and_init(req,count)             \
  ({                                                            \
    MPIDI_Win_request_tls_alloc(req);                           \
    MPIR_Assert(req != NULL);                                   \
    MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle)                \
                == MPIR_REQUEST);                               \
    MPIR_Object_set_ref(req, count);                            \
    memset((char*)req+MPIDI_REQUEST_HDR_SIZE, 0,                \
           sizeof(MPIDI_Win_request)-                           \
           MPIDI_REQUEST_HDR_SIZE);                             \
    req->noncontig = (MPIDI_Win_noncontig*)MPL_calloc(1,sizeof(*(req->noncontig))); \
  })

#define MPIDI_Ssendack_request_tls_alloc(req)                           \
  ({                                                                    \
  (req) = (MPIDI_Ssendack_request*)                                     \
    MPIR_Handle_obj_alloc(&MPIDI_Ssendack_request_mem);                 \
  if (req == NULL)                                                      \
    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1,                              \
               "Cannot allocate Ssendack Request");                     \
  })

#define MPIDI_Ssendack_request_tls_free(req) \
  MPIR_Handle_obj_free(&MPIDI_Ssendack_request_mem, (req))

#define MPIDI_Ssendack_request_alloc_and_init(req)      \
  ({                                                    \
    MPIDI_Ssendack_request_tls_alloc(req);              \
    MPIR_Assert(req != NULL);                           \
    MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle)        \
                == MPID_SSENDACK_REQUEST);              \
  })

#define IS_BUILTIN(_datatype)                           \
  (HANDLE_GET_KIND(_datatype) == HANDLE_KIND_BUILTIN)

#define MPIDI_Datatype_get_info(_count, _datatype,                      \
                                _dt_contig_out, _data_sz_out,           \
                                _dt_ptr, _dt_true_lb)                   \
  ({                                                                    \
    if (IS_BUILTIN(_datatype))                                          \
      {                                                                 \
        (_dt_ptr)        = NULL;                                        \
        (_dt_contig_out) = TRUE;                                        \
        (_dt_true_lb)    = 0;                                           \
        (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *                   \
          MPID_Datatype_get_basic_size(_datatype);                      \
      }                                                                 \
    else                                                                \
      {                                                                 \
        MPID_Datatype_get_ptr((_datatype), (_dt_ptr));                  \
        (_dt_contig_out) = (_dt_ptr)->is_contig;                        \
        (_dt_true_lb)    = (_dt_ptr)->true_lb;                          \
        (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size;  \
      }                                                                 \
  })

#define MPIDI_Datatype_check_contig(_datatype,_dt_contig_out)           \
  ({                                                                    \
    if (IS_BUILTIN(_datatype))                                          \
      {                                                                 \
        (_dt_contig_out) = TRUE;                                        \
      }                                                                 \
    else                                                                \
      {                                                                 \
        MPIR_Datatype *_dt_ptr;                                         \
        MPID_Datatype_get_ptr((_datatype), (_dt_ptr));                  \
        (_dt_contig_out) = (_dt_ptr)->is_contig;                        \
      }                                                                 \
  })

#define MPIDI_Datatype_check_contig_size(_datatype,_count,              \
                                         _dt_contig_out,                \
                                         _data_sz_out)                  \
  ({                                                                    \
    if (IS_BUILTIN(_datatype))                                          \
      {                                                                 \
        (_dt_contig_out) = TRUE;                                        \
        (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *                   \
          MPID_Datatype_get_basic_size(_datatype);                      \
      }                                                                 \
    else                                                                \
      {                                                                 \
        MPIR_Datatype *_dt_ptr;                                         \
        MPID_Datatype_get_ptr((_datatype), (_dt_ptr));                  \
        (_dt_contig_out) = (_dt_ptr)->is_contig;                        \
        (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size;  \
      }                                                                 \
  })

#define MPIDI_Datatype_check_contig_size_lb(_datatype,_count,           \
                                            _dt_contig_out,             \
                                            _data_sz_out,               \
                                            _dt_true_lb)                \
  ({                                                                    \
    if (IS_BUILTIN(_datatype))                                          \
      {                                                                 \
        (_dt_contig_out) = TRUE;                                        \
        (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) *                   \
          MPID_Datatype_get_basic_size(_datatype);                      \
        (_dt_true_lb)    = 0;                                           \
      }                                                                 \
    else                                                                \
      {                                                                 \
        MPIR_Datatype *_dt_ptr;                                         \
        MPID_Datatype_get_ptr((_datatype), (_dt_ptr));                  \
        (_dt_contig_out) = (_dt_ptr)->is_contig;                        \
        (_data_sz_out)   = (MPIDI_msg_sz_t)(_count) * (_dt_ptr)->size;  \
        (_dt_true_lb)    = (_dt_ptr)->true_lb;                          \
      }                                                                 \
  })

#define dtype_add_ref_if_not_builtin(datatype_)                         \
  do {                                                                  \
      if ((datatype_) != MPI_DATATYPE_NULL &&                           \
            HANDLE_GET_KIND((datatype_)) != HANDLE_KIND_BUILTIN)        \
        {                                                               \
         MPIR_Datatype *dtp_ = NULL;                                    \
         MPID_Datatype_get_ptr((datatype_), dtp_);                      \
         MPID_Datatype_add_ref(dtp_);                                   \
         }                                                              \
      } while (0)
#define dtype_release_if_not_builtin(datatype_)                 \
  do {                                                          \
      if ((datatype_) != MPI_DATATYPE_NULL &&                   \
          HANDLE_GET_KIND((datatype_)) != HANDLE_KIND_BUILTIN)  \
        {                                                       \
          MPIR_Datatype *dtp_ = NULL;                           \
          MPID_Datatype_get_ptr((datatype_), dtp_);             \
          MPID_Datatype_release(dtp_);                          \
        }                                                       \
  } while (0)

#define MPIDI_Request_create_null_rreq(rreq_, mpi_errno_, FAIL_)        \
  do {                                                                  \
    MPIDI_Request_create_hook(rreq_);                                   \
    if ((rreq_) != NULL) {                                              \
      MPIR_Object_set_ref((rreq_), 1);                                  \
      MPIR_cc_set(&(rreq_)->cc, 0);                                     \
      (rreq_)->kind = MPIR_REQUEST_KIND__RECV;                          \
      MPIR_Status_set_procnull(&(rreq_)->status);                       \
    }                                                                   \
    else {                                                              \
      MPIR_ERR_SETANDJUMP(mpi_errno_,MPI_ERR_OTHER,"**nomemreq");       \
    }                                                                   \
  } while (0)

#define FI_RC(FUNC,STR)                                         \
  do                                                            \
    {                                                           \
      MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);     \
      ssize_t _ret = FUNC;                                      \
      MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);      \
      MPIR_ERR_CHKANDJUMP4(_ret<0,                              \
                           mpi_errno,                           \
                           MPI_ERR_OTHER,                       \
                           "**ofid_"#STR,                       \
                           "**ofid_"#STR" %s %d %s %s",         \
                           __SHORT_FILE__,                      \
                           __LINE__,                            \
                           FCNAME,                              \
                           fi_strerror(-_ret));                 \
    } while (0)

#define FI_RC_RETRY(FUNC,STR)                                   \
  do                                                            \
    {                                                           \
      ssize_t _ret;                                             \
      do {                                                      \
        MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);   \
        _ret = FUNC;                                            \
        MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);    \
        if(likely(_ret==0)) break;                              \
        MPIR_ERR_CHKANDJUMP4(_ret!=-FI_EAGAIN,                  \
                             mpi_errno,                         \
                             MPI_ERR_OTHER,                     \
                             "**ofid_"#STR,                     \
                             "**ofid_"#STR" %s %d %s %s",       \
                             __SHORT_FILE__,                    \
                             __LINE__,                          \
                             FCNAME,                            \
                             fi_strerror(-_ret));               \
        mpi_errno = MPIDI_Progress_test();                      \
        if(mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);     \
      } while (_ret == -FI_EAGAIN);                             \
    } while (0)

#define FI_RC_RETRY_NOLOCK(FUNC,STR)                            \
  do                                                            \
    {                                                           \
      ssize_t _ret;                                             \
      do {                                                      \
	      _ret = FUNC;                                            \
        if(likely(_ret==0)) break;                              \
        MPIR_ERR_CHKANDJUMP4(_ret!=-FI_EAGAIN,                  \
                             mpi_errno,                         \
                             MPI_ERR_OTHER,                     \
                             "**ofid_"#STR,                     \
                             "**ofid_"#STR" %s %d %s %s",       \
                             __SHORT_FILE__,                    \
                             __LINE__,                          \
                             FCNAME,                            \
                             fi_strerror(-_ret));               \
        mpi_errno = MPIDI_Progress_test();                      \
        if(mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);     \
      } while (_ret == -FI_EAGAIN);                             \
    } while (0)

#define PMI_RC(FUNC,STR)                                        \
  do                                                            \
    {                                                           \
      pmi_errno  = FUNC;                                        \
      MPIR_ERR_CHKANDJUMP4(pmi_errno!=PMI_SUCCESS,              \
                           mpi_errno,                           \
                           MPI_ERR_OTHER,                       \
                           "**ofid_"#STR,                       \
                           "**ofid_"#STR" %s %d %s %s",         \
                           __SHORT_FILE__,                      \
                           __LINE__,                            \
                           FCNAME,                              \
                           #STR);                               \
    } while (0)

#define MPI_RC_POP(FUNC)                                        \
  do                                                            \
    {                                                           \
      mpi_errno = FUNC;                                         \
      if(mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);       \
    } while (0)

#define MPI_STR_RC(FUNC,STR)                                    \
  do                                                            \
    {                                                           \
      str_errno = FUNC;                                         \
      MPIR_ERR_CHKANDJUMP4(str_errno!=MPL_STR_SUCCESS,          \
                           mpi_errno,                           \
                           MPI_ERR_OTHER,                       \
                           "**"#STR,                            \
                           "**"#STR" %s %d %s %s",              \
                           __SHORT_FILE__,                      \
                           __LINE__,                            \
                           FCNAME,                              \
                           #STR);                               \
    } while (0)

#define REQ_CREATE(req)                         \
    ({                                          \
      MPIDI_Request_alloc_and_init(req,2);      \
    })


#define WINREQ_CREATE(req)                     \
  ({                                           \
    MPIDI_Win_request_alloc_and_init(req,1);   \
  })

#define SSENDACKREQ_CREATE(req)            \
  ({                                       \
    MPIDI_Ssendack_request_tls_alloc(req); \
  })

#define MPID_LKEY_START 16384

#define PROGRESS()                                    \
  ({                                                  \
  mpi_errno = MPIDI_Progress_test();                  \
  if(mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno); \
  })

#define PROGRESS_WHILE(cond)                          \
  ({                                                  \
  while(cond)                                         \
       PROGRESS();                                    \
  })


extern MPIDI_Addr_table_t    *MPIDI_Addr_table;
extern MPIDI_Global_t         MPIDI_Global;
extern MPIR_Object_alloc_t    MPIDI_Request_mem;
extern MPIR_Object_alloc_t    MPIDI_Ssendack_request_mem;

extern MPIR_Request           MPIDI_Request_direct[];
extern MPIDI_Ssendack_request MPIDI_Ssendack_request_direct[];

/* Utility functions */
extern int   MPIDI_VCRT_Create(int size, struct MPIDI_VCRT **vcrt_ptr);
extern int   MPIDI_VCRT_Release(struct MPIDI_VCRT* vcrt);
extern void  MPIDI_Map_create(void **map);
extern void  MPIDI_Map_destroy(void *map);
extern void  MPIDI_Map_set(void *_map, uint64_t id, void *val);
extern void  MPIDI_Map_erase(void *_map, uint64_t id);
extern void *MPIDI_Map_lookup(void *_map,uint64_t id);

extern int   MPIDI_Gethuge_callback(cq_tagged_entry_t *wc,
                                    MPIR_Request      *req);
extern int  MPIDI_control_callback(void *buf);
extern int  MPIDI_control_assert(void *buf);
extern void MPIDI_build_nodemap(uint32_t       *in_nodeids,
                                MPID_Node_id_t *out_nodemap,
                                int             sz,
                                MPID_Node_id_t *sz_out);
extern int  MPIDI_Control_dispatch(cq_tagged_entry_t *wc,
                                   MPIR_Request      *req)__attribute__((noinline));
extern void MPIDI_Index_datatypes();

/* Prototypes for inliner */
extern int MPIR_Datatype_init_names(void);
extern int MPIR_Allgather_impl(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                               void *recvbuf, int recvcount, MPI_Datatype recvtype,
                               MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern int MPIR_Barrier_impl(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern int MPIR_Comm_commit(MPIR_Comm *);
extern int MPIR_Comm_split_impl(MPIR_Comm *comm_ptr, int color, int key, MPIR_Comm **newcomm_ptr);
extern int MPIR_Allreduce_impl(const void *sendbuf, void *recvbuf, int count,
                               MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern int MPIR_Bcast_impl(void *buffer, int count, MPI_Datatype datatype, int
                           root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern int MPIR_Localcopy(const void *sendbuf, MPI_Aint sendcount, MPI_Datatype sendtype,
                          void *recvbuf, MPI_Aint recvcount, MPI_Datatype recvtype);
extern int MPIR_Info_set_impl(MPIR_Info *info_ptr, const char *key, const char *value);
extern int MPIR_Bcast_intra(void *buffer, int count, MPI_Datatype datatype, int
                            root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern int MPIDU_Sched_progress(int *made_progress);
extern int MPIR_Comm_create(MPIR_Comm **);
extern int MPIR_Comm_dup_impl(MPIR_Comm *comm_ptr, MPIR_Comm **newcomm_ptr);
extern int MPIR_Comm_free_impl(MPIR_Comm *comm_ptr);

/* Common Utility functions used by the
 * C and C++ components
 */
__SI__ fi_addr_t _comm_to_phys(MPIR_Comm *comm,
                               int        rank,
                               int        ep_family)
{
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
  int ep_num = COMM_TO_EP(comm,rank);
  int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
  int rx_idx = offset + ep_family;
  return fi_rx_addr(COMM_TO_PHYS(comm,rank),rx_idx,MPIDI_MAX_ENDPOINTS_BITS);
#else
  return COMM_TO_PHYS(comm,rank);
#endif
}

__SI__ fi_addr_t _to_phys(int        rank,
                          int        ep_family)
{
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
  int ep_num = 0;
  int offset = MPIDI_Global.ctx[ep_num].ctx_offset;
  int rx_idx = offset + ep_family;
  return fi_rx_addr(TO_PHYS(rank),rx_idx,MPIDI_MAX_ENDPOINTS_BITS);
#else
  return TO_PHYS(rank);
#endif
}

__SI__ bool is_tag_sync(uint64_t match_bits)
{
  return (0 != (MPID_SYNC_SEND & match_bits));
}

__SI__ uint64_t init_sendtag(MPIR_Context_id_t contextid,
                             int               source,
                             int               tag,
                             uint64_t          type)
{
  uint64_t  match_bits;
  match_bits = contextid;
  match_bits = (match_bits << MPID_SOURCE_SHIFT);
  match_bits |= source;
  match_bits = (match_bits << MPID_TAG_SHIFT);
  match_bits |= (MPID_TAG_MASK & tag) | type;
  return match_bits;
}
/* receive posting */
__SI__ uint64_t init_recvtag(uint64_t          *mask_bits,
                             MPIR_Context_id_t  contextid,
                             int                source,
                             int                tag)
{
  uint64_t match_bits = 0;
  *mask_bits  = MPID_PROTOCOL_MASK;
  match_bits = contextid;
  match_bits = (match_bits << MPID_SOURCE_SHIFT);

  if(MPI_ANY_SOURCE == source) {
    match_bits = (match_bits << MPID_TAG_SHIFT);
    *mask_bits |= MPID_SOURCE_MASK;
  } else {
    match_bits |= source;
    match_bits = (match_bits << MPID_TAG_SHIFT);
  }

  if(MPI_ANY_TAG == tag) *mask_bits |= MPID_TAG_MASK;
  else match_bits |= (MPID_TAG_MASK & tag);

  return match_bits;
}

__SI__ int get_tag(uint64_t match_bits)
{
  return ((int)(match_bits & MPID_TAG_MASK));
}
__SI__ int get_source(uint64_t match_bits)
{
  return ((int)((match_bits & MPID_SOURCE_MASK) >> MPID_TAG_SHIFT));
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_control)
__SI__ int do_control_win(MPIDI_Win_control_t *control,
                          int                  rank,
                          MPIR_Win            *win,
                          int                  use_comm)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  ADI_COMPILE_TIME_ASSERT(MPID_MIN_CTRL_MSG_SZ ==
                          sizeof(MPIDI_Send_control_t));

  control->win_id      = WIN_OFI(win)->win_id;
  control->origin_rank = win->comm_ptr->rank;

  MPIR_Assert(sizeof(*control)<=MPIDI_Global.max_buffered_send);
  /* Should already be holding a lock, so call the non-locking version*/
  FI_RC_RETRY_NOLOCK(fi_inject(G_TXC_MSG(0),
                     control,sizeof(*control),
                     use_comm?_comm_to_phys(win->comm_ptr,rank,MPIDI_API_MSG):
                     _to_phys(rank,MPIDI_API_MSG)),inject);
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_control)
__SI__ int do_control_send(MPIDI_Send_control_t *control,
                           char                 *send_buf,
                           size_t                msgsize,
                           int                   rank,
                           MPIR_Comm            *comm_ptr,
                           MPIR_Request         *ackreq)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  control->origin_rank = comm_ptr->rank;
  control->send_buf    = send_buf;
  control->msgsize     = msgsize;
  control->comm_id     = comm_ptr->context_id;
  control->endpoint_id = COMM_TO_EP(comm_ptr,comm_ptr->rank);
  control->ackreq      = ackreq;
  MPIR_Assert(sizeof(*control)<=MPIDI_Global.max_buffered_send);
  FI_RC_RETRY(fi_inject(G_TXC_MSG(0),
                  control,sizeof(*control),
                  _comm_to_phys(comm_ptr,rank,MPIDI_API_MSG)),inject);
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

EXTERN_C_END

#include "coll/coll_loader.h"

#endif /* MPIDOFI_TYPES_H_INCLUDED */
