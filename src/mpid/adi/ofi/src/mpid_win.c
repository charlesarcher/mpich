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
#include "mpid_iovec_util.h"
#include <opa_primitives.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#define MPIDI_ATOMIC_MAX 512

#ifdef HAVE_ERROR_CHECKING
#define EPOCH_CHECK1()                                                  \
  ({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                            \
    if(WIN_OFI(win)->sync.origin_epoch_type == WIN_OFI(win)->sync.target_epoch_type && \
       WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_REFENCE)    \
    {                                                                   \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_FENCE;        \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_FENCE;        \
    }                                                                   \
    if(WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_NONE ||     \
       WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_POST)       \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                  \
                          goto fn_fail, "**rmasync");                   \
    MPID_END_ERROR_CHECKS;                                              \
  })

#define EPOCH_CHECK2()                                                  \
  ({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                            \
    if(WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_NONE &&     \
       WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_REFENCE)    \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                  \
                          goto fn_fail, "**rmasync");                   \
  })

#define EPOCH_START_CHECK()                                             \
  ({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                            \
    if (WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_START &&   \
        !MPIDI_valid_group_rank(COMM_TO_INDEX(win->comm_ptr,target_rank), \
                                WIN_OFI(win)->sync.sc.group))           \
      MPIR_ERR_SETANDSTMT(mpi_errno,                                    \
                          MPI_ERR_RMA_SYNC,                             \
                          goto fn_fail,                                 \
                          "**rmasync");                                 \
    MPID_END_ERROR_CHECKS;                                              \
  })

#define EPOCH_FENCE_CHECK()                                             \
  ({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                            \
    if(WIN_OFI(win)->sync.origin_epoch_type != WIN_OFI(win)->sync.target_epoch_type) \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                  \
                          goto fn_fail, "**rmasync");                   \
    if (!(massert & MPI_MODE_NOPRECEDE) &&                              \
        WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_FENCE &&   \
        WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_REFENCE && \
        WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_NONE)      \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                  \
                          goto fn_fail, "**rmasync");                   \
    MPID_END_ERROR_CHECKS;                                              \
  })

#define EPOCH_POST_CHECK()                                              \
  ({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                            \
    if(WIN_OFI(win)->sync.target_epoch_type != MPID_EPOTYPE_NONE &&     \
       WIN_OFI(win)->sync.target_epoch_type != MPID_EPOTYPE_REFENCE)    \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                  \
                          goto fn_fail, "**rmasync");                   \
    MPID_END_ERROR_CHECKS;                                              \
  })

#define EPOCH_LOCK_CHECK()                                              \
  ({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                            \
    if((WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_LOCK) &&   \
       (WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_LOCK_ALL)) \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,                  \
                          goto fn_fail, "**rmasync");                   \
    MPID_END_ERROR_CHECKS;                                              \
  })

#define EPOCH_FREE_CHECK()                                              \
  ({                                                                    \
    MPID_BEGIN_ERROR_CHECKS;                                            \
    if(WIN_OFI(win)->sync.origin_epoch_type != WIN_OFI(win)->sync.target_epoch_type || \
       (WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_NONE &&    \
        WIN_OFI(win)->sync.origin_epoch_type != MPID_EPOTYPE_REFENCE))  \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC, return mpi_errno, "**rmasync"); \
    MPID_END_ERROR_CHECKS;                                              \
  })

#define EPOCH_ORIGIN_CHECK(epoch_type)                      \
  ({                                                        \
    MPID_BEGIN_ERROR_CHECKS;                                \
    if(WIN_OFI(win)->sync.origin_epoch_type != epoch_type)  \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,      \
                          return mpi_errno, "**rmasync");   \
    MPID_END_ERROR_CHECKS;                                  \
  })

#define EPOCH_TARGET_CHECK(epoch_type)                      \
  ({                                                        \
    MPID_BEGIN_ERROR_CHECKS;                                \
    if(WIN_OFI(win)->sync.target_epoch_type != epoch_type)  \
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,      \
                          return mpi_errno, "**rmasync");   \
    MPID_END_ERROR_CHECKS;                                  \
  })

#else /* HAVE_ERROR_CHECKING */
#define EPOCH_CHECK1()
#define EPOCH_CHECK2()
#define EPOCH_START_CHECK()
#define EPOCH_FENCE_CHECK()
#define EPOCH_POST_CHECK()
#define EPOCH_LOCK_CHECK()
#define EPOCH_FREE_CHECK()
#define EPOCH_ORIGIN_CHECK(epoch_type)
#define EPOCH_TARGET_CHECK(epoch_type)
#endif /* HAVE_ERROR_CHECKING */

#define EPOCH_FENCE_EVENT()                                         \
  ({                                                                \
    if(massert & MPI_MODE_NOSUCCEED)                                \
    {                                                               \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;     \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;     \
    }                                                               \
    else                                                            \
    {                                                               \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_REFENCE;  \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_REFENCE;  \
    }                                                               \
  })

#define EPOCH_TARGET_EVENT()                                            \
  ({                                                                    \
    if(WIN_OFI(win)->sync.target_epoch_type == MPID_EPOTYPE_REFENCE)    \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_REFENCE;      \
    else                                                                \
      WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;         \
  })

#define EPOCH_ORIGIN_EVENT()                                            \
  ({                                                                    \
    if(WIN_OFI(win)->sync.origin_epoch_type == MPID_EPOTYPE_REFENCE)    \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_REFENCE;      \
    else                                                                \
      WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;         \
  })


#define WINFO(w,rank)                                       \
  ({                                                        \
    void *_p;                                               \
    _p = &(((MPIDI_Win_info*) WIN_OFI(w)->winfo)[rank]);    \
    _p;                                                     \
  })

#define WINFO_BASE(w,rank)                                          \
  ({                                                                \
    void *_p;                                                       \
    _p = (((MPIDI_Win_info*) WIN_OFI(w)->winfo)[rank]).base_addr;   \
    _p;                                                             \
  })

#define WINFO_DISP_UNIT(w,rank)                                     \
  ({                                                                \
    uint32_t _v;                                                    \
    _v = (((MPIDI_Win_info*) WIN_OFI(w)->winfo)[rank]).disp_unit;   \
    _v;                                                             \
  })

#define WINFO_MR_KEY(w,rank)                    \
  ({                                            \
    uint64_t _v;                                \
    _v = MPIDI_Global.lkey;                     \
    _v;                                         \
  })

#define SETUP_SIGNAL_REQUEST()                              \
  ({                                                        \
    if(signal)                                              \
    {                                                       \
      REQ_CREATE(sigreq);                                   \
      sigreq->kind             = MPIR_REQUEST_KIND__RMA;    \
      MPIR_cc_set(sigreq->cc_ptr, 0);                       \
      flags                    = FI_COMPLETION;             \
      *signal                  = sigreq;                    \
      ep                       = G_TXC_RMA(0);              \
    }                                                       \
    else ep = G_TXC_CTR(0);                                 \
  })

#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
#define CONDITIONAL_GLOBAL_CNTR_INCR
#else
#define CONDITIONAL_GLOBAL_CNTR_INCR MPIDI_Global.cntr++
#endif

#define SETUP_CHUNK_CONTEXT()                               \
  ({                                                        \
    if(signal)                                              \
    {                                                       \
      int tmp;                                              \
      MPIDI_Chunk_request *creq;                            \
      MPIR_cc_incr(sigreq->cc_ptr, &tmp);                   \
      creq=(MPIDI_Chunk_request*)MPL_malloc(sizeof(*creq)); \
      creq->callback = chunk_done_callback;                 \
      creq->parent   = sigreq;                              \
      msg.context    = &creq->context;                      \
      CONDITIONAL_GLOBAL_CNTR_INCR;                         \
    }                                                       \
    else MPIDI_Global.cntr++;                               \
  })

#define GET_BASIC_TYPE(a,b)                     \
  ({                                            \
    if (MPIR_DATATYPE_IS_PREDEFINED(a))         \
      b = a;                                    \
    else {                                      \
      MPIR_Datatype *dt_ptr;                    \
      MPID_Datatype_get_ptr(a,dt_ptr);          \
      b = dt_ptr->basic_type;                   \
    }                                           \
  })

/*  Prototypes */
#define MPIDI_QUERY_ATOMIC_COUNT 0
#define MPIDI_QUERY_FETCH_ATOMIC_COUNT 1
#define MPIDI_QUERY_COMPARE_ATOMIC_COUNT 2

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Query_dt)
__SI__ int MPIDI_Query_dt(MPI_Datatype   dt,
                          fi_datatype_t *fi_dt,
                          MPI_Op         op,
                          fi_op_t       *fi_op,
                          size_t        *count,
                          size_t        *dtsize)
{
  BEGIN_FUNC(FCNAME);
  MPIR_Datatype *dt_ptr;
  int            op_index,dt_index, rc;

  MPID_Datatype_get_ptr(dt,dt_ptr);

  /* OP_NULL is the oddball                          */
  /* todo...change configure to table this correctly */
  dt_index = DT_OFI(dt_ptr)->index;

  if(op == MPI_OP_NULL) op_index = 14;
  else op_index = (0x000000FFU & op) - 1;

  *fi_dt  = (fi_datatype_t)MPIDI_Global.win_op_table[dt_index][op_index].dt;
  *fi_op  = (fi_op_t)MPIDI_Global.win_op_table[dt_index][op_index].op;
  *dtsize = MPIDI_Global.win_op_table[dt_index][op_index].dtsize;
  if(*count == MPIDI_QUERY_ATOMIC_COUNT)
    *count = MPIDI_Global.win_op_table[dt_index][op_index].max_atomic_count;
  if(*count == MPIDI_QUERY_FETCH_ATOMIC_COUNT)
    *count = MPIDI_Global.win_op_table[dt_index][op_index].max_fetch_atomic_count;
  if(*count == MPIDI_QUERY_COMPARE_ATOMIC_COUNT)
    *count = MPIDI_Global.win_op_table[dt_index][op_index].max_compare_atomic_count;

  if(*fi_dt==-1 || *fi_op==-1)
    rc = -1;
  else
    rc = MPI_SUCCESS;

  END_FUNC(FCNAME);
  return rc;
}

#undef FCNAME
#define FCNAME DECL_FUNC(get_mapsize)
__SI__ size_t get_mapsize(size_t size,
                          size_t *psz)
{
  long    page_sz = sysconf(_SC_PAGESIZE);
  size_t  mapsize = (size + (page_sz-1))&(~(page_sz-1));
  *psz            = page_sz;
  return mapsize;
}

#undef FCNAME
#define FCNAME DECL_FUNC(check_maprange_ok)
__SI__ int check_maprange_ok(void   *start,
                             size_t  size)
{
  int     rc          = 0;
  size_t  page_sz;
  size_t  mapsize     = get_mapsize(size,&page_sz);
  size_t  i,num_pages = mapsize/page_sz;
  char   *ptr         = (char *)start;

  for(i=0; i<num_pages; i++) {
    rc = msync(ptr,page_sz,0);

    if(rc == -1) {
      MPIR_Assert(errno == ENOMEM);
      ptr+=page_sz;
    } else
      return 0;
  }

  return 1;
}

#undef FCNAME
#define FCNAME DECL_FUNC(generate_random_addr)
__SI__ void *generate_random_addr(size_t size)
{
  /* starting position for pointer to map
   * This is not generic, probably only works properly on Linux
   * but it's not fatal since we bail after a fixed number of iterations
   */
#define MAP_POINTER ((random_int&((0x00006FFFFFFFFFFF&(~(page_sz-1)))|0x0000600000000000)))
  char            random_state[256];
  char           *oldstate;
  size_t          page_sz, random_int;
  size_t          mapsize     = get_mapsize(size,&page_sz);
  uintptr_t       map_pointer;
  struct timeval  ts;
  int             iter = 100;

  gettimeofday(&ts, NULL);
  initstate(ts.tv_usec,random_state,256);
  oldstate    = setstate(random_state);
  random_int  = random()<<32|random();
  map_pointer = MAP_POINTER;

  while(check_maprange_ok((void *)map_pointer,mapsize) == 0) {
    random_int  = random()<<32|random();
    map_pointer = MAP_POINTER;
    iter--;

    if(iter == 0)
      return (void *)-1ULL;
  }

  setstate(oldstate);
  return (void *)map_pointer;
}

#undef FCNAME
#define FCNAME DECL_FUNC(get_symmetric_heap)
__SI__ int get_symmetric_heap(MPI_Aint    size,
                              MPIR_Comm  *comm,
                              void      **base,
                              MPIR_Win   *win)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  unsigned  test, result;
  int       iter=100;
  MPIR_Errflag_t errflag = MPIR_ERR_NONE;
  void     *baseP;
  size_t    page_sz;
  size_t    mapsize;

  mapsize = get_mapsize(size, &page_sz);

  struct {
    uint64_t sz;
    int      loc;
  } maxloc, maxloc_result;

  maxloc.sz  = size;
  maxloc.loc = comm->rank;
  mpi_errno = MPIR_Allreduce_impl(&maxloc,
                                  &maxloc_result,
                                  1,
                                  MPI_LONG_INT,
                                  MPI_MAXLOC,
                                  comm,
                                  &errflag);

  if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

  if(maxloc_result.sz > 0) {
    result = 0;

    while(!result && --iter!=0) {
      uintptr_t  map_pointer = 0ULL;

      baseP                  = (void *)-1ULL;

      if(comm->rank == maxloc_result.loc) {
        map_pointer = (uintptr_t)generate_random_addr(mapsize);
        baseP       = mmap((void *)map_pointer,
                           mapsize,
                           PROT_READ|PROT_WRITE,
                           MAP_PRIVATE|MAP_ANON|MAP_FIXED,
                           -1, 0);
      }

      mpi_errno = MPIR_Bcast_impl(&map_pointer,
                                  1,
                                  MPI_UNSIGNED_LONG,
                                  maxloc_result.loc,
                                  comm,&errflag);

      if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

      if(comm->rank != maxloc_result.loc) {
        int rc = check_maprange_ok((void *)map_pointer,mapsize);

        if(rc) {
          baseP = mmap((void *)map_pointer,
                       mapsize,
                       PROT_READ|PROT_WRITE,
                       MAP_PRIVATE|MAP_ANON|MAP_FIXED,
                       -1, 0);
        } else
          baseP = (void *)-1ULL;
      }

      if(mapsize == 0) baseP = (void *)map_pointer;

      test = ((uintptr_t)baseP != -1ULL)?1:0;
      mpi_errno = MPIR_Allreduce_impl(&test,
                                      &result,
                                      1,
                                      MPI_UNSIGNED,
                                      MPI_BAND,
                                      comm,
                                      &errflag);

      if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

      if(result == 0 && baseP!=(void *)-1ULL)
        munmap(baseP, mapsize);
    }
  } else
    baseP = NULL;

  if(iter == 0) {
    fprintf(stderr, "WARNING: Win_allocate:  Unable to allocate symmetric heap\n");
    baseP = MPL_malloc(size);
    MPIR_ERR_CHKANDJUMP((baseP == NULL), mpi_errno,
                        MPI_ERR_BUFFER, "**bufnull");
    WIN_OFI(win)->mmap_sz   = -1ULL;
    WIN_OFI(win)->mmap_addr = NULL;
  } else {
    WIN_OFI(win)->mmap_sz   = mapsize;
    WIN_OFI(win)->mmap_addr = baseP;
  }

  *base = baseP;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_datatype_unmap)
__SI__ void MPIDI_Win_datatype_unmap(MPIDI_Win_dt *dt)
{
  BEGIN_FUNC(FCNAME);

  if(dt->map != &dt->__map)
    MPL_free(dt->map);

  END_FUNC(FCNAME);
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_datatype_map)
__SI__ void MPIDI_Win_datatype_map(MPIDI_Win_dt *dt)
{
  BEGIN_FUNC(FCNAME);

  if(dt->contig) {
    dt->num_contig              = 1;
    dt->map                     = &dt->__map;
    dt->map[0].DLOOP_VECTOR_BUF = (void *)(size_t)dt->true_lb;
    dt->map[0].DLOOP_VECTOR_LEN = dt->size;
  } else {
    unsigned map_size = dt->pointer->max_contig_blocks*dt->count + 1;
    dt->num_contig    = map_size;
    dt->map           = (DLOOP_VECTOR *)MPL_malloc(map_size * sizeof(DLOOP_VECTOR));
    MPIR_Assert(dt->map != NULL);

    MPID_Segment seg;
    DLOOP_Offset last = dt->pointer->size*dt->count;
    MPIDU_Segment_init(NULL, dt->count, dt->type, &seg, 0);
    MPIDU_Segment_pack_vector(&seg, 0, &last, dt->map, &dt->num_contig);
    MPIR_Assert((unsigned)dt->num_contig <= map_size);
  }

  END_FUNC(FCNAME);
}

#undef FCNAME
#define FCNAME DECL_FUNC(rma_done_callback)
__SI__ int rma_done_callback(cq_tagged_entry_t *wc,
                             MPIR_Request      *in_req)
{
  int   mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  MPIDI_Win_request *req = (MPIDI_Win_request *)in_req;
  MPIDI_Win_datatype_unmap(&req->noncontig->target_dt);
  MPIDI_Win_datatype_unmap(&req->noncontig->origin_dt);
  MPIDI_Win_datatype_unmap(&req->noncontig->result_dt);
  MPIDI_Win_request_complete(req);

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(chunk_done_callback)
__SI__ int chunk_done_callback(cq_tagged_entry_t *wc,
                               MPIR_Request      *req)
{
  int mpi_errno = MPI_SUCCESS;
  int c;
  BEGIN_FUNC(FCNAME);
  MPIDI_Chunk_request *creq = (MPIDI_Chunk_request *)req;
  MPIR_cc_decr(creq->parent->cc_ptr, &c);
  MPIR_Assert(c >= 0);
  if(c == 0)MPIDI_Request_free_hook(creq->parent);
  MPL_free(creq);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Progress_win_counter_fence)
__SI__ int MPIDI_Progress_win_counter_fence(MPIR_Win *win)
{
  int      mpi_errno = MPI_SUCCESS;
  uint64_t tcount, donecount;
  MPIDI_Win_request *r;
  BEGIN_FUNC(FCNAME);
  tcount    = MPIDI_Global.cntr;
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
  donecount = fi_cntr_read(MPIDI_Global.rma_ctr);
  MPIR_Assert(donecount <= tcount);
  while(tcount > donecount) {
    MPIR_Assert(donecount <= tcount);
    MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
    PROGRESS();
    MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
    donecount = fi_cntr_read(MPIDI_Global.rma_ctr);
  }
  r = WIN_OFI(win)->syncQ;
  while(r)  {
    MPIDI_Win_request *next = r->next;
    rma_done_callback(NULL,(MPIR_Request *)r);
    r = next;
  }
  WIN_OFI(win)->syncQ = NULL;
fn_exit:
  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

static int init_win() __attribute__((constructor));
static int init_win()
{
  if(!MPIDI_Global.control_init) {
    MPIDI_Global.lkey          = MPID_LKEY_START;
    MPIDI_Global.control_init  = 1;
    /* Todo:  Split callbacks to avoid the extra branch */
    MPIDI_Global.control_fn[MPIDI_CTRL_ASSERT]       = MPIDI_control_assert;
    MPIDI_Global.control_fn[MPIDI_CTRL_LOCKACK]      = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_LOCKALLACK]   = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_LOCKREQ]      = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_LOCKALLREQ]   = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_UNLOCK]       = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_UNLOCKACK]    = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_UNLOCKALL]    = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_UNLOCKALLACK] = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_COMPLETE]     = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_POST]         = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_HUGE]         = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_HUGEACK]      = MPIDI_control_callback;
    MPIDI_Global.control_fn[MPIDI_CTRL_HUGE_CLEANUP] = MPIDI_control_callback;
  }

  return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_valid_group_rank)
__SI__ int MPIDI_valid_group_rank(int         lpid,
                                  MPIR_Group *grp)
{
  BEGIN_FUNC(FCNAME);
  int size = grp->size;
  int z;

  for(z = 0; z < size &&lpid != grp->lrank_to_lpid[z].lpid; ++z);

  END_FUNC(FCNAME);
  return (z < size);
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_datatype_basic)
__SI__ void MPIDI_Win_datatype_basic(int           count,
                                     MPI_Datatype  datatype,
                                     MPIDI_Win_dt *dt)
{
  BEGIN_FUNC(FCNAME);

  if(datatype != MPI_DATATYPE_NULL)
    MPIDI_Datatype_get_info(dt->count = count,
                            dt->type  = datatype,
                            dt->contig,
                            dt->size,
                            dt->pointer,
                            dt->true_lb);
  END_FUNC(FCNAME);
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_init)
__SI__ int MPIDI_Win_init(MPI_Aint     length,
                          int          disp_unit,
                          MPIR_Win   **win_ptr,
                          MPIR_Info   *info,
                          MPIR_Comm   *comm_ptr,
                          int          create_flavor,
                          int          model)
{
  int             mpi_errno = MPI_SUCCESS;
  int             rank, size;

  BEGIN_FUNC(FCNAME);
  ADI_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devwin_t)>=sizeof(MPIDI_OFIWin_t));
  ADI_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devdt_t)>=sizeof(MPIDI_OFIdt_t));

  MPIR_Win *win = (MPIR_Win *)MPIR_Handle_obj_alloc(&MPIR_Win_mem);
  MPIR_ERR_CHKANDSTMT(win == NULL,
                      mpi_errno,
                      MPI_ERR_NO_MEM,
                      goto fn_fail,
                      "**nomem");
  *win_ptr = win;

  memset(WIN_OFI(win), 0, sizeof(*WIN_OFI(win)));
  win->comm_ptr = comm_ptr;
  size          = comm_ptr->local_size;
  rank          = comm_ptr->rank;
  MPIR_Comm_add_ref(comm_ptr);

  WIN_OFI(win)->winfo = (MPIDI_Win_info *)MPL_calloc(size,sizeof(MPIDI_Win_info));

  MPIR_ERR_CHKANDSTMT(WIN_OFI(win)->winfo == NULL,mpi_errno,MPI_ERR_NO_MEM,
                      goto fn_fail,"**nomem");
  win->errhandler          = NULL;
  win->base                = NULL;
  win->size                = length;
  win->disp_unit           = disp_unit;
  win->create_flavor       = (MPIR_Win_flavor_t)create_flavor;
  win->model               = (MPIR_Win_model_t)model;
  win->copyCreateFlavor    = (MPIR_Win_flavor_t)0;
  win->copyModel           = (MPIR_Win_model_t)0;
  win->attributes          = NULL;
  win->comm_ptr            = comm_ptr;
  win->copyDispUnit        = 0;
  win->copySize            = 0;

  if((info != NULL) && ((int *)info != (int *) MPI_INFO_NULL)) {
    mpi_errno= MPIDI_Win_set_info(win, info);
    MPIR_Assert(mpi_errno == 0);
  }

  /* Initialize the info (hint) flags per window */
  WIN_OFI(win)->info_args.no_locks               = 0;
  WIN_OFI(win)->info_args.accumulate_ordering    = (MPIDI_Win_info_accumulate_ordering)
    (MPIDI_ACCU_ORDER_RAR |
     MPIDI_ACCU_ORDER_RAW |
     MPIDI_ACCU_ORDER_WAR |
     MPIDI_ACCU_ORDER_WAW);
  WIN_OFI(win)->info_args.accumulate_ops         = MPIDI_ACCU_SAME_OP_NO_OP;
  WIN_OFI(win)->info_args.same_size              = 0;
  WIN_OFI(win)->info_args.alloc_shared_noncontig = 0;
  WIN_OFI(win)->mmap_sz                          = 0;
  WIN_OFI(win)->mmap_addr                        = NULL;
  MPIDI_Win_info *winfo;
  winfo            = (MPIDI_Win_info *)WINFO(win,rank);
  winfo->disp_unit = disp_unit;
  /* Fill out MR later, if required */

  /* context id lower bits, window instance upper bits */
  WIN_OFI(win)->win_id = 1+
    (((uint64_t)comm_ptr->context_id) |
     ((uint64_t)((COMM_OFI(comm_ptr)->window_instance)++)<<32));
  MPIDI_Map_set(MPIDI_Global.win_map,WIN_OFI(win)->win_id,win);

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:

  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_allgather)
__SI__ int MPIDI_Win_allgather(MPIR_Win  *win)
{
  int            mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t errflag = MPIR_ERR_NONE;
  BEGIN_FUNC(FCNAME);

  MPIR_Comm *comm_ptr = win->comm_ptr;
  mpi_errno = MPIR_Allgather_impl(MPI_IN_PLACE,
                                  0,
                                  MPI_DATATYPE_NULL,
                                  WIN_OFI(win)->winfo,
                                  sizeof(MPIDI_Win_info),
                                  MPI_BYTE,
                                  comm_ptr,
                                  &errflag);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_lock)
__ADI_INLINE__ int MPIDI_Win_lock(int       lock_type,
                                  int       rank,
                                  int       massert,
                                  MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);
  MPIDI_Win_sync_lock *slock = &WIN_OFI(win)->sync.lock;
  EPOCH_CHECK2();

  if(rank == MPI_PROC_NULL) goto fn_exit0;

  MPIDI_Win_control_t  msg;

  msg.type      = MPIDI_CTRL_LOCKREQ;
  msg.lock_type = lock_type;

  mpi_errno     = do_control_win(&msg, rank, win,1);

  if(mpi_errno != MPI_SUCCESS)
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                        goto fn_fail, "**rmasync");

  PROGRESS_WHILE(!slock->remote.locked);

fn_exit0:
  WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_LOCK;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_unlock)
__ADI_INLINE__ int MPIDI_Win_unlock(int       rank,
                                    MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  EPOCH_ORIGIN_CHECK(MPID_EPOTYPE_LOCK);

  if(rank == MPI_PROC_NULL) goto fn_exit0;

  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

  MPIDI_Win_control_t msg;
  msg.type  = MPIDI_CTRL_UNLOCK;
  mpi_errno = do_control_win(&msg, rank, win, 1);

  if(mpi_errno != MPI_SUCCESS)
    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                        goto fn_fail, "**rmasync");

  PROGRESS_WHILE(WIN_OFI(win)->sync.lock.remote.locked);
fn_exit0:
  WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;
  WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_set_info)
__ADI_INLINE__ int MPIDI_Win_set_info(MPIR_Win     *win,
                                      MPIR_Info    *info)
{
  int            mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t errflag = MPIR_ERR_NONE;
  BEGIN_FUNC(FCNAME);

  MPIR_Info *curr_ptr;
  char      *value, *token, *savePtr;
  uint       save_ordering;

  curr_ptr = info->next;

  while(curr_ptr) {
    if(!strcmp(curr_ptr->key,"no_locks")) {
      if(!strcmp(curr_ptr->value,"true"))
        WIN_OFI(win)->info_args.no_locks=1;
      else
        WIN_OFI(win)->info_args.no_locks=0;
    } else if(!strcmp(curr_ptr->key,"accumulate_ordering")) {
      save_ordering=(uint) WIN_OFI(win)->info_args.accumulate_ordering;
      WIN_OFI(win)->info_args.accumulate_ordering=(MPIDI_Win_info_accumulate_ordering)0;
      value = curr_ptr->value;
      token = (char *) strtok_r(value,"," , &savePtr);

      while(token) {
        if(!memcmp(token,"rar",3))
          WIN_OFI(win)->info_args.accumulate_ordering =
            (MPIDI_Win_info_accumulate_ordering)
            (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_RAR);
        else if(!memcmp(token,"raw",3))
          WIN_OFI(win)->info_args.accumulate_ordering =
            (MPIDI_Win_info_accumulate_ordering)
            (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_RAW);
        else if(!memcmp(token,"war",3))
          WIN_OFI(win)->info_args.accumulate_ordering =
            (MPIDI_Win_info_accumulate_ordering)
            (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_WAR);
        else if(!memcmp(token,"waw",3))
          WIN_OFI(win)->info_args.accumulate_ordering =
            (MPIDI_Win_info_accumulate_ordering)
            (WIN_OFI(win)->info_args.accumulate_ordering | MPIDI_ACCU_ORDER_WAW);
        else
          MPIR_Assert(0);

        token = (char *) strtok_r(NULL,"," , &savePtr);
      }

      if(WIN_OFI(win)->info_args.accumulate_ordering == 0)
        WIN_OFI(win)->info_args.accumulate_ordering=
          (MPIDI_Win_info_accumulate_ordering) save_ordering;
    } else if(!strcmp(curr_ptr->key,"accumulate_ops")) {
      /* the default setting is MPIDI_ACCU_SAME_OP_NO_OP */
      if(!strcmp(curr_ptr->value,"same_op"))
        WIN_OFI(win)->info_args.accumulate_ops = MPIDI_ACCU_SAME_OP;
    }

    curr_ptr = curr_ptr->next;
  }

  mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_free)
__ADI_INLINE__ int MPIDI_Win_free(MPIR_Win **win_ptr)
{
  int            mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
  BEGIN_FUNC(FCNAME);

  MPIR_Win *win  = *win_ptr;

  EPOCH_FREE_CHECK();

  mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

  if(mpi_errno != MPI_SUCCESS) goto fn_fail;

  if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE  && win->base) {
    if(WIN_OFI(win)->mmap_sz > 0)
      munmap(WIN_OFI(win)->mmap_addr, WIN_OFI(win)->mmap_sz);
    else if(WIN_OFI(win)->mmap_sz == -1)
      MPL_free(win->base);
  }

  if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
    if(WIN_OFI(win)->mmap_addr)
      munmap(WIN_OFI(win)->mmap_addr, WIN_OFI(win)->mmap_sz);

    MPL_free(WIN_OFI(win)->sizes);
  }

  if(WIN_OFI(win)->msgQ)
    MPL_free(WIN_OFI(win)->msgQ);

  MPL_free(WIN_OFI(win)->winfo);
  MPIR_Comm_release(win->comm_ptr);
  MPIR_Handle_obj_free(&MPIR_Win_mem, win);

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_fence)
__ADI_INLINE__ int MPIDI_Win_fence(int       massert,
                                   MPIR_Win *win)
{
  int            mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t errflag = MPIR_ERR_NONE;
  BEGIN_FUNC(FCNAME);

  EPOCH_FENCE_CHECK();

  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

  EPOCH_FENCE_EVENT();

  if(!(massert & MPI_MODE_NOPRECEDE))
    mpi_errno = MPIR_Barrier_impl(win->comm_ptr, &errflag);

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_create)
__ADI_INLINE__ int MPIDI_Win_create(void        *base,
                                    MPI_Aint     length,
                                    int          disp_unit,
                                    MPIR_Info   *info,
                                    MPIR_Comm   *comm_ptr,
                                    MPIR_Win   **win_ptr)
{
  int             mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t  errflag   = MPIR_ERR_NONE;
  MPIR_Win       *win;
  int             rank;
  MPIDI_Win_info *winfo;

  BEGIN_FUNC(FCNAME);

  mpi_errno        = MPIDI_Win_init(length,
                                    disp_unit,
                                    win_ptr,
                                    info,
                                    comm_ptr,
                                    MPI_WIN_FLAVOR_CREATE,
                                    MPI_WIN_UNIFIED);

  if(mpi_errno != MPI_SUCCESS) goto fn_fail;

  win              = *win_ptr;
  win->base        = base;
  rank             = comm_ptr->rank;
  winfo            = (MPIDI_Win_info *)WINFO(win,rank);
  winfo->base_addr = base;
  winfo->disp_unit = disp_unit;

  mpi_errno = MPIDI_Win_allgather(win);

  if(mpi_errno != MPI_SUCCESS) goto fn_fail;

  mpi_errno = MPIR_Barrier_impl(comm_ptr,&errflag);

  if(mpi_errno != MPI_SUCCESS) goto fn_fail;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_put)
__SI__ int do_put(const void    *origin_addr,
                  int            origin_count,
                  MPI_Datatype   origin_datatype,
                  int            target_rank,
                  MPI_Aint       target_disp,
                  int            target_count,
                  MPI_Datatype   target_datatype,
                  MPIR_Win      *win,
                  MPIR_Request **signal)
{
  int                rc,mpi_errno = MPI_SUCCESS;
  MPIDI_Win_request *req;
  size_t             offset;
  uint64_t           flags=0ULL;
  MPIR_Request      *sigreq;
  fid_ep_t           ep;

  BEGIN_FUNC(FCNAME);
  EPOCH_CHECK1();

  WINREQ_CREATE(req);
  SETUP_SIGNAL_REQUEST();

  offset   = target_disp * WINFO_DISP_UNIT(win,target_rank);

  MPIDI_Win_datatype_basic(origin_count,
                           origin_datatype,
                           &req->noncontig->origin_dt);
  MPIDI_Win_datatype_basic(target_count,
                           target_datatype,
                           &req->noncontig->target_dt);
  MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                      mpi_errno,MPI_ERR_SIZE,"**rmasize");

  if((req->noncontig->origin_dt.size == 0) ||
     (target_rank == MPI_PROC_NULL)) {
    MPIDI_Win_request_complete(req);
    if(signal) MPIDI_Request_free_hook(sigreq);
    goto fn_exit;
  }
  if(target_rank == win->comm_ptr->rank) {
    MPIDI_Win_request_complete(req);
    if(signal) MPIDI_Request_free_hook(sigreq);
    return MPIR_Localcopy(origin_addr,
                          origin_count,
                          origin_datatype,
                          (char *)win->base + offset,
                          target_count,
                          target_datatype);
  }
  EPOCH_START_CHECK();
  req->target_rank = target_rank;

  MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
  MPIDI_Win_datatype_map(&req->noncontig->target_dt);

  msg_rma_t msg;
  void *desc;
  desc = fi_mr_desc(MPIDI_Global.mr);
  req->callback = NULL;
  msg.desc      = &desc;
  msg.addr      = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
  msg.context   = NULL;
  msg.data      = 0;
  req->next              = WIN_OFI(win)->syncQ;
  WIN_OFI(win)->syncQ    = req;
  MPIDI_Init_iovec_state(&req->noncontig->iovs,
                         (uintptr_t)origin_addr,
                         (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                         req->noncontig->origin_dt.num_contig,
                         req->noncontig->target_dt.num_contig,
                         INT64_MAX,
                         req->noncontig->origin_dt.map,
                         req->noncontig->target_dt.map);
  rc = MPIDI_IOV_EAGAIN;
  while(rc==MPIDI_IOV_EAGAIN) {
    iovec_t   originv[1];
    rma_iov_t targetv[1];
    size_t    omax;
    size_t    tmax;
    size_t    tout, oout;
    int       i;
    omax=tmax=1;
    rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,originv,
                              omax,targetv,tmax,&oout,&tout);
    if(rc==MPIDI_IOV_DONE) break;
    for(i=0; i<tout; i++) targetv[i].key = WINFO_MR_KEY(win,target_rank);
    MPIR_Assert(rc != MPIDI_IOV_ERROR);
    msg.msg_iov       = originv;
    msg.iov_count     = oout;
    msg.rma_iov       = targetv;
    msg.rma_iov_count = tout;
    SETUP_CHUNK_CONTEXT();
    FI_RC_RETRY(fi_writemsg(ep, &msg, flags), rdma_write);
  }
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Put)
__ADI_INLINE__ int MPIDI_Put(const void   *origin_addr,
                             int           origin_count,
                             MPI_Datatype  origin_datatype,
                             int           target_rank,
                             MPI_Aint      target_disp,
                             int           target_count,
                             MPI_Datatype  target_datatype,
                             MPIR_Win     *win)
{
  BEGIN_FUNC(FCNAME);
  int            mpi_errno,target_contig,origin_contig;
  MPIDI_msg_sz_t target_bytes,origin_bytes;
  MPI_Aint       origin_true_lb,target_true_lb;

  MPIDI_Datatype_check_contig_size_lb(target_datatype,target_count,
                                      target_contig,target_bytes,
                                      target_true_lb);
  MPIDI_Datatype_check_contig_size_lb(origin_datatype,origin_count,
                                      origin_contig,origin_bytes,
                                      origin_true_lb);

  if(origin_contig && target_contig && origin_bytes &&
     target_rank != MPI_PROC_NULL       &&
     target_rank != win->comm_ptr->rank &&
     (origin_bytes<=MPIDI_Global.max_buffered_write)) {
    mpi_errno = MPI_SUCCESS;
    EPOCH_CHECK1();
    MPIR_ERR_CHKANDJUMP((origin_bytes != target_bytes),
                        mpi_errno,MPI_ERR_SIZE,"**rmasize");
    EPOCH_START_CHECK();
    MPIDI_Global.cntr++;
    FI_RC_RETRY(fi_inject_write(G_TXC_CTR(0),                                  /* endpoint     */
                                (char *)origin_addr+origin_true_lb,            /* local buffer */
                                target_bytes,                                  /* bytes        */
                                _comm_to_phys(win->comm_ptr,target_rank,MPIDI_API_CTR),       /* Destination  */
                                (uint64_t)(char *)WINFO_BASE(win,target_rank)+
                                target_disp*WINFO_DISP_UNIT(win,target_rank)+target_true_lb,  /* remote maddr */
                                WINFO_MR_KEY(win,target_rank)),rdma_inject_write);
  } else{
    mpi_errno = do_put(origin_addr,
                       origin_count,
                       origin_datatype,
                       target_rank,
                       target_disp,
                       target_count,
                       target_datatype,
                       win,
                       NULL);
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

__ADI_INLINE__ int MPIDI_Rput(const void    *origin_addr,
                              int            origin_count,
                              MPI_Datatype   origin_datatype,
                              int            target_rank,
                              MPI_Aint       target_disp,
                              int            target_count,
                              MPI_Datatype   target_datatype,
                              MPIR_Win      *win,
                              MPIR_Request **request)
{
  BEGIN_FUNC(FCNAME);
  MPIR_Request  *rreq;
  int mpi_errno = do_put((void *)origin_addr,
                         origin_count,
                         origin_datatype,
                         target_rank,
                         target_disp,
                         target_count,
                         target_datatype,
                         win,
                         &rreq);
  *request = rreq;
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_start)
__ADI_INLINE__ int MPIDI_Win_start(MPIR_Group *group,
                                   int         massert,
                                   MPIR_Win   *win)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  EPOCH_CHECK2();

  MPIR_Group_add_ref(group);

  PROGRESS_WHILE(group->size != WIN_OFI(win)->sync.pw.count);

  WIN_OFI(win)->sync.pw.count = 0;

  MPIR_ERR_CHKANDJUMP((WIN_OFI(win)->sync.sc.group != NULL),
                      mpi_errno,
                      MPI_ERR_GROUP,
                      "**group");
  WIN_OFI(win)->sync.sc.group          = group;
  WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_START;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_complete)
__ADI_INLINE__ int MPIDI_Win_complete(MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  EPOCH_ORIGIN_CHECK(MPID_EPOTYPE_START);

  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

  MPIR_Group *group;
  group = WIN_OFI(win)->sync.sc.group;
  MPIR_Assert(group != NULL);
  MPIDI_Win_control_t msg;
  msg.type = MPIDI_CTRL_COMPLETE;

  int index, peer;

  for(index=0; index < group->size; ++index) {
    peer      = group->lrank_to_lpid[index].lpid;
    mpi_errno = do_control_win(&msg, peer, win, 0);

    if(mpi_errno != MPI_SUCCESS)
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                          goto fn_fail, "**rmasync");
  }

  EPOCH_TARGET_EVENT();

  MPIR_Group_release(WIN_OFI(win)->sync.sc.group);
  WIN_OFI(win)->sync.sc.group = NULL;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_post)
__ADI_INLINE__ int MPIDI_Win_post(MPIR_Group *group,
                                  int         massert,
                                  MPIR_Win   *win)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  EPOCH_POST_CHECK();

  MPIR_Group_add_ref(group);
  MPIR_ERR_CHKANDJUMP((WIN_OFI(win)->sync.pw.group != NULL),
                      mpi_errno, MPI_ERR_GROUP,
                      "**group");

  WIN_OFI(win)->sync.pw.group = group;
  MPIR_Assert(group != NULL);
  MPIDI_Win_control_t msg;
  msg.type = MPIDI_CTRL_POST;

  int index;

  for(index=0; index < group->size; ++index) {
    int peer  = group->lrank_to_lpid[index].lpid;
    mpi_errno = do_control_win(&msg, peer, win, 0);

    if(mpi_errno != MPI_SUCCESS)
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                          goto fn_fail, "**rmasync");
  }

  WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_POST;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_wait)
__ADI_INLINE__ int MPIDI_Win_wait(MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  EPOCH_TARGET_CHECK(MPID_EPOTYPE_POST);

  MPIR_Group *group;
  group = WIN_OFI(win)->sync.pw.group;

  PROGRESS_WHILE(group->size != WIN_OFI(win)->sync.sc.count);

  WIN_OFI(win)->sync.sc.count = 0;
  WIN_OFI(win)->sync.pw.group = NULL;

  MPIR_Group_release(group);

  EPOCH_ORIGIN_EVENT();

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_test)
__ADI_INLINE__ int MPIDI_Win_test(MPIR_Win *win,
                                  int      *flag)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  EPOCH_TARGET_CHECK(MPID_EPOTYPE_POST);

  MPIR_Group *group;
  group = WIN_OFI(win)->sync.pw.group;

  if(group->size == WIN_OFI(win)->sync.sc.count) {
    WIN_OFI(win)->sync.sc.count = 0;
    WIN_OFI(win)->sync.pw.group = NULL;
    *flag          = 1;
    MPIR_Group_release(group);
    EPOCH_ORIGIN_EVENT();
  } else {
    PROGRESS();
    *flag=0;
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

extern int MPIR_Info_alloc(MPIR_Info **info_p_p);
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_get_info)
__ADI_INLINE__ int MPIDI_Win_get_info(MPIR_Win     *win,
                                      MPIR_Info   **info_p_p)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  mpi_errno = MPIR_Info_alloc(info_p_p);
  MPIR_Assert(mpi_errno == MPI_SUCCESS);

  if(WIN_OFI(win)->info_args.no_locks)
    mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "true");
  else
    mpi_errno = MPIR_Info_set_impl(*info_p_p, "no_locks", "false");

  MPIR_Assert(mpi_errno == MPI_SUCCESS);

  {
#define BUFSIZE 32
    char buf[BUFSIZE];
    int c = 0;

    if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_RAR)
      c += snprintf(buf+c, BUFSIZE-c, "%srar", (c > 0) ? "," : "");

    if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_RAW)
      c += snprintf(buf+c, BUFSIZE-c, "%sraw", (c > 0) ? "," : "");

    if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_WAR)
      c += snprintf(buf+c, BUFSIZE-c, "%swar", (c > 0) ? "," : "");

    if(WIN_OFI(win)->info_args.accumulate_ordering & MPIDI_ACCU_ORDER_WAW)
      c += snprintf(buf+c, BUFSIZE-c, "%swaw", (c > 0) ? "," : "");

    if(c == 0) {
      memcpy(&buf[0],"not set   ",10);
    }

    MPIR_Info_set_impl(*info_p_p, "accumulate_ordering", buf);
    MPIR_Assert(mpi_errno == MPI_SUCCESS);
#undef BUFSIZE
  }

  if(WIN_OFI(win)->info_args.accumulate_ops == MPIDI_ACCU_SAME_OP)
    mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op");
  else
    mpi_errno = MPIR_Info_set_impl(*info_p_p, "accumulate_ops", "same_op_no_op");

  MPIR_Assert(mpi_errno == MPI_SUCCESS);

  if(win->create_flavor == MPI_WIN_FLAVOR_SHARED) {
    if(WIN_OFI(win)->info_args.alloc_shared_noncontig)
      mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "true");
    else
      mpi_errno = MPIR_Info_set_impl(*info_p_p, "alloc_shared_noncontig", "false");

    MPIR_Assert(mpi_errno == MPI_SUCCESS);
  } else if(win->create_flavor == MPI_WIN_FLAVOR_ALLOCATE) {
    if(WIN_OFI(win)->info_args.same_size)
      mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "true");
    else
      mpi_errno = MPIR_Info_set_impl(*info_p_p, "same_size", "false");

    MPIR_Assert(mpi_errno == MPI_SUCCESS);
  }

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_get_lw)
__SI__ int do_get_lw(void               *origin_addr,
                     int                 origin_count,
                     MPI_Datatype        origin_datatype,
                     int                 target_rank,
                     MPI_Aint            target_disp,
                     int                 target_count,
                     MPI_Datatype        target_datatype,
                     MPIR_Win           *win)
{
  int                mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPIDI_Win_dt origin_dt, target_dt;
  size_t       offset;

  EPOCH_CHECK1();

  offset = target_disp * WINFO_DISP_UNIT(win,target_rank);
  MPIDI_Win_datatype_basic(origin_count,origin_datatype,&origin_dt);
  MPIDI_Win_datatype_basic(target_count,target_datatype,&target_dt);
  MPIR_ERR_CHKANDJUMP((origin_dt.size != target_dt.size),
                      mpi_errno, MPI_ERR_SIZE, "**rmasize");

  if((origin_dt.size == 0)||(target_rank == MPI_PROC_NULL))
    goto fn_exit;

  if(target_rank == win->comm_ptr->rank)
    return MPIR_Localcopy((char *)win->base + offset,
                          target_count,
                          target_datatype,
                          origin_addr,
                          origin_count,
                          origin_datatype);
  void *buffer,*tbuffer;
  buffer  = (char *)origin_addr + origin_dt.true_lb;
  tbuffer = (char *)WINFO_BASE(win,target_rank) + offset + target_dt.true_lb;

  EPOCH_START_CHECK();
  rma_iov_t riov;
  iovec_t   iov;
  msg_rma_t msg;
  void     *desc;
  desc = fi_mr_desc(MPIDI_Global.mr);
  msg.desc          = &desc;
  msg.msg_iov       = &iov;
  msg.iov_count     = 1;
  msg.addr          = _comm_to_phys(win->comm_ptr,target_rank,MPIDI_API_CTR);
  msg.rma_iov       = &riov;
  msg.rma_iov_count = 1;
  msg.context       = NULL;
  msg.data          = 0;
  iov.iov_base      = buffer;
  iov.iov_len       = target_dt.size;
  riov.addr         = (uint64_t)tbuffer;
  riov.len          = target_dt.size;
  riov.key          = WINFO_MR_KEY(win,target_rank);
  MPIDI_Global.cntr++;
  FI_RC_RETRY(fi_readmsg(G_TXC_CTR(0), &msg, 0), rdma_write);
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_get)
__SI__ int do_get(void          *origin_addr,
                  int            origin_count,
                  MPI_Datatype   origin_datatype,
                  int            target_rank,
                  MPI_Aint       target_disp,
                  int            target_count,
                  MPI_Datatype   target_datatype,
                  MPIR_Win      *win,
                  MPIR_Request **signal)
{
  int                rc, mpi_errno = MPI_SUCCESS;
  MPIDI_Win_request *req;
  size_t             offset;
  uint64_t           flags=0ULL;
  MPIR_Request      *sigreq;
  fid_ep_t           ep;

  BEGIN_FUNC(FCNAME);
  EPOCH_CHECK1();

  WINREQ_CREATE(req);
  SETUP_SIGNAL_REQUEST();

  offset   = target_disp * WINFO_DISP_UNIT(win,target_rank);

  MPIDI_Win_datatype_basic(origin_count,
                           origin_datatype,
                           &req->noncontig->origin_dt);
  MPIDI_Win_datatype_basic(target_count,
                           target_datatype,
                           &req->noncontig->target_dt);
  MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                      mpi_errno, MPI_ERR_SIZE, "**rmasize");

  if((req->noncontig->origin_dt.size == 0) ||
     (target_rank == MPI_PROC_NULL)) {
    MPIDI_Win_request_complete(req);
    if(signal) MPIDI_Request_free_hook(sigreq);
    goto fn_exit;
  }

  if(target_rank == win->comm_ptr->rank) {
    MPIDI_Win_request_complete(req);
    if(signal) MPIDI_Request_free_hook(sigreq);
    return MPIR_Localcopy((char *)win->base + offset,
                          target_count,
                          target_datatype,
                          origin_addr,
                          origin_count,
                          origin_datatype);
  }
  EPOCH_START_CHECK();
  req->target_rank = target_rank;

  MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
  MPIDI_Win_datatype_map(&req->noncontig->target_dt);

  req->callback          = NULL;
  msg_rma_t  msg;
  void      *desc;
  desc = fi_mr_desc(MPIDI_Global.mr);
  msg.desc      = &desc;
  msg.addr      = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
  msg.context   = NULL;
  msg.data      = 0;
  req->next              = WIN_OFI(win)->syncQ;
  WIN_OFI(win)->syncQ    = req;
  MPIDI_Init_iovec_state(&req->noncontig->iovs,
                         (uintptr_t)origin_addr,
                         (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                         req->noncontig->origin_dt.num_contig,
                         req->noncontig->target_dt.num_contig,
                         INT64_MAX,
                         req->noncontig->origin_dt.map,
                         req->noncontig->target_dt.map);
  rc = MPIDI_IOV_EAGAIN;
  while(rc==MPIDI_IOV_EAGAIN) {
    iovec_t   originv[1];
    rma_iov_t targetv[1];
    size_t    omax,tmax,tout,oout;
    int       i;
    omax=tmax=1;
    rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,originv,
                              omax,targetv,tmax,&oout,&tout);
    if(rc==MPIDI_IOV_DONE) break;
    MPIR_Assert(rc != MPIDI_IOV_ERROR);
    for(i=0; i<tout; i++) targetv[i].key = WINFO_MR_KEY(win,target_rank);
    msg.msg_iov       = originv;
    msg.iov_count     = oout;
    msg.rma_iov       = targetv;
    msg.rma_iov_count = tout;
    SETUP_CHUNK_CONTEXT();
    FI_RC_RETRY(fi_readmsg(ep, &msg, flags), rdma_write);
  }
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Get)
__ADI_INLINE__ int MPIDI_Get(void         *origin_addr,
                             int           origin_count,
                             MPI_Datatype  origin_datatype,
                             int           target_rank,
                             MPI_Aint      target_disp,
                             int           target_count,
                             MPI_Datatype  target_datatype,
                             MPIR_Win     *win)
{
  BEGIN_FUNC(FCNAME);
  int origin_contig,target_contig, mpi_errno;
  MPIDI_Datatype_check_contig(origin_datatype,origin_contig);
  MPIDI_Datatype_check_contig(target_datatype,target_contig);

  if(origin_contig && target_contig)
    mpi_errno = do_get_lw(origin_addr,
                          origin_count,
                          origin_datatype,
                          target_rank,
                          target_disp,
                          target_count,
                          target_datatype,
                          win);
  else {
    mpi_errno = do_get(origin_addr,
                       origin_count,
                       origin_datatype,
                       target_rank,
                       target_disp,
                       target_count,
                       target_datatype,
                       win,
                       NULL);
  }

  END_FUNC(FCNAME);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Rget(void          *origin_addr,
                              int            origin_count,
                              MPI_Datatype   origin_datatype,
                              int            target_rank,
                              MPI_Aint       target_disp,
                              int            target_count,
                              MPI_Datatype   target_datatype,
                              MPIR_Win      *win,
                              MPIR_Request **request)
{
  BEGIN_FUNC(FCNAME);
  MPIR_Request  *rreq;
  int mpi_errno = do_get(origin_addr,
                         origin_count,
                         origin_datatype,
                         target_rank,
                         target_disp,
                         target_count,
                         target_datatype,
                         win,
                         &rreq);
  *request               = rreq;
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_accumulate)
__SI__ int do_accumulate(const void    *origin_addr,
                         int            origin_count,
                         MPI_Datatype   origin_datatype,
                         int            target_rank,
                         MPI_Aint       target_disp,
                         int            target_count,
                         MPI_Datatype   target_datatype,
                         MPI_Op         op,
                         MPIR_Win      *win,
                         MPIR_Request **signal)
{
  int                rc,acccheck=0,mpi_errno = MPI_SUCCESS;
  uint64_t           flags=0;
  MPI_Datatype       tt,ot;
  MPIDI_Win_request *req;
  size_t             offset;
  MPIR_Request      *sigreq;
  fid_ep_t           ep;

  BEGIN_FUNC(FCNAME);

  WINREQ_CREATE(req);
  SETUP_SIGNAL_REQUEST();

  EPOCH_CHECK1();

  offset = target_disp * WINFO_DISP_UNIT(win, target_rank);
  ot=origin_datatype;
  tt=target_datatype;

  MPIDI_Win_datatype_basic(origin_count,ot,&req->noncontig->origin_dt);
  MPIDI_Win_datatype_basic(target_count,tt,&req->noncontig->target_dt);
  MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size),
                      mpi_errno,MPI_ERR_SIZE,"**rmasize");

  if((req->noncontig->origin_dt.size == 0) ||
     (target_rank == MPI_PROC_NULL)) {
    MPIDI_Win_request_complete(req);
    if(signal) MPIDI_Request_free_hook(sigreq);
    return MPI_SUCCESS;
  }
  EPOCH_START_CHECK();
  req->target_rank = target_rank;

  MPI_Datatype basic_type;
  GET_BASIC_TYPE(tt, basic_type);
  switch(basic_type) {
    /* 8 byte types */
  case MPI_FLOAT_INT:
  case MPI_2INT:
  case MPI_LONG_INT:
#ifdef HAVE_FORTRAN_BINDING
  case MPI_2REAL:
  case MPI_2INTEGER:
#endif
  {basic_type=tt=ot=MPI_LONG_LONG; acccheck=1; break;}
  /* 16-byte types */
  case MPI_2DOUBLE_PRECISION:
#ifdef MPICH_DEFINE_2COMPLEX
  case MPI_2COMPLEX:
#endif
  {basic_type=tt=ot=MPI_DOUBLE_COMPLEX; acccheck=1; break;}
  /* Types with pads or too large to handle*/
  case MPI_DATATYPE_NULL:
  case MPI_SHORT_INT:
  case MPI_DOUBLE_INT:
  case MPI_LONG_DOUBLE_INT:
#ifdef MPICH_DEFINE_2COMPLEX
  case MPI_2DOUBLE_COMPLEX:
#endif
    MPIR_ERR_SETANDSTMT(mpi_errno,MPI_ERR_TYPE,goto fn_fail,
                        "**rmatypenotatomic");
    break;
  }
  MPIR_ERR_CHKANDSTMT((acccheck && op != MPI_REPLACE),
                      mpi_errno,MPI_ERR_TYPE,
                      goto fn_fail, "**rmatypenotatomic");

  fi_op_t       fi_op;
  fi_datatype_t fi_dt;
  size_t        max_size, dt_size;
  max_size=MPIDI_QUERY_ATOMIC_COUNT;
  MPIDI_Win_datatype_map(&req->noncontig->target_dt);
  MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
  MPIDI_Query_dt(basic_type,&fi_dt,op,&fi_op,&max_size,&dt_size);
  req->callback          = NULL;
  req->next              = WIN_OFI(win)->syncQ;
  WIN_OFI(win)->syncQ    = req;
  max_size = max_size*dt_size;
  MPIR_ERR_CHKANDSTMT((max_size == 0), mpi_errno,MPI_ERR_TYPE,
                      goto fn_fail, "**rmatypenotatomic");

  MPIDI_Init_iovec_state(&req->noncontig->iovs,
                         (uintptr_t)origin_addr,
                         (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                         req->noncontig->origin_dt.num_contig,
                         req->noncontig->target_dt.num_contig,
                         max_size,
                         req->noncontig->origin_dt.map,
                         req->noncontig->target_dt.map);
  msg_atomic_t msg;
  void *desc;
  desc = fi_mr_desc(MPIDI_Global.mr);
  rc = MPIDI_IOV_EAGAIN;
  msg.desc          = &desc;
  msg.addr          = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
  msg.context       = NULL;
  msg.data          = 0;
  msg.datatype      = fi_dt;
  msg.op            = fi_op;
  while(rc==MPIDI_IOV_EAGAIN) {
    iovec_t   originv[1];
    rma_iov_t targetv[1];
    size_t    omax;
    size_t    tmax;
    size_t    tout, oout;
    int i;
    omax=tmax=1;
    rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,originv,omax,
                              targetv,tmax,&oout,&tout);
    if(rc==MPIDI_IOV_DONE) break;
    MPIR_Assert(rc != MPIDI_IOV_ERROR);
    for(i=0; i<tout; i++) targetv[i].key = WINFO_MR_KEY(win,target_rank);
    for(i=0; i<oout; i++)((ioc_t*)&originv[i])->count/=dt_size;
    for(i=0; i<tout; i++)((rma_ioc_t*)&targetv[i])->count/=dt_size;
    msg.msg_iov       = (ioc_t*)originv;
    msg.iov_count     = oout;
    msg.rma_iov       = (rma_ioc_t*)targetv;
    msg.rma_iov_count = tout;
    SETUP_CHUNK_CONTEXT();
    FI_RC_RETRY(fi_atomicmsg(ep, &msg, flags), rdma_atomicto);
  }
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Accumulate)
__ADI_INLINE__ int MPIDI_Accumulate(const void   *origin_addr,
                                    int           origin_count,
                                    MPI_Datatype  origin_datatype,
                                    int           target_rank,
                                    MPI_Aint      target_disp,
                                    int           target_count,
                                    MPI_Datatype  target_datatype,
                                    MPI_Op        op,
                                    MPIR_Win     *win)
{
  BEGIN_FUNC(FCNAME);
  int mpi_errno = do_accumulate(origin_addr,
                                origin_count,
                                origin_datatype,
                                target_rank,
                                target_disp,
                                target_count,
                                target_datatype,
                                op,
                                win,
                                NULL);
  END_FUNC(FCNAME);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Raccumulate(const void    *origin_addr,
                                     int            origin_count,
                                     MPI_Datatype   origin_datatype,
                                     int            target_rank,
                                     MPI_Aint       target_disp,
                                     int            target_count,
                                     MPI_Datatype   target_datatype,
                                     MPI_Op         op,
                                     MPIR_Win      *win,
                                     MPIR_Request **request)
{
  BEGIN_FUNC(FCNAME);
  MPIR_Request *rreq;
  int mpi_errno = do_accumulate((void *)origin_addr,
                                origin_count,
                                origin_datatype,
                                target_rank,
                                target_disp,
                                target_count,
                                target_datatype,
                                op,
                                win,
                                &rreq);
  *request               = rreq;
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_attach)
__ADI_INLINE__ int MPIDI_Win_attach(MPIR_Win *win,
                                    void     *base,
                                    MPI_Aint  size)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  MPIR_ERR_CHKANDSTMT((win->create_flavor != MPI_WIN_FLAVOR_DYNAMIC), mpi_errno,
                      MPI_ERR_RMA_FLAVOR, goto fn_fail, "**rmaflavor");
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_flush_local)
__ADI_INLINE__ int MPIDI_Win_flush_local(int       rank,
                                         MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  EPOCH_LOCK_CHECK();

  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_detach)
__ADI_INLINE__ int MPIDI_Win_detach(MPIR_Win   *win,
                                    const void *base)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  MPIR_ERR_CHKANDSTMT((win->create_flavor != MPI_WIN_FLAVOR_DYNAMIC), mpi_errno,
                      MPI_ERR_RMA_FLAVOR, goto fn_fail, "**rmaflavor");
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_allocate)
__ADI_INLINE__ int MPIDI_Win_allocate(MPI_Aint    size,
                                      int         disp_unit,
                                      MPIR_Info  *info,
                                      MPIR_Comm  *comm,
                                      void       *baseptr,
                                      MPIR_Win  **win_ptr)
{
  int            mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
  BEGIN_FUNC(FCNAME);

  void           *baseP;
  MPIDI_Win_info *winfo;
  MPIR_Win       *win;

  mpi_errno = MPIDI_Win_init(size,disp_unit,win_ptr, info, comm,
                             MPI_WIN_FLAVOR_ALLOCATE, MPI_WIN_UNIFIED);

  if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

  mpi_errno = get_symmetric_heap(size,comm,&baseP,*win_ptr);

  if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

  win              = *win_ptr;
  win->base        =  baseP;
  winfo            = (MPIDI_Win_info *)WINFO(win,comm->rank);
  winfo->base_addr =  baseP;
  winfo->disp_unit =  disp_unit;

  mpi_errno= MPIDI_Win_allgather(win);

  if(mpi_errno != MPI_SUCCESS)
    goto fn_fail;

  *(void **) baseptr = (void *) win->base;
  mpi_errno = MPIR_Barrier_impl(comm, &errflag);

  if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_flush)
__ADI_INLINE__ int MPIDI_Win_flush(int       rank,
                                   MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  EPOCH_LOCK_CHECK();

  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_flush_local_all)
__ADI_INLINE__ int MPIDI_Win_flush_local_all(MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  EPOCH_LOCK_CHECK();
  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_unlock_all)
__ADI_INLINE__ int MPIDI_Win_unlock_all(MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  int i;
  MPIDI_WinLock_info    *lockQ;

  EPOCH_ORIGIN_CHECK(MPID_EPOTYPE_LOCK_ALL);

  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

  MPIR_Assert(WIN_OFI(win)->msgQ != NULL);

  lockQ = (MPIDI_WinLock_info *) WIN_OFI(win)->msgQ;

  for(i = 0; i < win->comm_ptr->local_size; i++) {
    MPIDI_Win_control_t msg;
    lockQ[i].done = 0;
    lockQ[i].peer = i;
    lockQ[i].win  = win;
    msg.type      = MPIDI_CTRL_UNLOCKALL;
    mpi_errno     = do_control_win(&msg, lockQ[i].peer, win, 1);

    if(mpi_errno != MPI_SUCCESS)
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                          goto fn_fail, "**rmasync");

    if(WIN_OFI(win)->sync.lock.remote.allLocked == 1)
      lockQ[i].done = 1;
  }

  PROGRESS_WHILE(WIN_OFI(win)->sync.lock.remote.allLocked);

  WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_NONE;
  WIN_OFI(win)->sync.target_epoch_type = MPID_EPOTYPE_NONE;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_create_dynamic)
__ADI_INLINE__ int MPIDI_Win_create_dynamic(MPIR_Info  *info,
                                            MPIR_Comm  *comm,
                                            MPIR_Win   **win_ptr)
{
  int            mpi_errno = MPI_SUCCESS;
  int            rc        = MPI_SUCCESS;
  MPIR_Errflag_t errflag   = MPIR_ERR_NONE;

  BEGIN_FUNC(FCNAME);

  MPIR_Win       *win;

  rc = MPIDI_Win_init(0,1,win_ptr, info, comm,
                      MPI_WIN_FLAVOR_DYNAMIC,
                      MPI_WIN_UNIFIED);

  if(rc != MPI_SUCCESS)
    goto fn_fail;

  win       = *win_ptr;
  win->base =  MPI_BOTTOM;

  rc = MPIDI_Win_allgather(win);

  if(rc != MPI_SUCCESS)
    goto fn_fail;

  mpi_errno = MPIR_Barrier_impl(comm, &errflag);

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_sync)
__ADI_INLINE__ int MPIDI_Win_sync(MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  EPOCH_LOCK_CHECK();

  OPA_read_write_barrier();

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_flush_all)
__ADI_INLINE__ int MPIDI_Win_flush_all(MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
  EPOCH_LOCK_CHECK();
  MPI_RC_POP(MPIDI_Progress_win_counter_fence(win));

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_lock_all)
__ADI_INLINE__ int MPIDI_Win_lock_all(int massert,
                                      MPIR_Win *win)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  EPOCH_CHECK2();

  int size;
  size = win->comm_ptr->local_size;

  if(!WIN_OFI(win)->msgQ) {
    WIN_OFI(win)->msgQ = (void *) MPL_calloc(size, sizeof(MPIDI_WinLock_info));
    MPIR_Assert(WIN_OFI(win)->msgQ != NULL);
    WIN_OFI(win)->count=0;
  }

  MPIDI_WinLock_info *lockQ;
  lockQ = (MPIDI_WinLock_info *) WIN_OFI(win)->msgQ;
  int i;

  for(i = 0; i < size; i++) {
    MPIDI_Win_control_t msg;

    lockQ[i].done      = 0;
    lockQ[i].peer      = i;
    lockQ[i].win       = win;
    lockQ[i].lock_type = MPI_LOCK_SHARED;

    msg.type           = MPIDI_CTRL_LOCKALLREQ;
    msg.lock_type      = MPI_LOCK_SHARED;
    mpi_errno          = do_control_win(&msg, lockQ[i].peer, lockQ[i].win, 1);

    if(mpi_errno != MPI_SUCCESS)
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_RMA_SYNC,
                          goto fn_fail, "**rmasync");

    if(WIN_OFI(win)->sync.lock.remote.allLocked == 1)
      lockQ[i].done = 1;
  }
  PROGRESS_WHILE(size != WIN_OFI(win)->sync.lock.remote.allLocked);

  WIN_OFI(win)->sync.origin_epoch_type = MPID_EPOTYPE_LOCK_ALL;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(do_get_accumulate)
__SI__ int do_get_accumulate(const void    *origin_addr,
                             int            origin_count,
                             MPI_Datatype   origin_datatype,
                             void          *result_addr,
                             int            result_count,
                             MPI_Datatype   result_datatype,
                             int            target_rank,
                             MPI_Aint       target_disp,
                             int            target_count,
                             MPI_Datatype   target_datatype,
                             MPI_Op         op,
                             MPIR_Win      *win,
                             MPIR_Request **signal)
{
  int                rc, acccheck=0,mpi_errno = MPI_SUCCESS;
  uint64_t           flags=0ULL;
  MPI_Datatype       tt,ot,rt;
  MPIDI_Win_request *req;
  size_t             offset;
  MPIR_Request      *sigreq;
  fid_ep_t           ep;

  BEGIN_FUNC(FCNAME);
  WINREQ_CREATE(req);
  SETUP_SIGNAL_REQUEST();

  EPOCH_CHECK1();

  offset = target_disp * WINFO_DISP_UNIT(win,target_rank);
  ot=origin_datatype;
  tt=target_datatype;
  rt=result_datatype;

  MPIDI_Win_datatype_basic(origin_count,ot,&req->noncontig->origin_dt);
  MPIDI_Win_datatype_basic(target_count,tt,&req->noncontig->target_dt);
  MPIDI_Win_datatype_basic(result_count,rt,&req->noncontig->result_dt);

  MPIR_ERR_CHKANDJUMP((req->noncontig->origin_dt.size != req->noncontig->target_dt.size && op != MPI_NO_OP),
                      mpi_errno, MPI_ERR_SIZE, "**rmasize");
  MPIR_ERR_CHKANDJUMP((req->noncontig->result_dt.size != req->noncontig->target_dt.size),
                      mpi_errno, MPI_ERR_SIZE, "**rmasize");

  if((req->noncontig->result_dt.size == 0) ||
     (target_rank == MPI_PROC_NULL)) {
    MPIDI_Win_request_complete(req);
    if(signal) MPIDI_Request_free_hook(sigreq);
    goto fn_exit;
  }

  EPOCH_START_CHECK();
  req->target_rank     = target_rank;

  MPI_Datatype basic_type;
  GET_BASIC_TYPE(tt, basic_type);
  switch(basic_type) {
    /* 8 byte types */
  case MPI_FLOAT_INT:
  case MPI_2INT:
  case MPI_LONG_INT:
#ifdef HAVE_FORTRAN_BINDING
  case MPI_2REAL:
  case MPI_2INTEGER:
#endif
  {basic_type=tt=ot=rt=MPI_LONG_LONG; acccheck=1; break;}
  /* 16-byte types */
  case MPI_2DOUBLE_PRECISION:
#ifdef MPICH_DEFINE_2COMPLEX
  case MPI_2COMPLEX:
#endif
  {basic_type=tt=ot=rt=MPI_DOUBLE_COMPLEX; acccheck=1; break;}
  /* Types with pads or too large to handle*/
  case MPI_DATATYPE_NULL:
  case MPI_SHORT_INT:
  case MPI_DOUBLE_INT:
  case MPI_LONG_DOUBLE_INT:
#ifdef MPICH_DEFINE_2COMPLEX
  case MPI_2DOUBLE_COMPLEX:
#endif
    MPIR_ERR_SETANDSTMT(mpi_errno,MPI_ERR_TYPE,goto fn_fail,
                        "**rmatypenotatomic");
    break;
  }
  MPIR_ERR_CHKANDSTMT((acccheck && op != MPI_REPLACE && op != MPI_NO_OP),
                      mpi_errno,MPI_ERR_TYPE,
                      goto fn_fail, "**rmatypenotatomic");

  MPI_Datatype basic_type_res;
  GET_BASIC_TYPE(rt, basic_type_res);

  MPIR_Assert(basic_type_res != MPI_DATATYPE_NULL);

  fi_op_t       fi_op;
  fi_datatype_t fi_dt;
  size_t        max_size,dt_size;
  max_size = MPIDI_QUERY_FETCH_ATOMIC_COUNT;
  MPIDI_Win_datatype_map(&req->noncontig->target_dt);
  if(op != MPI_NO_OP) MPIDI_Win_datatype_map(&req->noncontig->origin_dt);
  MPIDI_Win_datatype_map(&req->noncontig->result_dt);
  MPIDI_Query_dt(basic_type_res,&fi_dt,op,&fi_op,&max_size,&dt_size);
  req->callback       = rma_done_callback;
  req->next           = WIN_OFI(win)->syncQ;
  WIN_OFI(win)->syncQ = req;


  max_size = max_size*dt_size;
  MPIR_ERR_CHKANDSTMT((max_size == 0), mpi_errno,MPI_ERR_TYPE,
                      goto fn_fail, "**rmatypenotatomic");

  if(op != MPI_NO_OP)
    MPIDI_Init_iovec_state2(&req->noncontig->iovs,
                            (uintptr_t)origin_addr,
                            (uintptr_t)result_addr,
                            (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                            req->noncontig->origin_dt.num_contig,
                            req->noncontig->result_dt.num_contig,
                            req->noncontig->target_dt.num_contig,
                            max_size,
                            req->noncontig->origin_dt.map,
                            req->noncontig->result_dt.map,
                            req->noncontig->target_dt.map);
  else
    MPIDI_Init_iovec_state(&req->noncontig->iovs,
                           (uintptr_t)result_addr,
                           (uintptr_t)WINFO_BASE(win,req->target_rank) + offset,
                           req->noncontig->result_dt.num_contig,
                           req->noncontig->target_dt.num_contig,
                           max_size,
                           req->noncontig->result_dt.map,
                           req->noncontig->target_dt.map);
  msg_atomic_t  msg;
  void *desc;
  desc = fi_mr_desc(MPIDI_Global.mr);
  rc = MPIDI_IOV_EAGAIN;
  msg.desc          = &desc;
  msg.addr          = _comm_to_phys(win->comm_ptr,req->target_rank,MPIDI_API_CTR);
  msg.context       = NULL;
  msg.data          = 0;
  msg.datatype      = fi_dt;
  msg.op            = fi_op;
  while(rc==MPIDI_IOV_EAGAIN) {
    iovec_t   originv[1]={0};
    iovec_t   resultv[1]={0};
    rma_iov_t targetv[1]={0};
    size_t    omax,rmax,tmax;
    size_t    tout,rout,oout;
    omax=rmax=tmax=1;
    if(op != MPI_NO_OP)
      rc = MPIDI_Merge_iov_list2(&req->noncontig->iovs,originv,
                                 omax,resultv,rmax,targetv,tmax,
                                 &oout,&rout,&tout);
    else
    {
      oout = 0;
      rc = MPIDI_Merge_iov_list(&req->noncontig->iovs,resultv,
                                rmax,targetv,tmax,&rout,&tout);
    }
    if(rc==MPIDI_IOV_DONE) break;
    MPIR_Assert(rc != MPIDI_IOV_ERROR);
    int i;
    for(i=0; i<oout; i++)((ioc_t*)&originv[i])->count/=dt_size;
    for(i=0; i<rout; i++)((rma_ioc_t*)&resultv[i])->count/=dt_size;
    for(i=0; i<tout; i++)
    {
      ((rma_ioc_t*)&targetv[i])->count/=dt_size;
      ((rma_ioc_t*)&targetv[i])->key = WINFO_MR_KEY(win,target_rank);
    }
    msg.msg_iov       = (ioc_t*)originv;
    msg.iov_count     = oout;
    msg.rma_iov       = (rma_ioc_t*)targetv;
    msg.rma_iov_count = tout;
    SETUP_CHUNK_CONTEXT();
    FI_RC_RETRY(fi_fetch_atomicmsg(ep, &msg,(ioc_t*)resultv,
                                   NULL,rout,flags), rdma_readfrom);
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Get_accumulate)
__ADI_INLINE__ int MPIDI_Get_accumulate(const void   *origin_addr,
                                        int           origin_count,
                                        MPI_Datatype  origin_datatype,
                                        void         *result_addr,
                                        int           result_count,
                                        MPI_Datatype  result_datatype,
                                        int           target_rank,
                                        MPI_Aint      target_disp,
                                        int           target_count,
                                        MPI_Datatype  target_datatype,
                                        MPI_Op        op,
                                        MPIR_Win     *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  mpi_errno = do_get_accumulate(origin_addr, origin_count, origin_datatype,
                                result_addr, result_count, result_datatype,
                                target_rank, target_disp, target_count,
                                target_datatype, op, win, NULL);
  END_FUNC(FCNAME);
  return mpi_errno;
}

__ADI_INLINE__ int MPIDI_Rget_accumulate(const void    *origin_addr,
                                         int            origin_count,
                                         MPI_Datatype   origin_datatype,
                                         void          *result_addr,
                                         int            result_count,
                                         MPI_Datatype   result_datatype,
                                         int            target_rank,
                                         MPI_Aint       target_disp,
                                         int            target_count,
                                         MPI_Datatype   target_datatype,
                                         MPI_Op         op,
                                         MPIR_Win      *win,
                                         MPIR_Request **request)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  MPIR_Request *rreq;
  mpi_errno = do_get_accumulate(origin_addr, origin_count, origin_datatype,
                                result_addr, result_count, result_datatype,
                                target_rank, target_disp, target_count,
                                target_datatype, op, win, &rreq);
  *request = rreq;
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Fetch_and_op)
__ADI_INLINE__ int MPIDI_Fetch_and_op(const void   *origin_addr,
                                      void         *result_addr,
                                      MPI_Datatype  datatype,
                                      int           target_rank,
                                      MPI_Aint      target_disp,
                                      MPI_Op        op,
                                      MPIR_Win     *win)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  /*  This can be optimized by directly calling the fi directly
   *  and avoiding all the datatype processing of the full
   *  MPIDI_Get_accumulate
   */
  mpi_errno = do_get_accumulate(origin_addr,1,datatype,
                                result_addr,1,datatype,
                                target_rank,target_disp,1,
                                datatype,op,win,NULL);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Compare_and_swap)
__ADI_INLINE__ int MPIDI_Compare_and_swap(const void   *origin_addr,
                                          const void   *compare_addr,
                                          void         *result_addr,
                                          MPI_Datatype  datatype,
                                          int           target_rank,
                                          MPI_Aint      target_disp,
                                          MPIR_Win     *win)
{
  int mpi_errno = MPI_SUCCESS;
  fi_op_t fi_op;
  fi_datatype_t fi_dt;
  BEGIN_FUNC(FCNAME);
  EPOCH_CHECK1();

  MPIDI_Win_dt origin_dt, target_dt, result_dt;
  size_t       offset;

  offset = target_disp * WINFO_DISP_UNIT(win,target_rank);

  MPIDI_Win_datatype_basic(1,datatype,&origin_dt);
  MPIDI_Win_datatype_basic(1,datatype,&result_dt);
  MPIDI_Win_datatype_basic(1,datatype,&target_dt);

  if((origin_dt.size==0) ||(target_rank==MPI_PROC_NULL))
    goto fn_exit;

  void *buffer, *tbuffer, *rbuffer;
  buffer  = (char *)origin_addr + origin_dt.true_lb;
  rbuffer = (char *)result_addr + result_dt.true_lb;
  tbuffer = (char *)WINFO_BASE(win,target_rank) + offset;

  EPOCH_START_CHECK();
  size_t max_size,dt_size;
  max_size=MPIDI_QUERY_COMPARE_ATOMIC_COUNT;
  MPIDI_Query_dt(datatype,&fi_dt,MPI_OP_NULL,&fi_op,&max_size,&dt_size);

  ioc_t     originv, resultv, comparev;
  rma_ioc_t targetv;
  originv.addr   = (void*)buffer;
  originv.count  = 1;
  resultv.addr   = (void*)rbuffer;
  resultv.count  = 1;
  comparev.addr  = (void*)compare_addr;
  comparev.count = 1;
  targetv.addr   = (uint64_t)tbuffer;
  targetv.count  = 1;
  targetv.key    = WINFO_MR_KEY(win,target_rank);;

  msg_atomic_t msg;
  void *desc;
  desc = fi_mr_desc(MPIDI_Global.mr);
  msg.msg_iov       = &originv;
  msg.desc      = &desc;
  msg.iov_count     = 1;
  msg.addr          = _comm_to_phys(win->comm_ptr,target_rank,MPIDI_API_CTR);
  msg.rma_iov       = (rma_ioc_t*)&targetv;
  msg.rma_iov_count = 1;
  msg.datatype      = fi_dt;
  msg.op            = fi_op;
  msg.context       = NULL;
  msg.data          = 0;
  MPIDI_Global.cntr++;
  FI_RC_RETRY(fi_compare_atomicmsg(G_TXC_CTR(0),&msg,
                                   &comparev,NULL,1,
                                   &resultv,NULL,1,0),atomicto);
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_allocate_shared)
__ADI_INLINE__ int MPIDI_Win_allocate_shared(MPI_Aint     size,
                                             int          disp_unit,
                                             MPIR_Info   *info_ptr,
                                             MPIR_Comm   *comm_ptr,
                                             void       **base_ptr,
                                             MPIR_Win   **win_ptr)
{
  int            mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
  BEGIN_FUNC(FCNAME);

  void           *baseP      = NULL;
  MPIDI_Win_info *winfo      = NULL;
  MPIR_Win       *win        = NULL;
  ssize_t         total_size = 0LL;
  int             i          = 0;

  mpi_errno = MPIDI_Win_init(size,disp_unit,win_ptr,info_ptr,comm_ptr,
                             MPI_WIN_FLAVOR_SHARED, MPI_WIN_UNIFIED);

  win                   = *win_ptr;
  WIN_OFI(win)->sizes   = (MPI_Aint *)MPL_malloc(sizeof(MPI_Aint)*comm_ptr->local_size);
  MPI_Aint     *sizes   = WIN_OFI(win)->sizes;
  sizes[comm_ptr->rank] = size;
  mpi_errno             = MPIR_Allgather_impl(MPI_IN_PLACE,
                                              0,
                                              MPI_DATATYPE_NULL,
                                              sizes,
                                              sizeof(MPI_Aint),
                                              MPI_BYTE,
                                              comm_ptr,
                                              &errflag);

  if(mpi_errno != MPI_SUCCESS) goto fn_fail;

  /* No allreduce here because this is a shared memory domain
   * and should be a relatively small number of processes
   * and a non performance sensitive API.
   */
  for(i=0; i<comm_ptr->local_size; i++)
    total_size+=sizes[i];

  if(total_size == 0) goto fn_zero;

  int  fd, rc, first;
  char shm_key[64];
  void *map_ptr;
  sprintf(shm_key, "/mpi-%X-%lX",
          MPIDI_Global.jobid,
          WIN_OFI(win)->win_id);

  rc    = shm_open(shm_key,
                   O_CREAT|O_EXCL|O_RDWR,
                   0600);
  first = (rc != -1);

  if(!first) {
    rc = shm_open(shm_key, O_RDWR, 0);

    if(rc == -1) {
      shm_unlink(shm_key);
      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                          goto fn_fail, "**nomem");
    }
  }

  /* Make the addresses symmetric by using MAP_FIXED */
  size_t page_sz, mapsize;

  mapsize = get_mapsize(total_size, &page_sz);
  fd      = rc;
  rc      = ftruncate(fd, mapsize);

  if(rc == -1) {
    close(fd);

    if(first)shm_unlink(shm_key);

    MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                        goto fn_fail, "**nomem");
  }

  if(comm_ptr->rank == 0) {
    map_ptr = generate_random_addr(mapsize);
    map_ptr = mmap(map_ptr, mapsize,
                   PROT_READ|PROT_WRITE,
                   MAP_SHARED|MAP_FIXED,
                   fd, 0);

    if(map_ptr == NULL || map_ptr == MAP_FAILED) {
      close(fd);

      if(first)shm_unlink(shm_key);

      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                          goto fn_fail, "**nomem");
    }

    mpi_errno = MPIR_Bcast_impl(&map_ptr,1,MPI_UNSIGNED_LONG,
                                0,comm_ptr,&errflag);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    WIN_OFI(win)->mmap_addr = map_ptr;
    WIN_OFI(win)->mmap_sz   = mapsize;
  } else {
    mpi_errno = MPIR_Bcast_impl(&map_ptr,1,MPI_UNSIGNED_LONG,
                                0,comm_ptr,&errflag);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    rc = check_maprange_ok(map_ptr,mapsize);
    /* If we hit this assert, we need to iterate
     * trying more addresses
     */
    MPIR_Assert(rc == 1);
    map_ptr = mmap(map_ptr, mapsize,
                   PROT_READ|PROT_WRITE,
                   MAP_SHARED|MAP_FIXED,
                   fd, 0);
    WIN_OFI(win)->mmap_addr = map_ptr;
    WIN_OFI(win)->mmap_sz   = mapsize;

    if(map_ptr == NULL || map_ptr == MAP_FAILED) {
      close(fd);

      if(first)shm_unlink(shm_key);

      MPIR_ERR_SETANDSTMT(mpi_errno, MPI_ERR_NO_MEM,
                          goto fn_fail, "**nomem");
    }
  }

  /* Scan for my offset into the buffer             */
  /* Could use exscan if this is expensive at scale */
  MPI_Aint size_out;
  size_out = 0;

  for(i=0; i<comm_ptr->rank; i++)
    size_out+=sizes[i];

fn_zero:

  baseP            = (size==0)?NULL:(void *)((char *)map_ptr + size_out);
  win->base        =  baseP;
  win->size        =  size;

  winfo            = (MPIDI_Win_info *)WINFO(win,comm_ptr->rank);
  winfo->base_addr = baseP;
  winfo->disp_unit = disp_unit;
  mpi_errno        = MPIDI_Win_allgather(win);

  if(mpi_errno != MPI_SUCCESS)
    return mpi_errno;

  *(void **) base_ptr = (void *) win->base;
  mpi_errno = MPIR_Barrier_impl(comm_ptr, &errflag);

  close(fd);

  if(first) shm_unlink(shm_key);

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Win_shared_query)
__ADI_INLINE__ int MPIDI_Win_shared_query(MPIR_Win *win,
                                          int       rank,
                                          MPI_Aint *size,
                                          int      *disp_unit,
                                          void     *baseptr)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  int offset = rank;

  if(rank < 0)
    offset = 0;

  *(void **)baseptr = WINFO_BASE(win, offset);
  *size             = WIN_OFI(win)->sizes[offset];
  *disp_unit        = WINFO_DISP_UNIT(win,offset);

  END_FUNC(FCNAME);
  return mpi_errno;
}
