/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#include "./coll.h"

static inline int phase_send(Phase *p)
{
  TRANSPORT_t_send(p->op.comm.buf,
                   p->op.comm.count,
                   p->op.comm.datatype,
                   p->op.comm.dest,
                   p->op.comm.tag,
                   p->op.comm.comm,
                   p->op.comm.request);
  return PHASE_DONE;
}

static inline int phase_recv(Phase *p)
{
  TRANSPORT_t_recv(p->op.comm.buf,
                   p->op.comm.count,
                   p->op.comm.datatype,
                   p->op.comm.dest,
                   p->op.comm.tag,
                   p->op.comm.comm,
                   p->op.comm.request);
  return PHASE_DONE;
}

static inline int phase_recv_noop(Phase *p)
{
  return PHASE_DONE;
}

static inline int phase_fence(Phase *p)
{
  int rc, phase_state;
  *(CollTree_g.do_progress) = 0;
  rc = TRANSPORT_t_test(p->op.fence.count,p->op.fence.startReq);

  if(rc) phase_state = PHASE_DONE;
  else phase_state = PHASE_BUSY;

  *(CollTree_g.do_progress) = phase_state;
  return phase_state;
}

static inline int phase_reduce_local(Phase *p)
{
  TRANSPORT_t_reduce_local(p->op.reduce_local.inbuf,
                           p->op.reduce_local.inoutbuf,
                           p->op.reduce_local.count,
                           p->op.reduce_local.datatype,
                           p->op.reduce_local.op);
  return PHASE_DONE;
}

static inline int phase_dtcopy(Phase *p)
{
  TRANSPORT_t_dtcopy(p->op.dtcopy.tobuf,
                     p->op.dtcopy.tocount,
                     p->op.dtcopy.totype,
                     p->op.dtcopy.frombuf,
                     p->op.dtcopy.fromcount,
                     p->op.dtcopy.fromtype);
  return PHASE_DONE;
}

static inline int phase_freebuf(Phase *p)
{
  TRANSPORT_t_free(p->op.freebuf.buffer);
  return PHASE_DONE;
}
static inline int phase_freeop(Phase *p)
{
  TRANSPORT_t_addref_op(p->op.freeop.op,0);
  return PHASE_DONE;
}
static inline int phase_freedt(Phase *p)
{
  TRANSPORT_t_addref_dt(p->op.freedt.dt,0);
  return PHASE_DONE;
}

static inline
int kary_tree(int         rank,
              int         k,
              int         numranks,
              int        *parent,
              ChildRange  children[])
{
  int child;
  int numRanges=0;
  assert(k >=1);

  *parent = (rank<=0)?-1:(rank-1)/k;

  for(child=1; child<=k; child++) {
    int val = rank*k+child;

    if(val >= numranks)
      break;

    if(child==1) {
      children[0].startRank=val;
      children[0].endRank=val;
    } else
      children[0].endRank=val;

    numRanges=1;
  }

  return numRanges;
}

static inline
int ilog(int k, int number)
{
  int i=1, p=k-1;

  for(; p-1<number; i++)
    p *= k;

  return i;
}

static inline
int ipow(int base, int exp)
{
  int result = 1;

  while(exp) {
    if(exp & 1)
      result *= base;

    exp >>= 1;
    base *= base;
  }

  return result;
}

static inline
int getdigit(int k,
             int number,
             int digit)
{
  return (number/ipow(k, digit))%k;
}

static inline
int setdigit(int k,int number,
             int digit,int newdigit)
{
  int res     = number;
  int lshift  = ipow(k, digit);
  res        -= getdigit(k, number, digit)*lshift;
  res        += newdigit*lshift;
  return res;
}

static inline
int knomial_tree(int         rank,
                 int         k,
                 int         numranks,
                 int        *parent,
                 ChildRange  children[])
{
  int num_children = 0;
  int basek, i, j;
  assert(k>=2);

  /* Parent Calculation */
  if(rank<=0) *parent=-1;
  else {
    basek = ilog(k, numranks-1);

    for(i=0; i<basek; i++) {
      if(getdigit(k,rank,i)) {
        *parent = setdigit(k, rank, i, 0);
        break;
      }
    }
  }

  if(rank>=numranks)
    return -1;

  /* Children Calculation */
  num_children = 0;
  basek        = ilog(k, numranks-1);

  for(j=0; j<basek; j++) {
    if(getdigit(k, rank, j))
      break;

    for(i=1; i<k; i++) {
      int child = setdigit(k, rank, j, i);

      if(child < numranks) {
        assert(num_children < MAX_TREE_BREADTH);
        children[num_children].startRank = child;
        children[num_children].endRank   = child;
        num_children++;
      }
    }
  }

  return num_children;
}

static inline
void initKary(int       rank,
              int       nranks,
              int       k,
              CollTree *ct)
{
  ct->rank      = rank;
  ct->nranks    = nranks;
  ct->numRanges = kary_tree(ct->rank,
                            k,
                            ct->nranks,
                            &ct->parent,
                            ct->children);
}

static inline
void initKnomial(int       rank,
                 int       nranks,
                 int       k,
                 CollTree *ct)
{
  ct->rank        = rank;
  ct->nranks      = nranks;
  ct->numRanges   = knomial_tree(ct->rank,
                                 k,
                                 ct->nranks,
                                 &ct->parent,
                                 ct->children);
}

static inline
void initTree(int       rank,
              int       nranks,
              int       k,
              CollTree *ct)
{
#ifdef USE_KNOMIAL
  initKnomial(rank,nranks,k,ct);
#else
  initKary(rank,nranks,k,ct);
#endif
}

static inline
int dumpTree(int         tree_size)
{
  int   i;
  const char *name       = "collective tree";
  int   tree_radix = TREE_RADIX;

  fprintf(stderr, "digraph \"%s%d\" {\n",
          name, tree_radix);

  for(i=0; i<tree_size; i++) {
    CollTree ct;
    initTree(i,tree_size,tree_radix,&ct);
    fprintf(stderr, "%d -> %d\n",ct.rank, ct.parent);
    int j;

    for(j=0; j< ct.numRanges;  j++) {
      int k;

      for(k=ct.children[j].startRank;
          k<=ct.children[j].endRank;
          k++) {
        fprintf(stderr, "%d -> %d\n",ct.rank,k);
      }
    }
  }

  fprintf(stderr, "}\n");

  return 0;
}

static inline int
sched_barrier_exchange(int         tag,
                       COLL_COMM   comm,
                       PhaseArray *s)
{
  int           i,n;
  int           nphases = 0;
  Communicator *mycomm  = COMMSTRUCT(comm);
  COLL_DT       dt      = CollTree_g.control_dt;

  for(n=mycomm->tree.nranks-1; n>0; n>>=1)nphases++;

  for(i=0; i<nphases; i++) {
    int shift   = (1<<i);
    int to      = (mycomm->tree.rank+shift)%mycomm->tree.nranks;
    int from    = (mycomm->tree.rank)-shift;

    if(from<0) from = mycomm->tree.nranks+from;

    PHASE_SCHED_RECV(s,NULL,0,dt,from,tag,comm,phase_recv_noop);
    PHASE_SCHED_SEND(s,NULL,0,dt,to,tag,comm,phase_send);
    PHASE_FENCE(s,phase_fence);
  }

  return 0;
}


static inline
int sched_barrier(int         tag,
                  COLL_COMM   comm,
                  PhaseArray *s)
{
  int i, j;
  Communicator *mycomm    = COMMSTRUCT(comm);
  CollTree     *tree      = &mycomm->tree;
  int           numRanges = tree->numRanges;
  COLL_DT       dt        = CollTree_g.control_dt;
  int           children=0;

  FOREACHCHILDDO(children++);

  if(tree->parent == -1) {
    FOREACHCHILDDO(PHASE_SCHED_RECV(s,NULL,0,dt,j,tag,comm,phase_recv_noop));
    PHASE_FENCE(s,phase_fence);
    FOREACHCHILDDO(PHASE_SCHED_SEND(s,NULL, 0,dt,j,tag,comm,phase_send));
    PHASE_FENCE(s,phase_fence);
  } else {
    /* Receive from all children */
    FOREACHCHILDDO(PHASE_SCHED_RECV(s,NULL,0,dt,j,tag,comm,phase_recv_noop));
    PHASE_FENCE(s,phase_fence);
    /* Send to Parent      */
    PHASE_SCHED_SEND(s,NULL,0,dt,tree->parent,tag,comm, phase_send);
    PHASE_FENCE(s,phase_fence);
    /* Receive from Parent */
    PHASE_SCHED_RECV(s,NULL,0,dt,tree->parent,tag,comm,phase_recv_noop);
    PHASE_FENCE(s,phase_fence);
    /* Send to all children */
    FOREACHCHILDDO(PHASE_SCHED_SEND(s,NULL,0,dt,j,tag,comm, phase_send));
    PHASE_FENCE(s,phase_fence);
  }

  return 0;
}


static inline
int sched_bcast(void       *buffer,
                int         count,
                COLL_DT     datatype,
                int         root,
                int         tag,
                COLL_COMM   comm,
                PhaseArray *s)
{
  CollTree      logicalTree;
  int i,j,numRanges,k     = TREE_RADIX;
  Communicator *mycomm    = COMMSTRUCT(comm);
  int           size      = mycomm->tree.nranks;
  int           rank      = mycomm->tree.rank;
  int           lrank     = (rank+(size-root))%size;
  CollTree     *tree      = &logicalTree;
  int          children   = 0;

  initTree(lrank,size,k,tree);
  numRanges = tree->numRanges;
  FOREACHCHILDDO(children++);

  if(tree->parent == -1) {
    FOREACHCHILDDO(PHASE_SCHED_SEND(s,buffer,count,datatype,(j+root)%size,tag,comm,phase_send));
    PHASE_FENCE(s,phase_fence);
  } else {
    /* Receive from Parent */
    PHASE_SCHED_RECV(s,buffer,count,datatype,(tree->parent+root)%size,tag,comm,phase_recv_noop);
    PHASE_FENCE(s,phase_fence);
    /* Receive from all children */
    FOREACHCHILDDO(PHASE_SCHED_SEND(s,buffer,count,datatype,(j+root)%size,tag,comm,phase_send));
    PHASE_FENCE(s,phase_fence);
  }

  return 0;
}

static inline
int sched_reduce(const void *sendbuf,
                 void       *recvbuf,
                 int         count,
                 COLL_DT     datatype,
                 COLL_OP     op,
                 int         root,
                 int         tag,
                 COLL_COMM   comm,
                 int         is_commutative,
                 PhaseArray *s)
{
  CollTree logicalTree;
  int i,j,numRanges,k      = TREE_RADIX;
  Communicator *mycomm     = COMMSTRUCT(comm);
  int           size       = mycomm->tree.nranks;
  int           rank       = mycomm->tree.rank;
  int           lrank      = (rank+(size-root))%size;
  CollTree     *tree       = &logicalTree;
  int           is_inplace = TRANSPORT_t_isinplace((void *)sendbuf);
  size_t        lb,extent;
  int           is_contig;
  void         *free_ptr0  = NULL,*free_ptr1=NULL;
  int           children   = 0;

  TRANSPORT_t_dtinfo(datatype,&is_contig,&extent,&lb);

  if(is_commutative)
    initTree(lrank,size,k,tree);
  else
    /* We have to use knomial trees to get rank order */
    initKnomial(lrank,size,k,tree);

  numRanges = tree->numRanges;
  FOREACHCHILDDO(children++);

  if(tree->parent == -1) {
    k=0;

    if(!is_inplace)TRANSPORT_t_dtcopy(recvbuf,count,datatype,
                                        sendbuf,count,datatype);

    if(is_commutative) {
      FOREACHCHILD()
      PHASE_SCHED_RECVREDUCE(s,recvbuf,count,datatype,op,(j+root)%size,tag,comm,phase_recv_noop);
      PHASE_FENCE(s,phase_fence);
    } else {
      free_ptr0     = TRANSPORT_t_malloc(extent*count*children);
      FOREACHCHILD() {
        void *tmp_buf = (void *) (((char *)free_ptr0)+(k++*extent*count)-lb);
        PHASE_SCHED_RECV(s,tmp_buf,count,datatype,(j+root)%size,tag,comm,phase_recv_noop);
        PHASE_FENCE(s,phase_fence);
        PHASE_REDUCE_LOCAL(s,recvbuf,tmp_buf,count,datatype,op,phase_reduce_local);
        PHASE_DTCOPY(s,recvbuf,count,datatype,tmp_buf,count,datatype,phase_dtcopy);
      }
    }
  } else {
    void *result_buf;
    free_ptr1  = result_buf = TRANSPORT_t_malloc(extent*count);
    result_buf = (void *)((char *)result_buf-lb);
    k          = 0;
    TRANSPORT_t_dtcopy(result_buf,count,datatype,sendbuf,count,datatype);
    {
      if(is_commutative) {
        FOREACHCHILD()
        PHASE_SCHED_RECVREDUCE(s,result_buf,count,datatype,op,(j+root)%size,tag,comm,phase_recv_noop);
        PHASE_FENCE(s,phase_fence);
      } else {
        free_ptr0  = TRANSPORT_t_malloc(extent*count*children);
        FOREACHCHILD() {
          void *tmp_buf = (void *)(((char *)free_ptr0)+(k++*extent*count)-lb);
          PHASE_SCHED_RECV(s,tmp_buf,count,datatype,(j+root)%size,tag,comm,phase_recv_noop);
          PHASE_FENCE(s,phase_fence);
          PHASE_REDUCE_LOCAL(s,result_buf,tmp_buf,count,datatype,op,phase_reduce_local);
          PHASE_DTCOPY(s,result_buf,count,datatype,tmp_buf,count,datatype,phase_dtcopy);
        }
      }
    }
    PHASE_SCHED_SEND(s,result_buf,count,datatype,(tree->parent+root)%size,tag,comm,phase_send);
    PHASE_FENCE(s,phase_fence);
  }

  if(free_ptr0) PHASE_FREEBUF(s,free_ptr0,phase_freebuf);

  if(free_ptr1) PHASE_FREEBUF(s,free_ptr1,phase_freebuf);

  return 0;
}

static inline
int sched_reduce_full(const void *sendbuf,
                      void       *recvbuf,
                      int         count,
                      COLL_DT     datatype,
                      COLL_OP     op,
                      int         root,
                      int         tag,
                      COLL_COMM   comm,
                      PhaseArray *s)
{
  int       is_commutative,rc;
  TRANSPORT_t_opinfo(op,&is_commutative);

  if(root == 0 || is_commutative) {
    rc = sched_reduce(sendbuf,recvbuf,count,datatype,
                      op,root,tag,comm,is_commutative,s);
    return rc;
  } else {
    Communicator *mycomm    = COMMSTRUCT(comm);
    int           rank      = mycomm->tree.rank;
    size_t        lb,extent;
    int           is_contig;
    int           is_inplace = TRANSPORT_t_isinplace((void *)sendbuf);

    TRANSPORT_t_dtinfo(datatype,&is_contig,&extent,&lb);
    void *tmp_buf = NULL;
    void *sb      = NULL;

    if(rank == 0)
      tmp_buf = TRANSPORT_t_malloc(extent*count);

    if(rank == root && is_inplace)
      sb = recvbuf;
    else
      sb = (void *)sendbuf;

    sched_reduce(sb,tmp_buf,count,datatype,
                 op,0,tag,comm,is_commutative,s);

    if(rank == 0) {
      PHASE_SCHED_SEND(s,tmp_buf,count,datatype,root,tag,comm,phase_send);
      PHASE_FENCE(s,phase_fence);
    } else if(rank == root) {
      PHASE_SCHED_RECV(s,recvbuf,count,datatype,0,tag,comm,phase_recv_noop);
      PHASE_FENCE(s,phase_fence);
    }

    if(tmp_buf)PHASE_FREEBUF(s,tmp_buf,phase_freebuf);
  }

  return 0;
}

static inline int
sched_allreduce_exchange(const void *sendbuf,
                         void       *recvbuf,
                         int         count,
                         COLL_DT     datatype,
                         COLL_OP     op,
                         int         tag,
                         COLL_COMM   comm,
                         PhaseArray *s)
{
  /* does not handle in place or communative */
  int           upperPow,lowerPow,nphases = 0;
  int           i,n,is_contig,notPow2,inLower,exchangePhases,exchangeRanks;
  Communicator *mycomm  = COMMSTRUCT(comm);
  size_t        extent,lb;

  TRANSPORT_t_dtinfo(datatype,&is_contig,&extent,&lb);

  for(n=mycomm->tree.nranks-1; n>0; n>>=1)nphases++;

  upperPow = (1<<nphases);
  lowerPow = (1<<(nphases-1));
  notPow2  = (upperPow!=mycomm->tree.nranks);

  PHASE_DTCOPY(s,recvbuf,count,datatype,sendbuf,count,datatype,phase_dtcopy);
  inLower        = mycomm->tree.rank<lowerPow;
  exchangePhases = nphases-1;
  exchangeRanks  = lowerPow;

  if(notPow2 && inLower) {
    int from = mycomm->tree.rank+lowerPow;

    if(from < mycomm->tree.nranks) {
      PHASE_SCHED_RECVREDUCE(s,recvbuf,count,datatype,op,from,tag,comm,phase_recv_noop);
      PHASE_FENCE(s,phase_fence);
    }
  } else if(notPow2) {
    int to = mycomm->tree.rank%lowerPow;
    PHASE_SCHED_SEND(s,recvbuf,count,datatype,to,tag,comm,phase_send);
    PHASE_FENCE(s,phase_fence);
  } else {
    inLower        = 1;
    exchangePhases = nphases;
    exchangeRanks  = mycomm->tree.nranks;
  }

  if(inLower) {
    for(i=0; i<exchangePhases; i++) {
      int shift      = (1<<i);
      int to         = (mycomm->tree.rank+shift)%exchangeRanks;
      int from       = (mycomm->tree.rank)-shift;

      if(from<0)from = exchangeRanks+from;

      PHASE_SCHED_SEND(s,recvbuf,count,datatype,to,tag,comm,phase_send);
      PHASE_FENCE(s,phase_fence);
      PHASE_SCHED_RECVREDUCE(s,recvbuf,count,datatype,op,from,tag,comm,phase_recv_noop);
      PHASE_FENCE(s,phase_fence);
    }
  }

  if(notPow2 && inLower) {
    int to = mycomm->tree.rank+lowerPow;

    if(to < mycomm->tree.nranks) {
      PHASE_SCHED_SEND(s,recvbuf,count,datatype,to,tag,comm,phase_send);
      PHASE_FENCE(s,phase_fence);
    }
  } else if(notPow2) {
    int from = mycomm->tree.rank%lowerPow;
    PHASE_SCHED_RECV(s,recvbuf,count,datatype,from,tag,comm,phase_recv_noop);
    PHASE_FENCE(s,phase_fence);
  }

  return 0;
}

static inline
int sched_allreduce(const void *sendbuf,
                    void       *recvbuf,
                    int         count,
                    COLL_DT     datatype,
                    COLL_OP     op,
                    int         tag,
                    COLL_COMM   comm,
                    PhaseArray *s)
{
  int    is_commutative, is_inplace,is_contig;
  size_t lb,extent;
  void *tmp_buf = NULL;
  void *sbuf    = (void *)sendbuf;
  void *rbuf    = recvbuf;

  is_inplace = TRANSPORT_t_isinplace((void *)sendbuf);
  TRANSPORT_t_dtinfo(datatype,&is_contig,&extent,&lb);
  TRANSPORT_t_opinfo(op,&is_commutative);

  if(is_inplace) {
    tmp_buf = TRANSPORT_t_malloc(extent*count);
    sbuf    = recvbuf;
    rbuf    = tmp_buf;
  }

  if(is_commutative)
    sched_allreduce_exchange(sbuf,rbuf,count,datatype,op,tag,comm,s);
  else {
    sched_reduce(sbuf,rbuf,count,datatype,
                 op,0,tag,comm,is_commutative,s);
    sched_bcast(rbuf,count,datatype,0,tag,comm,s);
  }

  if(is_inplace) {
    PHASE_DTCOPY(s,recvbuf,count,datatype,tmp_buf,
                 count,datatype,phase_dtcopy);
    PHASE_FREEBUF(s,tmp_buf,phase_freebuf);
  }

  return 0;
}

__COLL_INLINE__ int COLLI_Init(TransportFunc *fcn,
                               int            comm_offset,
                               int            comm_sizeof,
                               int            req_offset,
                               int            req_sizeof,
                               int           *do_progress,
                               COLL_DT        control_dt)
{
  CollTree_g.headp          = &CollTree_g.head;
  CollTree_g.func           = *fcn;
  CollTree_g.comm_offset    =  comm_offset;
  CollTree_g.comm_sizeof    =  comm_sizeof;
  CollTree_g.req_offset     =  req_offset;
  CollTree_g.req_sizeof     =  req_sizeof;
  CollTree_g.do_progress    =  do_progress;
  CollTree_g.control_dt     =  control_dt;
  *(CollTree_g.do_progress) = 1;
  TAILQ_INIT(CollTree_g.headp);
  return 0;
}

__COLL_INLINE__ int COLLI_Comm_init(COLL_COMM comm)
{
  Communicator *mycomm   = COMMSTRUCT(comm);
  assert(CollTree_g.comm_sizeof>sizeof(*mycomm));
  assert(CollTree_g.req_sizeof>sizeof(PhaseArray *));
  int k = TREE_RADIX;
  initTree(TRANSPORT_t_rank(comm),
           TRANSPORT_t_size(comm),
           k,
           &mycomm->tree);
  char *e = getenv("COLL_DUMPTREE");

  if(e && atoi(e))
    dumpTree(mycomm->tree.nranks);

  return 0;
}

__COLL_INLINE__ int COLLI_Kick(void)
{
  PhaseArray *s = CollTree_g.head.tqh_first;
  int         counter = 0;

  for(s = CollTree_g.head.tqh_first; s != NULL; s = s->list_data.tqe_next) {
    int done = PHASE_KICK_NB(s,&s->collreq);

    if(done) {
      TAILQ_REMOVE(&CollTree_g.head, s, list_data);
      TRANSPORT_t_free(s);
      s = CollTree_g.head.tqh_first;

      if(s == NULL) {
        counter = 0;
        break;
      }
    }

    counter++;
  }

  if(counter==0) {
    *(CollTree_g.do_progress) = 0;
    return 1;
  }

  return 0;
}

__COLL_INLINE__ int COLLI_Allgather(const void *sendbuf,
                                    int sendcount,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    int recvcount,
                                    COLL_DT recvtype,
                                    COLL_COMM comm,
                                    int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Allgatherv(const void *sendbuf,
                                     int sendcount,
                                     COLL_DT sendtype,
                                     void *recvbuf,
                                     const int *recvcounts,
                                     const int *displs,
                                     COLL_DT recvtype,
                                     COLL_COMM comm,
                                     int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Allreduce(const void *sendbuf,
                                    void       *recvbuf,
                                    int         count,
                                    COLL_DT     datatype,
                                    COLL_OP     op,
                                    COLL_COMM   comm,
                                    int        *errflag)
{
  int        rc;
  PhaseArray s;
  int tag = COMMSTRUCT(comm)->curTag++;

  PHASE_INIT(&s);
  rc = sched_allreduce(sendbuf,recvbuf,count,
                       datatype,op,tag,comm,&s);
  PHASE_KICK(&s);
  return rc;
}

__COLL_INLINE__ int COLLI_Alltoall(const void *sendbuf,
                                   int sendcount,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   int recvcount,
                                   COLL_DT recvtype,
                                   COLL_COMM comm,
                                   int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Alltoallv(const void *sendbuf,
                                    const int *sendcnts,
                                    const int *sdispls,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    const int *recvcnts,
                                    const int *rdispls,
                                    COLL_DT recvtype,
                                    COLL_COMM comm,
                                    int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Alltoallw(const void *sendbuf,
                                    const int *sendcnts,
                                    const int *sdispls,
                                    const COLL_DT *sendtypes,
                                    void *recvbuf,
                                    const int *recvcnts,
                                    const int *rdispls,
                                    const COLL_DT *recvtypes,
                                    COLL_COMM comm,
                                    int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Bcast(void      *buffer,
                                int        count,
                                COLL_DT    datatype,
                                int        root,
                                COLL_COMM  comm,
                                int       *errflag)
{
  int        rc;
  PhaseArray s;
  int tag = COMMSTRUCT(comm)->curTag++;
  PHASE_INIT(&s);
  rc = sched_bcast(buffer,count,datatype,root,tag,comm,&s);
  PHASE_KICK(&s);
  return rc;
}

__COLL_INLINE__ int COLLI_Exscan(const void *sendbuf,
                                 void *recvbuf,
                                 int count,
                                 COLL_DT datatype,
                                 COLL_OP op,
                                 COLL_COMM comm,
                                 int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLICollTree_gather(const void *sendbuf,
                                 int sendcnt,
                                 COLL_DT sendtype,
                                 void *recvbuf,
                                 int recvcnt,
                                 COLL_DT recvtype,
                                 int root,
                                 COLL_COMM comm,
                                 int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLICollTree_gatherv(const void *sendbuf,
                                  int sendcnt,
                                  COLL_DT sendtype,
                                  void *recvbuf,
                                  const int *recvcnts,
                                  const int *displs,
                                  COLL_DT recvtype,
                                  int root,
                                  COLL_COMM comm,
                                  int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Reduce_scatter(const void *sendbuf,
                                         void *recvbuf,
                                         const int *recvcnts,
                                         COLL_DT datatype,
                                         COLL_OP op,
                                         COLL_COMM comm,
                                         int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Reduce_scatter_block(const void *sendbuf,
                                               void *recvbuf,
                                               int recvcount,
                                               COLL_DT datatype,
                                               COLL_OP op,
                                               COLL_COMM comm,
                                               int *errflag)
{
  assert(0);
  return 0;
}


__COLL_INLINE__ int COLLI_Reduce(const void *sendbuf,
                                 void       *recvbuf,
                                 int         count,
                                 COLL_DT     datatype,
                                 COLL_OP     op,
                                 int         root,
                                 COLL_COMM   comm,
                                 int        *errflag)
{
  int        rc;
  PhaseArray s;
  int tag = COMMSTRUCT(comm)->curTag++;
  PHASE_INIT(&s);
  rc = sched_reduce_full(sendbuf,recvbuf,count,datatype,
                         op,root,tag,comm,&s);
  PHASE_KICK(&s);
  return rc;
}

__COLL_INLINE__ int COLLI_Scan(const void *sendbuf,
                               void *recvbuf,
                               int count,
                               COLL_DT datatype,
                               COLL_OP op,
                               COLL_COMM comm,
                               int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Scatter(const void *sendbuf,
                                  int sendcnt,
                                  COLL_DT sendtype,
                                  void *recvbuf,
                                  int recvcnt,
                                  COLL_DT recvtype,
                                  int root,
                                  COLL_COMM comm,
                                  int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Scatterv(const void *sendbuf,
                                   const int *sendcnts,
                                   const int *displs,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   int recvcnt,
                                   COLL_DT recvtype,
                                   int root,
                                   COLL_COMM comm,
                                   int *errflag)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Barrier(COLL_COMM comm,
                                  int       *errflag)
{
  int        rc;
  PhaseArray s;
  int tag = COMMSTRUCT(comm)->curTag++;
  PHASE_INIT(&s);
  rc = sched_barrier_exchange(tag, comm,&s);
  PHASE_KICK(&s);
  return rc;
}

__COLL_INLINE__ int COLLI_Iallgather(const void *sendbuf,
                                     int sendcount,
                                     COLL_DT sendtype,
                                     void *recvbuf,
                                     int recvcount,
                                     COLL_DT recvtype,
                                     COLL_COMM comm,
                                     COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iallgatherv(const void *sendbuf,
                                      int sendcount,
                                      COLL_DT sendtype,
                                      void *recvbuf,
                                      const int *recvcounts,
                                      const int *displs,
                                      COLL_DT recvtype,
                                      COLL_COMM comm,
                                      COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iallreduce(const void *sendbuf,
                                     void       *recvbuf,
                                     int         count,
                                     COLL_DT     datatype,
                                     COLL_OP     op,
                                     COLL_COMM   comm,
                                     COLL_REQ   *request)
{
  PhaseArray  *s,**rs;
  int done = 0;
  int tag = COMMSTRUCT(comm)->curTag++;
  PHASE_INIT_NB(s,rs,request);
  sched_allreduce(sendbuf,recvbuf,count,datatype,
                  op,tag,comm,s);
  done = PHASE_KICK_NB(s,&s->collreq);

  if(!done) {
    TRANSPORT_t_addref_op(op,1);
    TRANSPORT_t_addref_dt(datatype,1);
    PHASE_FREEOP(s,op,phase_freeop);
    PHASE_FREEDT(s,datatype,phase_freedt);
    TAILQ_INSERT_TAIL(CollTree_g.headp,s,list_data);
    *(CollTree_g.do_progress) = 1;
  } else
    TRANSPORT_t_free(s);

  return 0;
}

__COLL_INLINE__ int COLLI_Ialltoall(const void *sendbuf,
                                    int sendcount,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    int recvcount,
                                    COLL_DT recvtype,
                                    COLL_COMM comm,
                                    COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ialltoallv(const void *sendbuf,
                                     const int *sendcnts,
                                     const int *sdispls,
                                     COLL_DT sendtype,
                                     void *recvbuf,
                                     const int *recvcnts,
                                     const int *rdispls,
                                     COLL_DT recvtype,
                                     COLL_COMM comm,
                                     COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ialltoallw(const void *sendbuf,
                                     const int *sendcnts,
                                     const int *sdispls,
                                     const COLL_DT *sendtypes,
                                     void *recvbuf,
                                     const int *recvcnts,
                                     const int *rdispls,
                                     const COLL_DT *recvtypes,
                                     COLL_COMM comm,
                                     COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ibcast(void      *buffer,
                                 int        count,
                                 COLL_DT    datatype,
                                 int        root,
                                 COLL_COMM  comm,
                                 COLL_REQ  *request)
{
  PhaseArray  *s,**rs;
  int done=0;
  int tag = COMMSTRUCT(comm)->curTag++;
  PHASE_INIT_NB(s,rs,request);
  sched_bcast(buffer,count,datatype,
              root,tag,comm,s);
  done = PHASE_KICK_NB(s,&s->collreq);

  if(!done) {
    TRANSPORT_t_addref_dt(datatype,1);
    PHASE_FREEDT(s,datatype,phase_freedt);
    TAILQ_INSERT_TAIL(CollTree_g.headp,s,list_data);
    *(CollTree_g.do_progress) = 1;
  } else
    TRANSPORT_t_free(s);

  return 0;
}

__COLL_INLINE__ int COLLI_Iexscan(const void *sendbuf,
                                  void *recvbuf,
                                  int count,
                                  COLL_DT datatype,
                                  COLL_OP op,
                                  COLL_COMM comm,
                                  COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Igather(const void *sendbuf,
                                  int sendcnt,
                                  COLL_DT sendtype,
                                  void *recvbuf,
                                  int recvcnt,
                                  COLL_DT recvtype,
                                  int root,
                                  COLL_COMM comm,
                                  COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Igatherv(const void *sendbuf,
                                   int sendcnt,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   const int *recvcnts,
                                   const int *displs,
                                   COLL_DT recvtype,
                                   int root,
                                   COLL_COMM comm,
                                   COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ireduce_scatter(const void *sendbuf,
                                          void *recvbuf,
                                          const int *recvcnts,
                                          COLL_DT datatype,
                                          COLL_OP op,
                                          COLL_COMM comm,
                                          COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ireduce_scatter_block(const void *sendbuf,
                                                void *recvbuf,
                                                int recvcount,
                                                COLL_DT datatype,
                                                COLL_OP op,
                                                COLL_COMM comm,
                                                COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ireduce(const void *sendbuf,
                                  void       *recvbuf,
                                  int         count,
                                  COLL_DT     datatype,
                                  COLL_OP     op,
                                  int         root,
                                  COLL_COMM   comm,
                                  COLL_REQ   *request)
{
  PhaseArray  *s,**rs;
  int done = 0;
  int tag = COMMSTRUCT(comm)->curTag++;
  PHASE_INIT_NB(s,rs,request);
  sched_reduce_full(sendbuf,recvbuf,count,datatype,
                    op,root,tag,comm,s);
  done = PHASE_KICK_NB(s,&s->collreq);

  if(!done) {
    TRANSPORT_t_addref_op(op,1);
    TRANSPORT_t_addref_dt(datatype,1);
    PHASE_FREEOP(s,op,phase_freeop);
    PHASE_FREEDT(s,datatype,phase_freedt);
    TAILQ_INSERT_TAIL(CollTree_g.headp,s,list_data);
    *(CollTree_g.do_progress) = 1;
  } else
    TRANSPORT_t_free(s);

  return 0;
}

__COLL_INLINE__ int COLLI_Iscan(const void *sendbuf,
                                void *recvbuf,
                                int count,
                                COLL_DT datatype,
                                COLL_OP op,
                                COLL_COMM comm,
                                COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iscatter(const void *sendbuf,
                                   int sendcnt,
                                   COLL_DT sendtype,
                                   void *recvbuf,
                                   int recvcnt,
                                   COLL_DT recvtype,
                                   int root,
                                   COLL_COMM comm,
                                   COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Iscatterv(const void *sendbuf,
                                    const int *sendcnts,
                                    const int *displs,
                                    COLL_DT sendtype,
                                    void *recvbuf,
                                    int recvcnt,
                                    COLL_DT recvtype,
                                    int root,
                                    COLL_COMM comm,
                                    COLL_REQ     *request)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ibarrier(COLL_COMM comm,
                                   COLL_REQ  *request)
{
  PhaseArray  *s,**rs;
  int done = 0;
  int tag  = COMMSTRUCT(comm)->curTag++;
  PHASE_INIT_NB(s,rs,request);
  sched_barrier_exchange(tag,comm,s);
  done = PHASE_KICK_NB(s,&s->collreq);

  if(!done) {
    TAILQ_INSERT_TAIL(CollTree_g.headp,s,list_data);
    *(CollTree_g.do_progress) = 1;
  } else
    TRANSPORT_t_free(s);

  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_allgather(const void *sendbuf,
                                             int sendcount,
                                             COLL_DT sendtype,
                                             void *recvbuf,
                                             int recvcount,
                                             COLL_DT recvtype,
                                             COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_allgatherv(const void *sendbuf,
                                              int sendcount,
                                              COLL_DT sendtype,
                                              void *recvbuf,
                                              const int recvcounts[],
                                              const int displs[],
                                              COLL_DT recvtype,
                                              COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_alltoall(const void *sendbuf,
                                            int sendcount,
                                            COLL_DT sendtype,
                                            void *recvbuf,
                                            int recvcount,
                                            COLL_DT recvtype,
                                            COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_alltoallv(const void *sendbuf,
                                             const int sendcounts[],
                                             const int sdispls[],
                                             COLL_DT sendtype,
                                             void *recvbuf,
                                             const int recvcounts[],
                                             const int rdispls[],
                                             COLL_DT recvtype,
                                             COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Neighbor_alltoallw(const void *sendbuf,
                                             const int sendcounts[],
                                             const COLL_Aint sdispls[],
                                             const COLL_DT sendtypes[],
                                             void *recvbuf,
                                             const int recvcounts[],
                                             const COLL_Aint rdispls[],
                                             const COLL_DT recvtypes[],
                                             COLL_COMM comm)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_allgather(const void *sendbuf,
                                              int sendcount,
                                              COLL_DT sendtype,
                                              void *recvbuf,
                                              int recvcount,
                                              COLL_DT recvtype,
                                              COLL_COMM comm,
                                              COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_allgatherv(const void *sendbuf,
                                               int sendcount,
                                               COLL_DT sendtype,
                                               void *recvbuf,
                                               const int recvcounts[],
                                               const int displs[],
                                               COLL_DT recvtype,
                                               COLL_COMM comm,
                                               COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_alltoall(const void *sendbuf,
                                             int sendcount,
                                             COLL_DT sendtype,
                                             void *recvbuf,
                                             int recvcount,
                                             COLL_DT recvtype,
                                             COLL_COMM comm,
                                             COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_alltoallv(const void *sendbuf,
                                              const int sendcounts[],
                                              const int sdispls[],
                                              COLL_DT sendtype,
                                              void *recvbuf,
                                              const int recvcounts[],
                                              const int rdispls[],
                                              COLL_DT recvtype,
                                              COLL_COMM comm,
                                              COLL_SCHED s)
{
  assert(0);
  return 0;
}

__COLL_INLINE__ int COLLI_Ineighbor_alltoallw(const void *sendbuf,
                                              const int sendcounts[],
                                              const COLL_Aint sdispls[],
                                              const COLL_DT sendtypes[],
                                              void *recvbuf,
                                              const int recvcounts[],
                                              const COLL_Aint rdispls[],
                                              const COLL_DT recvtypes[],
                                              COLL_COMM comm,
                                              COLL_SCHED s)
{
  assert(0);
  return 0;
}
