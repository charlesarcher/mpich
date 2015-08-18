/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef COLLTREE_COLL_H_INCLUDED
#define COLLTREE_COLL_H_INCLUDED

#ifdef USE_DYNAMIC_COLL
#define TRANSPORT_OBJ CollTree_g.func
#endif

#include "../../coll.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>

/*#define USE_KNOMIAL*/
#define TREE_RADIX 2
#define MAX_TREE_BREADTH 16
#define MAX_PHASES 32

enum {
  PHASE_INVAL,
  PHASE_BUSY,
  PHASE_DONE
};

typedef union {
  struct comm {
    void       *buf;
    int         count;
    COLL_DT     datatype;
    int         dest;
    int         tag;
    COLL_COMM   comm;
    COLL_REQ   *request;
  } comm;
  struct reduce_local {
    const void *inbuf;
    void       *inoutbuf;
    int         count;
    COLL_DT     datatype;
    COLL_OP     op;
  } reduce_local;
  struct dtcopy {
    void       *tobuf;
    int         tocount;
    COLL_DT     totype;
    const void *frombuf;
    int         fromcount;
    COLL_DT     fromtype;
  } dtcopy;
  struct fence {
    COLL_REQ *startReq;
    int       count;
  } fence;
  struct freebuf {
    void *buffer;
  } freebuf;
  struct freeop {
    COLL_OP op;
  } freeop;
  struct freedt {
    COLL_DT dt;
  } freedt;
} Command;

struct Phase;
typedef int (*CommandFunc)(struct Phase *p);

typedef struct Phase {
  CommandFunc go;
  Command     op;
} Phase;

typedef struct PhaseArray {
  TAILQ_ENTRY(PhaseArray) list_data;
  int      totalPhases;
  int      curPhase;
  int      curRequest;
  int      lastFence;
  Phase    phases[MAX_PHASES];
  COLL_REQ requests[MAX_PHASES];
  COLL_REQ collreq;
} PhaseArray;
#define REQSTRUCT(req) ((PhaseArray**) ((((char*)req)+CollTree_g.req_offset)))

typedef struct GlobalData {
  TAILQ_HEAD(PhaseQ, PhaseArray)  head;
  struct PhaseQ                  *headp;
  TransportFunc                   func;
  int                             comm_offset;
  int                             comm_sizeof;
  int                             req_offset;
  int                             req_sizeof;
  int                            *do_progress;
  COLL_DT                         control_dt;
} GlobalData;
extern GlobalData CollTree_g;


typedef struct ChildRange {
  int startRank;
  int endRank;
} ChildRange;

typedef struct CollTree {
  int        rank;
  int        nranks;
  int        parent;
  int        numRanges;
  ChildRange children[MAX_TREE_BREADTH];
} CollTree;

typedef struct Communicator {
  CollTree tree;
  int      curTag;
} Communicator;
#define COMMSTRUCT(comm) ((Communicator*) ((((char*)comm)+CollTree_g.comm_offset)))

#define PHASE_SCHED_SEND(SCHED,BUF,COUNT,DATATYPE,DEST,TAG,COMM,GOFUNC) \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.comm.buf      =  BUF;                           \
  (SCHED)->phases[C].op.comm.count    =  COUNT;                         \
  (SCHED)->phases[C].op.comm.datatype =  DATATYPE;                      \
  (SCHED)->phases[C].op.comm.dest     =  DEST;                          \
  (SCHED)->phases[C].op.comm.tag      =  TAG;                           \
  (SCHED)->phases[C].op.comm.comm     =  COMM;                          \
  (SCHED)->phases[C].op.comm.request  = &(SCHED)->requests[(SCHED)->curRequest]; \
  (SCHED)->phases[C].go               =  GOFUNC;                        \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
  (SCHED)->curRequest++;                                                \
})

#define PHASE_SCHED_RECV_NOPOST(SCHED,BUF,COUNT,DATATYPE,DEST,TAG,COMM,GOFUNC) \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.comm.buf      =  BUF;                           \
  (SCHED)->phases[C].op.comm.count    =  COUNT;                         \
  (SCHED)->phases[C].op.comm.datatype =  DATATYPE;                      \
  (SCHED)->phases[C].op.comm.dest     =  DEST;                          \
  (SCHED)->phases[C].op.comm.tag      =  TAG;                           \
  (SCHED)->phases[C].op.comm.comm     =  COMM;                          \
  (SCHED)->phases[C].op.comm.request  = &(SCHED)->requests[(SCHED)->curRequest]; \
  (SCHED)->phases[C].go               =  GOFUNC;                        \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
  (SCHED)->curRequest++;                                                \
})

#define PHASE_SCHED_RECV(SCHED,BUF,COUNT,DATATYPE,DEST,TAG,COMM,GOFUNC) \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  TRANSPORT_t_recv(BUF,COUNT,DATATYPE,DEST,TAG,COMM,                    \
                 &(SCHED)->requests[(SCHED)->curRequest]);              \
  (SCHED)->phases[C].go               =  GOFUNC;                        \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
  (SCHED)->curRequest++;                                                \
})

#define PHASE_SCHED_RECVREDUCE(SCHED,BUF,COUNT,DATATYPE,OP,DEST,TAG,COMM,GOFUNC) \
({                                                                               \
  int C = (SCHED)->totalPhases;                                                  \
  TRANSPORT_t_recvreduce(BUF,COUNT,DATATYPE,OP,DEST,TAG,COMM,                    \
                         &(SCHED)->requests[(SCHED)->curRequest]);               \
  (SCHED)->phases[C].go               =  GOFUNC;                                 \
  (SCHED)->totalPhases++;                                                        \
  assert((SCHED)->totalPhases<MAX_PHASES);                                       \
  (SCHED)->curRequest++;                                                         \
})


#define PHASE_REDUCE_LOCAL(SCHED,INBUF,INOUTBUF,COUNT,DATATYPE,OP,GOFUNC) \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.reduce_local.inbuf    =  INBUF;                 \
  (SCHED)->phases[C].op.reduce_local.inoutbuf =  INOUTBUF;              \
  (SCHED)->phases[C].op.reduce_local.count    =  COUNT;                 \
  (SCHED)->phases[C].op.reduce_local.datatype =  DATATYPE;              \
  (SCHED)->phases[C].op.reduce_local.op       =  OP;                    \
  (SCHED)->phases[C].go                       =  GOFUNC;                \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
})
#define PHASE_DTCOPY(SCHED,TOBUF,TOCOUNT,TOTYPE,FROMBUF,FROMCOUNT,FROMTYPE,GOFUNC) \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.dtcopy.tobuf     =  TOBUF;                      \
  (SCHED)->phases[C].op.dtcopy.tocount   =  TOCOUNT;                    \
  (SCHED)->phases[C].op.dtcopy.totype    =  TOTYPE;                     \
  (SCHED)->phases[C].op.dtcopy.frombuf   =  FROMBUF;                    \
  (SCHED)->phases[C].op.dtcopy.fromcount =  FROMCOUNT;                  \
  (SCHED)->phases[C].op.dtcopy.fromtype  =  FROMTYPE;                   \
  (SCHED)->phases[C].go                  =  GOFUNC;                     \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
})

#define PHASE_FENCE(SCHED,GOFUNC)                                       \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.fence.startReq = &(SCHED)->requests[(SCHED)->lastFence]; \
  (SCHED)->phases[C].op.fence.count    = (SCHED)->curRequest - (SCHED)->lastFence; \
  (SCHED)->phases[C].go                = GOFUNC;                        \
  (SCHED)->lastFence                   = (SCHED)->curRequest;           \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
})

#define PHASE_FREEBUF(SCHED,BUFFER,GOFUNC)                              \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.freebuf.buffer = BUFFER;                        \
  (SCHED)->phases[C].go                = GOFUNC;                        \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
})

#define PHASE_FREEDT(SCHED,DT,GOFUNC)                                   \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.freedt.dt = DT;                                 \
  (SCHED)->phases[C].go           = GOFUNC;                             \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
})

#define PHASE_FREEOP(SCHED,OP,GOFUNC)                                   \
({                                                                      \
  int C = (SCHED)->totalPhases;                                         \
  (SCHED)->phases[C].op.freeop.op = OP;                                 \
  (SCHED)->phases[C].go           = GOFUNC;                             \
  (SCHED)->totalPhases++;                                               \
  assert((SCHED)->totalPhases<MAX_PHASES);                              \
})

#define PHASE_KICK(SCHED)                                       \
({                                                              \
  int rc,i,nphases = (SCHED)->totalPhases;                      \
  for(i=0; i<nphases;)                                          \
    {                                                           \
      rc = (SCHED)->phases[i].go(&(SCHED)->phases[i]);          \
      if(rc != PHASE_BUSY)i++;                                  \
    }                                                           \
})

#define PHASE_KICK_NB(SCHED,REQUEST)                                    \
({                                                                      \
  int rc,i,nphases = (SCHED)->totalPhases;                              \
  int _done;                                                            \
  for(i=(SCHED)->curPhase; i<nphases;)                                  \
    {                                                                   \
      rc = (SCHED)->phases[i].go(&(SCHED)->phases[i]);                  \
      if(rc == PHASE_BUSY)break;                                        \
      else i++;                                                         \
    }                                                                   \
  (SCHED)->curPhase=i;                                                  \
  if((SCHED)->curPhase == (SCHED)->totalPhases)                         \
    {                                                                   \
      TRANSPORT_t_reqcomplete(REQUEST);                                 \
      _done = 1;                                                        \
    }                                                                   \
  else                                                                  \
    _done = 0;                                                          \
  _done;                                                                \
})

#define PHASE_INIT(SCHED) memset(SCHED, 0, sizeof(*SCHED));
#define PHASE_INIT_NB(SCHED,SCHEDPTR,REQUEST)                           \
({                                                                      \
  TRANSPORT_t_reqalloc(REQUEST);                                        \
  SCHEDPTR = REQSTRUCT(*REQUEST);                                       \
  *SCHEDPTR = SCHED = (PhaseArray*)TRANSPORT_t_malloc(sizeof(*SCHED));  \
  memset(SCHED,0, sizeof(*SCHED));                                      \
  SCHED->collreq = *REQUEST;                                            \
})

#define FOREACHCHILD()                          \
  for(i=0; i<numRanges; i++)                    \
    for(j=tree->children[i].startRank;          \
        j<=tree->children[i].endRank;           \
        j++)                                    \

#define FOREACHCHILDDO(stmt)                    \
({                                              \
  FOREACHCHILD()                                \
    {                                           \
      stmt;                                     \
    }                                           \
})

#endif /* COLLTREE_COLL_H_INCLUDED */
