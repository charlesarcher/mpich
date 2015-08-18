/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#if !defined(COLL_LOADER_H_INCLUDED)
#define COLL_LOADER_H_INCLUDED
#include <dlfcn.h>
#include <stdio.h>
#include "coll.h"

#ifdef USE_DYNAMIC_COLL

static const char *COLL_cmd_list[] = {
  "COLLI_Init", "COLLI_Comm_init", "COLLI_Allgather","COLLI_Allgatherv",
  "COLLI_Allreduce","COLLI_Alltoall","COLLI_Alltoallv","COLLI_Alltoallw",
  "COLLI_Bcast","COLLI_Exscan","COLLI_Gather","COLLI_Gatherv",
  "COLLI_Reduce_scatter","COLLI_Reduce_scatter_block","COLLI_Reduce","COLLI_Scan",
  "COLLI_Scatter","COLLI_Scatterv","COLLI_Barrier","COLLI_Iallgather",
  "COLLI_Iallgatherv","COLLI_Iallreduce","COLLI_Ialltoall","COLLI_Ialltoallv",
  "COLLI_Ialltoallw","COLLI_Ibcast","COLLI_Iexscan","COLLI_Igather","COLLI_Igatherv",
  "COLLI_Ireduce_scatter","COLLI_Ireduce_scatter_block","COLLI_Ireduce",
  "COLLI_Iscan","COLLI_Iscatter","COLLI_Iscatterv","COLLI_Ibarrier",
  "COLLI_Neighbor_allgather","COLLI_Neighbor_allgatherv","COLLI_Neighbor_alltoall",
  "COLLI_Neighbor_alltoallv","COLLI_Neighbor_alltoallw","COLLI_Ineighbor_allgather",
  "COLLI_Ineighbor_allgatherv","COLLI_Ineighbor_alltoall","COLLI_Ineighbor_alltoallv",
  "COLLI_Ineighbor_alltoallw","COLLI_Kick",
};

enum {
  I_COLLI_Init=0,
  I_COLLI_Comm_init,I_COLLI_Allgather,I_COLLI_Allgatherv,I_COLLI_Allreduce,
  I_COLLI_Alltoall,I_COLLI_Alltoallv,I_COLLI_Alltoallw,I_COLLI_Bcast,
  I_COLLI_Exscan,I_COLLI_Gather,I_COLLI_Gatherv,I_COLLI_Reduce_scatter,
  I_COLLI_Reduce_scatter_block,I_COLLI_Reduce,I_COLLI_Scan,I_COLLI_Scatter,
  I_COLLI_Scatterv,I_COLLI_Barrier,I_COLLI_Iallgather,I_COLLI_Iallgatherv,
  I_COLLI_Iallreduce,I_COLLI_Ialltoall,I_COLLI_Ialltoallv,I_COLLI_Ialltoallw,
  I_COLLI_Ibcast,I_COLLI_Iexscan,I_COLLI_Igather,I_COLLI_Igatherv,
  I_COLLI_Ireduce_scatter,I_COLLI_Ireduce_scatter_block,I_COLLI_Ireduce,
  I_COLLI_Iscan,I_COLLI_Iscatter,I_COLLI_Iscatterv,I_COLLI_Ibarrier,
  I_COLLI_Neighbor_allgather,I_COLLI_Neighbor_allgatherv,
  I_COLLI_Neighbor_alltoall,I_COLLI_Neighbor_alltoallv,
  I_COLLI_Neighbor_alltoallw,I_COLLI_Ineighbor_allgather,
  I_COLLI_Ineighbor_allgatherv,I_COLLI_Ineighbor_alltoall,
  I_COLLI_Ineighbor_alltoallv,I_COLLI_Ineighbor_alltoallw,I_COLLI_Kick,
};

static inline void *do_import(void       *dlopen_file,
                              const char *funcname,
                              const char *component)
{
  dlerror();    /* Clear any existing error */
  void *handle = NULL;
  char *error  = NULL;
  handle       = dlsym(dlopen_file, funcname);

  if((error = dlerror()) != NULL) {
    fprintf(stderr, "Error when loading %s function %s: %s\n",
            component, funcname, error);
    handle = NULL;
  }

  return handle;
}

#define COLL_LDTABLE(x,y) table->x = (x##_fn)do_import(dlopen_obj,COLL_cmd_list[I_COLLI_##y],"Collectives")
static inline int coll_load(struct CollFunc *table)
{
  void            *dlopen_obj  = NULL;
  char            *filename    = (char *)"libcolldefault.so";
  char            *c           = getenv("COLL_MODULE");

  if(!c) c = filename;

  dlopen_obj = dlopen(c, RTLD_NOW | RTLD_GLOBAL);

  if(NULL == dlopen_obj) {
    fprintf(stderr, "Error opening %s: %s\n", filename, dlerror());
    return -1;
  }

  COLL_LDTABLE(Init,Init);
  COLL_LDTABLE(Comm_init,Comm_init);
  COLL_LDTABLE(Allgather,Allgather);
  COLL_LDTABLE(Allgatherv,Allgatherv);
  COLL_LDTABLE(Allreduce,Allreduce);
  COLL_LDTABLE(Alltoall,Alltoall);
  COLL_LDTABLE(Alltoallv,Alltoallv);
  COLL_LDTABLE(Alltoallw,Alltoallw);
  COLL_LDTABLE(Bcast,Bcast);
  COLL_LDTABLE(Exscan,Exscan);
  COLL_LDTABLE(Gather,Gather);
  COLL_LDTABLE(Gatherv,Gatherv);
  COLL_LDTABLE(Reduce_scatter,Reduce_scatter);
  COLL_LDTABLE(Reduce_scatter_block,Reduce_scatter_block);
  COLL_LDTABLE(Reduce,Reduce);
  COLL_LDTABLE(Scan,Scan);
  COLL_LDTABLE(Scatter,Scatter);
  COLL_LDTABLE(Scatterv,Scatterv);
  COLL_LDTABLE(Barrier,Barrier);
  COLL_LDTABLE(Iallgather,Iallgather);
  COLL_LDTABLE(Iallgatherv,Iallgatherv);
  COLL_LDTABLE(Iallreduce,Iallreduce);
  COLL_LDTABLE(Ialltoall,Ialltoall);
  COLL_LDTABLE(Ialltoallv,Ialltoallv);
  COLL_LDTABLE(Ialltoallw,Ialltoallw);
  COLL_LDTABLE(Ibcast,Ibcast);
  COLL_LDTABLE(Iexscan,Iexscan);
  COLL_LDTABLE(Igather,Igather);
  COLL_LDTABLE(Igatherv,Igatherv);
  COLL_LDTABLE(Ireduce_scatter,Ireduce_scatter);
  COLL_LDTABLE(Ireduce_scatter_block,Ireduce_scatter_block);
  COLL_LDTABLE(Ireduce,Ireduce);
  COLL_LDTABLE(Iscan,Iscan);
  COLL_LDTABLE(Iscatter,Iscatter);
  COLL_LDTABLE(Iscatterv,Iscatterv);
  COLL_LDTABLE(Ibarrier,Ibarrier);
  COLL_LDTABLE(Neighbor_allgather,Neighbor_allgather);
  COLL_LDTABLE(Neighbor_allgatherv,Neighbor_allgatherv);
  COLL_LDTABLE(Neighbor_alltoall,Neighbor_alltoall);
  COLL_LDTABLE(Neighbor_alltoallv,Neighbor_alltoallv);
  COLL_LDTABLE(Neighbor_alltoallw,Neighbor_alltoallw);
  COLL_LDTABLE(Ineighbor_allgather,Ineighbor_allgather);
  COLL_LDTABLE(Ineighbor_allgatherv,Ineighbor_allgatherv);
  COLL_LDTABLE(Ineighbor_alltoall,Ineighbor_alltoall);
  COLL_LDTABLE(Ineighbor_alltoallv,Ineighbor_alltoallv);
  COLL_LDTABLE(Ineighbor_alltoallw,Ineighbor_alltoallw);
  COLL_LDTABLE(Kick,Kick);
  return 0;
}

static const char *TRANSPORT_cmd_list[] = {
  "TRANSPORTI_t_init",        "TRANSPORTI_t_pack",         "TRANSPORTI_t_unpack",
  "TRANSPORTI_t_isinplace",   "TRANSPORTI_t_send",         "TRANSPORTI_t_recv",
  "TRANSPORTI_t_recvreduce",  "TRANSPORTI_t_test",         "TRANSPORTI_t_rank",
  "TRANSPORTI_t_size",        "TRANSPORTI_t_reduce_local", "TRANSPORTI_t_dtinfo",
  "TRANSPORTI_t_dtcopy",      "TRANSPORTI_t_opinfo",       "TRANSPORTI_t_addref_op",
  "TRANSPORTI_t_addref_dt",   "TRANSPORTI_t_malloc",       "TRANSPORTI_t_free",
  "TRANSPORTI_t_reqcomplete", "TRANSPORTI_t_reqalloc",
};

enum {
  I_TRANSPORTI_t_init = 0,    I_TRANSPORTI_t_pack,         I_TRANSPORTI_t_unpack,
  I_TRANSPORTI_t_isinplace,   I_TRANSPORTI_t_send,         I_TRANSPORTI_t_recv,
  I_TRANSPORTI_t_recvreduce,  I_TRANSPORTI_t_test,         I_TRANSPORTI_t_rank,
  I_TRANSPORTI_t_size,        I_TRANSPORTI_t_reduce_local, I_TRANSPORTI_t_dtinfo,
  I_TRANSPORTI_t_dtcopy,      I_TRANSPORTI_t_opinfo,       I_TRANSPORTI_t_addref_op,
  I_TRANSPORTI_t_addref_dt,   I_TRANSPORTI_t_malloc,       I_TRANSPORTI_t_free,
  I_TRANSPORTI_t_reqcomplete, I_TRANSPORTI_t_reqalloc,
};


#define TRANSPORT_LDTABLE(x) table->x = \
    (x##_fn)do_import(dlopen_obj, TRANSPORT_cmd_list[I_TRANSPORTI_##x], "Transport")
static inline int transport_load(struct TransportFunc *table)
{
  void            *dlopen_obj  = NULL;
  char            *filename    = (char *)"libtspmpich.so";
  char            *c           = getenv("TRANSPORT_MODULE");

  if(!c) c = filename;

  dlopen_obj = dlopen(c, RTLD_NOW | RTLD_GLOBAL);

  if(NULL == dlopen_obj) {
    fprintf(stderr, "Error opening %s: %s\n", filename, dlerror());
    return -1;
  }

  TRANSPORT_LDTABLE(t_init);
  TRANSPORT_LDTABLE(t_pack);
  TRANSPORT_LDTABLE(t_unpack);
  TRANSPORT_LDTABLE(t_isinplace);
  TRANSPORT_LDTABLE(t_send);
  TRANSPORT_LDTABLE(t_recv);
  TRANSPORT_LDTABLE(t_recvreduce);
  TRANSPORT_LDTABLE(t_test);
  TRANSPORT_LDTABLE(t_rank);
  TRANSPORT_LDTABLE(t_size);
  TRANSPORT_LDTABLE(t_reduce_local);
  TRANSPORT_LDTABLE(t_dtinfo);
  TRANSPORT_LDTABLE(t_dtcopy);
  TRANSPORT_LDTABLE(t_opinfo);
  TRANSPORT_LDTABLE(t_addref_op);
  TRANSPORT_LDTABLE(t_addref_dt);
  TRANSPORT_LDTABLE(t_malloc);
  TRANSPORT_LDTABLE(t_free);
  TRANSPORT_LDTABLE(t_reqcomplete);
  TRANSPORT_LDTABLE(t_reqalloc);
  return 0;
}

#else

#define coll_load(tbl)
#define transport_load(tbl)

#include "coll_inliner.h"

#endif /* USE_COLL_DYNAMIC */


#endif /* COLL_H_INCLUDED */
