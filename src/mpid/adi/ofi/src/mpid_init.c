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
#include <pmi.h>
#include "mpid_types.h"
#include "mpir_cvars.h"
/* Function Prototypes */

__SI__ int MPIDI_Choose_provider(info_t *prov, info_t **prov_use);
__SI__ int MPIDI_Create_endpoint(info_t        *prov_use,
                                 fid_domain_t   domain,
                                 fid_cq_t       p2p_cq,
                                 fid_cntr_t     rma_ctr,
                                 fid_mr_t       mr,
                                 fid_av_t       av,
                                 fid_base_ep_t *ep,
                                 int            index);

#define CHOOSE_PROVIDER(prov, prov_use,errstr)                          \
  ({                                                                    \
    info_t *p = prov;                                                   \
    MPIR_ERR_CHKANDJUMP4(p==NULL, mpi_errno,MPI_ERR_OTHER,"**ofid_addrinfo", \
                         "**ofid_addrinfo %s %d %s %s",__SHORT_FILE__,  \
                         __LINE__,FCNAME, errstr);                      \
    MPIDI_Choose_provider(prov,prov_use);                               \
  })

#if (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_PER_OBJECT)
#define MAX_THREAD_MODE MPI_THREAD_MULTIPLE
#elif  (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_GLOBAL)
#define MAX_THREAD_MODE MPI_THREAD_MULTIPLE
#elif  (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_SINGLE)
#define MAX_THREAD_MODE MPI_THREAD_SERIALIZED
#elif  (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_LOCK_FREE)
#define MAX_THREAD_MODE MPI_THREAD_SERIALIZED
#else
#error "Thread Granularity:  Invalid"
#endif

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Init)
__ADI_INLINE__ int MPIDI_Init(int    *argc,
                              char ***argv,
                              int     requested,
                              int    *provided,
                              int    *has_args,
                              int    *has_env)
{
  int            mpi_errno = MPI_SUCCESS,pmi_errno,spawned,size,rank,i,fi_version;
  MPIR_Errflag_t errflag   = MPIR_ERR_NONE;
  char          *table     = NULL;
  MPIR_Comm     *comm;

  BEGIN_FUNC(FCNAME);

  ADI_COMPILE_TIME_ASSERT(offsetof(struct MPIR_Request, dev.pad)==offsetof(MPIDI_Chunk_request,context));
  ADI_COMPILE_TIME_ASSERT(offsetof(struct MPIR_Request, dev.pad)==offsetof(MPIDI_Huge_chunk_t,context));
  ADI_COMPILE_TIME_ASSERT(offsetof(struct MPIR_Request, dev.pad)==offsetof(MPIDI_Ctrl_req,context));
  ADI_COMPILE_TIME_ASSERT(offsetof(struct MPIR_Request, dev.pad)==offsetof(MPIDI_Ssendack_request,context));
  ADI_COMPILE_TIME_ASSERT(offsetof(struct MPIR_Request, dev.pad)==offsetof(MPIDI_Dynproc_req,context));
  ADI_COMPILE_TIME_ASSERT(offsetof(struct MPIR_Request, dev.pad)==offsetof(MPIDI_Win_request,context));
  ADI_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devreq_t)>=sizeof(MPIDI_OFIReq_t));
  ADI_COMPILE_TIME_ASSERT(sizeof(MPIR_Request)>=sizeof(MPIDI_Win_request));
  PMI_RC(PMI_Init(&spawned),pmi);
  PMI_RC(PMI_Get_appnum(&MPIDI_Global.jobid),pmi);

  /* ------------------------------- */
  /* Set process attributes          */
  /* ------------------------------- */
  MPIR_Process.attrs.appnum          = MPIDI_Global.jobid;
  MPIR_Process.attrs.tag_ub          = (1<<MPID_TAG_SHIFT)-1;
  MPIR_Process.attrs.wtime_is_global = 1;
  MPIR_Process.attrs.io              = MPI_ANY_SOURCE;

  /* -------------------------------------------- */
  /* Load/Initialize collective transport library */
  /* -------------------------------------------- */
  transport_load(&TRANSPORT_OBJ);
  TRANSPORT_t_init();

  /* -------------------------------------------- */
  /* Load/Initialize collective algorithm library */
  /* -------------------------------------------- */
  coll_load(&COLL_OBJ);
  COLL_Init(&TRANSPORT_OBJ,
            COMMBLOBOFFSET(),
            COMMBLOBSIZE(),
            MPIDI_REQUEST_HDR_SIZE,
            sizeof(MPIDI_OFIReq_t),
            &MPIDI_Global.coll_progress,
            (COLL_DT)(uint64_t)MPI_CHAR);

  /* ---------------------------------------------- */
  /* Initialize job rank/size                       */
  /* ---------------------------------------------- */
  PMI_RC(PMI_Get_rank(&rank),pmi);
  PMI_RC(PMI_Get_size(&size),pmi);

  /* ---------------------------------- */
  /* Initialize MPI_COMM_SELF and VCRT  */
  /* ---------------------------------- */
  comm                               = MPIR_Process.comm_self;
  comm->rank                         = 0;
  comm->remote_size                  = 1;
  comm->local_size                   = 1;
  MPI_RC_POP(MPIDI_VCRT_Create(comm->remote_size, &COMM_OFI(comm)->vcrt));
  COMM_OFI(comm)->vcrt->vcr_table[0].addr_idx  = rank;

  /* ---------------------------------- */
  /* Initialize MPI_COMM_WORLD and VCRT */
  /* ---------------------------------- */
  comm              = MPIR_Process.comm_world;
  comm->rank        = rank;
  comm->remote_size = size;
  comm->local_size  = size;
  MPI_RC_POP(MPIDI_VCRT_Create(comm->remote_size, &COMM_OFI(comm)->vcrt));

  /* ------------------------------------------------------------------------ */
  /* Hints to filter providers                                                */
  /* See man fi_getinfo for a list                                            */
  /* of all filters                                                           */
  /* mode:  Select capabilities ADI is prepared to support.                   */
  /*        In this case, ADI will pass in context into                       */
  /*        communication calls.                                              */
  /*        Note that we do not fill in FI_LOCAL_MR, which means this ADI     */
  /*        does not support exchange of memory regions on communication      */
  /*        calls. OFI requires that all communication calls use a registered */
  /*        mr but in our case this ADI is written to only support            */
  /*        transfers on a dynamic memory region that spans all of memory.    */
  /*        So, we do not set the FI_LOCAL_MR mode bit, and we set the        */
  /*        FI_SCALABLE_MR on the domain to  tell OFI our requirement and     */
  /*        filter providers appropriately                                    */
  /* caps:     Capabilities required from the provider.  The bits specified   */
  /*           with buffered receive, cancel, and remote complete implements  */
  /*           MPI semantics.                                                 */
  /*           Tagged: used to support tag matching, 2-sided                  */
  /*           RMA|Atomics:  supports MPI 1-sided                             */
  /*           MSG|MULTI_RECV:  Supports synchronization protocol for 1-sided */
  /*           We expect to register all memory up front for use with this    */
  /*           endpoint, so the ADI requires dynamic memory regions           */
/* -------------------------------------------------------------------------- */
  info_t *hints, *prov,*prov_use;

  /* ------------------------------------------------------------------------ */
  /* fi_allocinfo: allocate and zero an fi_info structure and all related
   * substructures																														*/
  /* ------------------------------------------------------------------------ */
  hints = fi_allocinfo();
  MPIR_Assert(hints != NULL);

  hints->mode      = FI_CONTEXT;         /* We can handle contexts  */
  hints->caps      = FI_TAGGED;          /* Tag matching interface  */
  hints->caps     |= FI_MSG;             /* Message Queue apis      */
  hints->caps     |= FI_MULTI_RECV;      /* Shared receive buffer   */
  hints->caps     |= FI_RMA;             /* RMA(read/write)         */
  hints->caps     |= FI_ATOMICS;         /* Atomics capabilities    */
  /* ------------------------------------------------------------------------ */
  /* FI_VERSION provides binary backward and forward compatibility support    */
  /* Specify the version of OFI is coded to, the provider will select struct  */
  /* layouts that are compatible with this version.                           */
  /* ------------------------------------------------------------------------ */
  fi_version    = FI_VERSION(MPIDI_FI_MAJOR_VERSION,MPIDI_FI_MINOR_VERSION);

  /* ------------------------------------------------------------------------ */
  /* Set object options to be filtered by getinfo                             */
  /* domain_attr:  domain attribute requirements                              */
  /* op_flags:     persistent flag settings for an endpoint                   */
  /* endpoint type:  see FI_EP_RDM                                            */
  /* Filters applied (for this ADI, we need providers that can support):      */
  /* THREAD_ENDPOINT:  Progress serialization is handled by ADI (locking)     */
  /* PROGRESS_AUTO:  request providers that make progress without requiring   */
  /*                 the ADI to dedicate a thread to advance the state        */
  /* FI_DELIVERY_COMPLETE:  RMA operations are visible in remote memory       */
  /* FI_COMPLETION:  Selective completions of RMA ops                         */
  /* FI_EP_RDM:  Reliable datagram                                            */
  /* ------------------------------------------------------------------------ */
  hints->addr_format	                 = FI_FORMAT_UNSPEC;
  hints->domain_attr->threading        = FI_THREAD_ENDPOINT;
  hints->domain_attr->control_progress = FI_PROGRESS_AUTO;
  hints->domain_attr->data_progress		 = FI_PROGRESS_AUTO;
  hints->domain_attr->resource_mgmt		 = FI_RM_ENABLED;
  hints->domain_attr->av_type					 = FI_AV_UNSPEC;
  hints->domain_attr->mr_mode					 = FI_MR_SCALABLE;
  hints->tx_attr->op_flags             = FI_DELIVERY_COMPLETE|FI_COMPLETION;
  hints->rx_attr->op_flags             = FI_COMPLETION;
  hints->ep_attr->type                 = FI_EP_RDM;

  /* ------------------------------------------------------------------------ */
  /* fi_getinfo:  returns information about fabric  services for reaching a   */
  /* remote node or service.  this does not necessarily allocate resources.   */
  /* Pass NULL for name/service because we want a list of providers supported */
  /* ------------------------------------------------------------------------ */
  char * provname;
  provname = MPIR_CVAR_OFI_USE_PROVIDER?(char*)MPL_strdup(MPIR_CVAR_OFI_USE_PROVIDER):NULL;
  hints->fabric_attr->prov_name = provname;
  FI_RC(fi_getinfo(fi_version,NULL,NULL,0ULL,hints,&prov),addrinfo);
  CHOOSE_PROVIDER(prov, &prov_use, "No suitable provider provider found");

  /* ------------------------------------------------------------------------ */
  /* Set global attributes attributes based on the provider choice            */
  /* ------------------------------------------------------------------------ */
  MPIDI_Global.max_buffered_send  = prov_use->tx_attr->inject_size;
  MPIDI_Global.max_buffered_write = prov_use->tx_attr->inject_size;
  MPIDI_Global.max_send           = prov_use->ep_attr->max_msg_size;
  MPIDI_Global.max_write          = prov_use->ep_attr->max_msg_size;

  /* ------------------------------------------------------------------------ */
  /* Open fabric                                                              */
  /* The getinfo struct returns a fabric attribute struct that can be used to */
  /* instantiate the virtual or physical network.  This opens a "fabric       */
  /* provider".   We choose the first available fabric, but getinfo           */
  /* returns a list.                                                          */
  /* ------------------------------------------------------------------------ */
  FI_RC(fi_fabric(prov_use->fabric_attr,&MPIDI_Global.fabric, NULL),fabric);

  /* ------------------------------------------------------------------------ */
  /* Create the access domain, which is the physical or virtual network or    */
  /* hardware port/collection of ports.  Returns a domain object that can be  */
  /* used to create endpoints.                                                */
  /* ------------------------------------------------------------------------ */
  FI_RC(fi_domain(MPIDI_Global.fabric, prov_use, &MPIDI_Global.domain, NULL),opendomain);

  /* ------------------------------------------------------------------------ */
  /* Create the objects that will be bound to the endpoint.                   */
  /* The objects include:                                                     */
  /*     * dynamic memory-spanning memory region                              */
  /*     * completion queues for events                                       */
  /*     * counters for rma operations                                        */
  /*     * address vector of other endpoint addresses                         */
  /* ------------------------------------------------------------------------ */

  /* ------------------------------------------------------------------------ */
  /* Construct:  Memory region                                                */
  /* ------------------------------------------------------------------------ */

  uint64_t mr_flags;
#ifdef MPIDI_USE_MR_OFFSET
  mr_flags = FI_MR_OFFSET;
#else
  mr_flags = 0ULL;
#endif
  FI_RC(fi_mr_reg(MPIDI_Global.domain,            /* In:  Domain Object       */
                  0,                              /* In:  Lower memory address*/
                  UINTPTR_MAX,                    /* In:  Upper memory address*/
                  FI_REMOTE_READ|FI_REMOTE_WRITE| /* In:  Expose MR for read/write */
                  FI_SEND|FI_RECV,
                  0ULL,                           /* In:  base MR offset      */
                  0ULL,                           /* In:  requested key       */
                  mr_flags,                       /* In:  flags               */
                  &MPIDI_Global.mr,               /* Out: memregion object    */
                  NULL),mr_reg);                  /* In:  context             */
  MPIDI_Global.lkey = fi_mr_key(MPIDI_Global.mr);
  MPIR_Assert(MPIDI_Global.lkey == 0);

  /* ------------------------------------------------------------------------ */
  /* Construct:  Completion Queues                                            */
  /* ------------------------------------------------------------------------ */
  cq_attr_t  cq_attr;
  memset(&cq_attr, 0, sizeof(cq_attr));
  cq_attr.format = FI_CQ_FORMAT_TAGGED;
  FI_RC(fi_cq_open(MPIDI_Global.domain,  /* In:  Domain Object                */
                   &cq_attr,             /* In:  Configuration object         */
                   &MPIDI_Global.p2p_cq, /* Out: CQ Object                    */
                   NULL),opencq);        /* In:  Context for cq events        */

  /* ------------------------------------------------------------------------ */
  /* Construct:  Counters                                                     */
  /* ------------------------------------------------------------------------ */
  cntr_attr_t cntr_attr;
  memset(&cntr_attr,0,sizeof(cntr_attr));
  cntr_attr.events   = FI_CNTR_EVENTS_COMP;
  FI_RC(fi_cntr_open(MPIDI_Global.domain,   /* In:  Domain Object        */
                     &cntr_attr,            /* In:  Configuration object */
                     &MPIDI_Global.rma_ctr, /* Out: Counter Object       */
                     NULL),openct);         /* Context: counter events   */

  /* ------------------------------------------------------------------------ */
  /* Construct:  Address Vector                                               */
  /* ------------------------------------------------------------------------ */
  fi_addr_t *mapped_table;
  av_attr_t  av_attr;
  memset(&av_attr, 0, sizeof(av_attr));
#ifdef MPIDI_USE_AV_TABLE
  av_attr.type           = FI_AV_TABLE;
  av_attr.rx_ctx_bits    = MPIDI_MAX_ENDPOINTS_BITS;
  MPIDI_Addr_table       = (MPIDI_Addr_table_t *)MPL_malloc(sizeof(MPIDI_Addr_table_t));
  MPIDI_Addr_table->size = size;
  mapped_table           = NULL;
#else
  av_attr.type           = FI_AV_MAP;
  av_attr.rx_ctx_bits    = MPIDI_MAX_ENDPOINTS_BITS;
  MPIDI_Addr_table       = (MPIDI_Addr_table_t *)MPL_malloc(size * sizeof(fi_addr_t) + sizeof(MPIDI_Addr_table_t));
  MPIDI_Addr_table->size = size;
  mapped_table           = (fi_addr_t *)MPIDI_Addr_table->table;
#endif
  FI_RC(fi_av_open(MPIDI_Global.domain, /* In:  Domain Object         */
                   &av_attr,            /* In:  Configuration object  */
                   &MPIDI_Global.av,    /* Out: AV Object             */
                   NULL),avopen);       /* Context: AV events         */

  /* ------------------------------------------------------------------------ */
  /* Create a transport level communication endpoint.  To use the endpoint,   */
  /* it must be bound to completion counters or event queues and enabled,     */
  /* and the resources consumed by it, such as address vectors, counters,     */
  /* completion queues, etc.                                                  */
  /* ------------------------------------------------------------------------ */
  MPI_RC_POP(MPIDI_Create_endpoint(prov_use,
                                   MPIDI_Global.domain,
                                   MPIDI_Global.p2p_cq,
                                   MPIDI_Global.rma_ctr,
                                   MPIDI_Global.mr,
                                   MPIDI_Global.av,
                                   &MPIDI_Global.ep,0));
  /* ---------------------------------- */
  /* Get our endpoint name and publish  */
  /* the socket to the KVS              */
  /* ---------------------------------- */
  char   valS[MPIDI_KVSAPPSTRLEN],*val;
  int    str_errno, maxlen;
  MPIDI_Global.addrnamelen = FI_NAME_MAX;
  FI_RC(fi_getname((fid_t)MPIDI_Global.ep,MPIDI_Global.addrname,
                   &MPIDI_Global.addrnamelen), getname);
  MPIR_Assert(MPIDI_Global.addrnamelen <= FI_NAME_MAX);

  val       = valS;
  str_errno = MPL_STR_SUCCESS;
  maxlen    = MPIDI_KVSAPPSTRLEN;
  memset(val, 0, maxlen);
  MPI_STR_RC(MPL_str_add_binary_arg(&val,&maxlen,"OFI",(char *)&MPIDI_Global.addrname,
                                    MPIDI_Global.addrnamelen),buscard_len);
  PMI_RC(PMI_KVS_Get_my_name(MPIDI_Global.kvsname, MPIDI_KVSAPPSTRLEN),pmi);

  char keyS[MPIDI_KVSAPPSTRLEN];
  val = valS;
  sprintf(keyS, "OFI-%d",rank);
  PMI_RC(PMI_KVS_Put(MPIDI_Global.kvsname, keyS, val), pmi);
  PMI_RC(PMI_KVS_Commit(MPIDI_Global.kvsname),pmi);
  PMI_RC(PMI_Barrier(),pmi);

  /* -------------------------------- */
  /* Create our address table from    */
  /* encoded KVS values               */
  /* -------------------------------- */
  table  = (char *) MPL_malloc(size*MPIDI_Global.addrnamelen);
  maxlen = MPIDI_KVSAPPSTRLEN;

  for(i=0; i<size; i++) {
    sprintf(keyS, "OFI-%d",i);
    PMI_RC(PMI_KVS_Get(MPIDI_Global.kvsname, keyS, valS, MPIDI_KVSAPPSTRLEN),pmi);
    MPI_STR_RC(MPL_str_get_binary_arg(valS,"OFI",(char *)&table[i*MPIDI_Global.addrnamelen],
                                      MPIDI_Global.addrnamelen,&maxlen),buscard_len);
  }

  /* -------------------------------- */
  /* Table is constructed.  Map it    */
  /* -------------------------------- */
  FI_RC(fi_av_insert(MPIDI_Global.av,table,size,mapped_table,0ULL,NULL),avmap);

  /* -------------------------------- */
  /* Create the id to object maps     */
  /* -------------------------------- */
  MPIDI_Map_create(&MPIDI_Global.win_map);
  MPIDI_Map_create(&MPIDI_Global.comm_map);

  /* ------------------------------------------- */
  /* Post Buffers for Protocol Control Messages  */
  /* ------------------------------------------- */
  int       iov_len;
  MPIDI_Global.num_ctrlblock = 2;
  iov_len                    = 128*1024*MPID_MIN_CTRL_MSG_SZ;
  MPIDI_Global.iov           = (iovec_t *) MPL_malloc(sizeof(iovec_t)*MPIDI_Global.num_ctrlblock);
  MPIDI_Global.msg           = (msg_t *) MPL_malloc(sizeof(msg_t)*MPIDI_Global.num_ctrlblock);
  MPIDI_Global.control_req   = (MPIDI_Ctrl_req *) MPL_malloc(sizeof(MPIDI_Ctrl_req)*MPIDI_Global.num_ctrlblock);
  MPIR_Assert(MPIDI_Global.iov!=NULL);
  MPIR_Assert(MPIDI_Global.msg!=NULL);
  size_t optlen;
  optlen = MPID_MIN_CTRL_MSG_SZ;
  FI_RC(fi_setopt((fid_t)G_RXC_MSG(0),
                  FI_OPT_ENDPOINT,
                  FI_OPT_MIN_MULTI_RECV,
                  &optlen,
                  sizeof(optlen)),setopt);

  for(i = 0; i < MPIDI_Global.num_ctrlblock; i++) {
    MPIDI_Global.iov[i].iov_base  = MPL_malloc(iov_len);
    MPIR_Assert(MPIDI_Global.iov[i].iov_base!=NULL);
    MPIDI_Global.iov[i].iov_len   = iov_len;
    MPIDI_Global.msg[i].msg_iov   = &MPIDI_Global.iov[i];
    MPIDI_Global.msg[i].desc      = (void **)&MPIDI_Global.mr;
    MPIDI_Global.msg[i].iov_count = 1;
    MPIDI_Global.msg[i].addr      = FI_ADDR_UNSPEC;
    MPIDI_Global.msg[i].context   = &MPIDI_Global.control_req[i].context;
    MPIDI_Global.control_req[i].callback = MPIDI_Control_dispatch;
    MPIDI_Global.msg[i].data      = 0;
    FI_RC_RETRY(fi_recvmsg(G_RXC_MSG(0),&MPIDI_Global.msg[i],
                           FI_MULTI_RECV|FI_COMPLETION),prepost);
  }

  /* -------------------------------- */
  /* Calculate per-node map           */
  /* -------------------------------- */
  uint32_t *nodemap;
  nodemap = (uint32_t *)MPL_malloc(MPIR_Process.comm_world->local_size*sizeof(*nodemap));
  nodemap[MPIR_Process.comm_world->rank] = gethostid();
  MPI_RC_POP(MPIR_Allgather_impl(MPI_IN_PLACE,0,MPI_DATATYPE_NULL,nodemap,
                                 sizeof(*nodemap),MPI_BYTE,MPIR_Process.comm_world,
                                 &errflag));
  MPIDI_Global.node_map = (MPID_Node_id_t *)MPL_malloc(MPIR_Process.comm_world->local_size
                                                       *sizeof(*MPIDI_Global.node_map));
  MPIDI_build_nodemap(nodemap,MPIDI_Global.node_map,
                      MPIR_Process.comm_world->local_size,
                      &MPIDI_Global.max_node_id);
  MPL_free(nodemap);

  MPIR_Datatype_init_names();
  MPIDI_Index_datatypes();

  MPIR_Comm_commit(MPIR_Process.comm_world);
  MPIR_Comm_commit(MPIR_Process.comm_self);

  /* -------------------------------- */
  /* Initialize Dynamic Tasking       */
  /* -------------------------------- */
  if(spawned) {
    char  parent_port[MPIDI_MAX_KVS_VALUE_LEN];
    PMI_RC(PMI_KVS_Get(MPIDI_Global.kvsname,
                       MPIDI_PARENT_PORT_KVSKEY,
                       parent_port,
                       MPIDI_MAX_KVS_VALUE_LEN),pmi);
    MPI_RC_POP(MPIDI_Comm_connect(parent_port,
                                  NULL,
                                  0,
                                  MPIR_Process.comm_world,
                                  &comm));
    MPIR_Process.comm_parent = comm;
    MPIR_Assert(MPIR_Process.comm_parent != NULL);
    MPL_strncpy(comm->name, "MPI_COMM_PARENT", MPI_MAX_OBJECT_NAME);
  }

  /* -------------------------------- */
  /* Return MPICH Parameters          */
  /* -------------------------------- */
  switch(requested) {
  case MPI_THREAD_SINGLE:
  case MPI_THREAD_SERIALIZED:
  case MPI_THREAD_FUNNELED:
    *provided = requested;
    break;

  case MPI_THREAD_MULTIPLE:
    *provided = MAX_THREAD_MODE;
    break;
  }

  *has_args = TRUE;
  *has_env  = TRUE;

fn_exit:
  /* -------------------------------- */
  /* Free temporary resources         */
  /* -------------------------------- */
  if(provname) {
    MPL_free(provname);
    hints->fabric_attr->prov_name = NULL;
  }
  if(prov)fi_freeinfo(prov);
  fi_freeinfo(hints);
  if(table)MPL_free(table);

  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Init)
__SI__ int MPIDI_Create_endpoint(info_t        *prov_use,
                                 fid_domain_t   domain,
                                 fid_cq_t       p2p_cq,
                                 fid_cntr_t     rma_ctr,
                                 fid_mr_t       mr,
                                 fid_av_t       av,
                                 fid_base_ep_t *ep,
                                 int            index)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
  FI_RC(fi_scalable_ep(domain, prov_use, ep, NULL),ep);
  FI_RC(fi_scalable_ep_bind(*ep,(fid_t)av,0),bind);

  tx_attr_t tx_attr;
  tx_attr           = *prov_use->tx_attr;
  tx_attr.caps      = FI_TAGGED;
  tx_attr.caps     |= FI_DELIVERY_COMPLETE;
  tx_attr.op_flags  = FI_DELIVERY_COMPLETE;
  FI_RC(fi_tx_context(*ep,index,&tx_attr,&G_TXC_TAG(index),NULL),ep);
  FI_RC(fi_ep_bind(G_TXC_TAG(index),(fid_t)p2p_cq,FI_SEND),bind);

  tx_attr           = *prov_use->tx_attr;
  tx_attr.caps      = FI_RMA;
  tx_attr.caps     |= FI_ATOMICS;
  tx_attr.caps     |= FI_DELIVERY_COMPLETE;
  tx_attr.op_flags  = FI_DELIVERY_COMPLETE;
  FI_RC(fi_tx_context(*ep,index+1,&tx_attr,&G_TXC_RMA(index),NULL),ep);
  FI_RC(fi_ep_bind(G_TXC_RMA(index),(fid_t)p2p_cq,FI_SEND),bind);

  tx_attr           = *prov_use->tx_attr;
  tx_attr.caps      = FI_MSG;
  tx_attr.op_flags  = 0;
  FI_RC(fi_tx_context(*ep,index+2,&tx_attr,&G_TXC_MSG(index),NULL),ep);
  FI_RC(fi_ep_bind(G_TXC_MSG(index),(fid_t)p2p_cq,FI_SEND),bind);

  tx_attr           = *prov_use->tx_attr;
  tx_attr.caps      = FI_RMA;
  tx_attr.caps     |= FI_ATOMICS;
  tx_attr.caps     |= FI_DELIVERY_COMPLETE;
  tx_attr.op_flags  = FI_DELIVERY_COMPLETE;
  FI_RC(fi_tx_context(*ep,index+3,&tx_attr,&G_TXC_CTR(index),NULL),ep);
  FI_RC(fi_ep_bind(G_TXC_CTR(index),(fid_t)rma_ctr,FI_WRITE|FI_READ),bind);

  rx_attr_t rx_attr;
  rx_attr           = *prov_use->rx_attr;
  rx_attr.caps      = FI_TAGGED;
  rx_attr.caps     |= FI_DELIVERY_COMPLETE;
  rx_attr.op_flags  = 0;
  FI_RC(fi_rx_context(*ep,index,&rx_attr,&G_RXC_TAG(index),NULL),ep);
  FI_RC(fi_ep_bind(G_RXC_TAG(index),(fid_t)p2p_cq,FI_RECV),bind);

  rx_attr           = *prov_use->rx_attr;
  rx_attr.caps      = FI_RMA;
  rx_attr.caps     |= FI_ATOMICS;
  rx_attr.op_flags  = 0;
  FI_RC(fi_rx_context(*ep,index+1,&rx_attr,&G_RXC_RMA(index),NULL),ep);

  rx_attr           = *prov_use->rx_attr;
  rx_attr.caps      = FI_MSG;
  rx_attr.caps     |= FI_MULTI_RECV;
  rx_attr.op_flags  = FI_MULTI_RECV;
  FI_RC(fi_rx_context(*ep,index+2,&rx_attr,&G_RXC_MSG(index),NULL),ep);
  FI_RC(fi_ep_bind(G_RXC_MSG(index),(fid_t)p2p_cq,FI_RECV),bind);

  rx_attr           = *prov_use->rx_attr;
  rx_attr.caps      = FI_RMA;
  rx_attr.caps     |= FI_ATOMICS;
  rx_attr.op_flags  = 0;
  FI_RC(fi_rx_context(*ep,index+3,&rx_attr,&G_RXC_CTR(index),NULL),ep);

  FI_RC(fi_enable(G_TXC_TAG(index)),ep_enable);
  FI_RC(fi_enable(G_TXC_RMA(index)),ep_enable);
  FI_RC(fi_enable(G_TXC_MSG(index)),ep_enable);
  FI_RC(fi_enable(G_TXC_CTR(index)),ep_enable);

  FI_RC(fi_enable(G_RXC_TAG(index)),ep_enable);
  FI_RC(fi_enable(G_RXC_RMA(index)),ep_enable);
  FI_RC(fi_enable(G_RXC_MSG(index)),ep_enable);
  FI_RC(fi_enable(G_RXC_CTR(index)),ep_enable);

  FI_RC(fi_ep_bind(G_TXC_RMA(0),
                   (fid_t)mr,
                   FI_REMOTE_READ|FI_REMOTE_WRITE),bind);
  FI_RC(fi_ep_bind(G_RXC_RMA(0),
                   (fid_t)mr,
                   FI_REMOTE_READ|FI_REMOTE_WRITE),bind);
  FI_RC(fi_ep_bind(G_TXC_CTR(0),
                   (fid_t)mr,
                   FI_REMOTE_READ|FI_REMOTE_WRITE),bind);
  FI_RC(fi_ep_bind(G_RXC_CTR(0),
                   (fid_t)mr,
                   FI_REMOTE_READ|FI_REMOTE_WRITE),bind);
#else
  /* ---------------------------------------------------------- */
  /* Bind the MR, CQs, counters,  and AV to the endpoint object */
  /* ---------------------------------------------------------- */
  FI_RC(fi_endpoint(domain, prov_use, ep, NULL),ep);
  FI_RC(fi_ep_bind(*ep,(fid_t)p2p_cq,FI_SEND|FI_RECV|FI_SELECTIVE_COMPLETION),bind);
  FI_RC(fi_ep_bind(*ep,(fid_t)rma_ctr,FI_READ|FI_WRITE),bind);
  FI_RC(fi_ep_bind(*ep,(fid_t)av,0),bind);
  FI_RC(fi_enable(*ep),ep_enable);
  FI_RC(fi_ep_bind(*ep,(fid_t)mr,FI_REMOTE_READ|FI_REMOTE_WRITE),bind);
#endif /* MPIDI_USE_SCALABLE_ENDPOINTS */

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_InitCompleted)
__ADI_INLINE__ int MPIDI_InitCompleted(void)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Finalize)
__ADI_INLINE__ int MPIDI_Finalize(void)
{
  int             mpi_errno  = MPI_SUCCESS;
  int             i          = 0;
  int             barrier[2] = {0};
  MPIR_Errflag_t  errflag    = MPIR_ERR_NONE;
  MPIR_Comm      *comm;

  BEGIN_FUNC(FCNAME);

  /* Barrier over allreduce, but force non-immediate send */
  MPIDI_Global.max_buffered_send  = 0;
  MPI_RC_POP(MPIR_Allreduce_impl(&barrier[0],&barrier[1],1,MPI_INT,
                                 MPI_SUM,MPIR_Process.comm_world,
                                 &errflag));

#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
  for(i=0; i<MPIDI_Global.num_ctrlblock; i++)
    FI_RC(fi_cancel((fid_t)G_RXC_MSG(0),
                    &MPIDI_Global.control_req[i].context),ctrlcancel);

  FI_RC(fi_close((fid_t)G_TXC_TAG(0)),epclose);
  FI_RC(fi_close((fid_t)G_TXC_RMA(0)),epclose);
  FI_RC(fi_close((fid_t)G_TXC_MSG(0)),epclose);
  FI_RC(fi_close((fid_t)G_TXC_CTR(0)),epclose);

  FI_RC(fi_close((fid_t)G_RXC_TAG(0)),epclose);
  FI_RC(fi_close((fid_t)G_RXC_RMA(0)),epclose);
  FI_RC(fi_close((fid_t)G_RXC_MSG(0)),epclose);
  FI_RC(fi_close((fid_t)G_RXC_CTR(0)),epclose);
#else /* MPIDI_USE_SCALABLE_ENDPOINTS not defined */
  for(i=0; i<MPIDI_Global.num_ctrlblock; i++)
    FI_RC(fi_cancel((fid_t)MPIDI_Global.ep,
                    &MPIDI_Global.control_req[i].context),ctrlcancel);
#endif
  FI_RC(fi_close((fid_t)MPIDI_Global.mr),mr_unreg);
  FI_RC(fi_close((fid_t)MPIDI_Global.ep),epclose);
  FI_RC(fi_close((fid_t)MPIDI_Global.av),avclose);
  FI_RC(fi_close((fid_t)MPIDI_Global.p2p_cq),cqclose);
  FI_RC(fi_close((fid_t)MPIDI_Global.rma_ctr),cqclose);
  FI_RC(fi_close((fid_t)MPIDI_Global.domain),domainclose);

  /* --------------------------------------- */
  /* Free comm world VCRT and addr table     */
  /* comm_release will also releace the vcrt */
  /* --------------------------------------- */
  comm = MPIR_Process.comm_world;
  MPIR_Comm_release_always(comm);

  comm = MPIR_Process.comm_self;
  MPIR_Comm_release_always(comm);

  MPL_free(MPIDI_Addr_table);

  MPL_free(MPIDI_Global.node_map);

  for(i = 0; i < MPIDI_Global.num_ctrlblock; i++)
    MPL_free(MPIDI_Global.iov[i].iov_base);

  MPL_free(MPIDI_Global.iov);
  MPL_free(MPIDI_Global.msg);
  MPL_free(MPIDI_Global.control_req);

  MPIDI_Map_destroy(MPIDI_Global.win_map);
  MPIDI_Map_destroy(MPIDI_Global.comm_map);

  PMI_Finalize();

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Get_universe_size)
__ADI_INLINE__ int MPIDI_Get_universe_size(int *universe_size)
{
  int mpi_errno = MPI_SUCCESS;
  int pmi_errno = PMI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  pmi_errno = PMI_Get_universe_size(universe_size);

  if(pmi_errno != PMI_SUCCESS)
    MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                         "**pmi_get_universe_size",
                         "**pmi_get_universe_size %d",
                         pmi_errno);

  if(*universe_size < 0)
    *universe_size = MPIR_UNIVERSE_SIZE_NOT_AVAILABLE;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Get_processor_name)
__ADI_INLINE__ int MPIDI_Get_processor_name(char *name,
                                            int   namelen,
                                            int  *resultlen)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  if(!MPIDI_Global.pname_set) {
#if defined(HAVE_GETHOSTNAME)

    if(gethostname(MPIDI_Global.pname, MPI_MAX_PROCESSOR_NAME) == 0)
      MPIDI_Global.pname_len = (int)strlen(MPIDI_Global.pname);

#elif defined(HAVE_SYSINFO)

    if(sysinfo(SI_HOSTNAME, MPIDI_Global.pname, MPI_MAX_PROCESSOR_NAME) == 0)
      MPIDI_Global.pname_len = (int)strlen(MPIDI_Global.pname);

#else
    MPIU_Snprintf(MPIDI_Global.pname, MPI_MAX_PROCESSOR_NAME, "%d",
                  MPIR_Process.comm_world->rank);
    MPIDI_Global.pname_len = (int)strlen(MPIDI_Global.pname);
#endif
    MPIDI_Global.pname_set = 1;
  }

  MPIR_ERR_CHKANDJUMP(MPIDI_Global.pname_len <= 0,
                      mpi_errno,
                      MPI_ERR_OTHER,
                      "**procnamefailed");
  MPL_strncpy(name, MPIDI_Global.pname, namelen);

  if(resultlen)
    *resultlen = MPIDI_Global.pname_len;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Abort)
__ADI_INLINE__ int MPIDI_Abort(MPIR_Comm  *comm,
                               int         mpi_errno,
                               int         exit_code,
                               const char *error_msg)
{
  char sys_str[MPI_MAX_ERROR_STRING+5]  = "";
  char comm_str[MPI_MAX_ERROR_STRING]   = "";
  char world_str[MPI_MAX_ERROR_STRING]  = "";
  char error_str[2*MPI_MAX_ERROR_STRING + 128];
  BEGIN_FUNC(FCNAME);

  if(MPIR_Process.comm_world) {
    int rank = MPIR_Process.comm_world->rank;
    snprintf(world_str, sizeof(world_str), " on node %d", rank);
  }

  if(comm) {
    int rank       = comm->rank;
    int context_id = comm->context_id;
    snprintf(comm_str, sizeof(comm_str), " (rank %d in comm %d)", rank, context_id);
  }

  if(!error_msg)
    error_msg = "Internal error";

  if(mpi_errno != MPI_SUCCESS) {
    char msg[MPI_MAX_ERROR_STRING] = "";
    MPIR_Err_get_string(mpi_errno, msg, MPI_MAX_ERROR_STRING, NULL);
    snprintf(sys_str, sizeof(msg), " (%s)", msg);
  }

  snprintf(error_str, sizeof(error_str), "Abort(%d)%s%s: %s%s\n",
           exit_code, world_str, comm_str, error_msg, sys_str);
  MPL_error_printf("%s", error_str);


  END_FUNC(FCNAME);
  fflush(stderr);
  fflush(stdout);
  exit(1);
  return 0;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Alloc_mem)
__ADI_INLINE__ void *MPIDI_Alloc_mem(size_t size, MPIR_Info *info_ptr)
{
  BEGIN_FUNC(FCNAME);

  void *ap;
  ap = MPL_malloc(size);

  END_FUNC(FCNAME);
  return ap;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Free_mem)
__ADI_INLINE__ int MPIDI_Free_mem(void *ptr)
{
  int mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  MPL_free(ptr);

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_VCR_set_ep)
__ADI_INLINE__ int MPIDI_VCR_Set_ep(MPIDI_VCR *vcr, int ep_idx)
{
  BEGIN_FUNC(FCNAME);


  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Get_node_id)
__ADI_INLINE__ int MPIDI_Get_node_id(MPIR_Comm      *comm,
                                     int             rank,
                                     MPID_Node_id_t *id_p)
{
  *id_p   = MPIDI_Global.node_map[COMM_TO_INDEX(comm,rank)];
  return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Get_max_node_id)
__ADI_INLINE__ int MPIDI_Get_max_node_id(MPIR_Comm      *comm,
                                         MPID_Node_id_t *max_id_p)
{
  *max_id_p = MPIDI_Global.max_node_id;
  return MPI_SUCCESS;
}

/* LPID Routines */
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_get_lpid)
__ADI_INLINE__ int MPIDI_Comm_get_lpid(MPIR_Comm *comm_ptr,
                                       int        idx,
                                       int       *lpid_ptr,
                                       MPL_bool  is_remote)
{
  BEGIN_FUNC(FCNAME);
  if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM)
    *lpid_ptr = COMM_TO_INDEX(comm_ptr,idx);
  else if (is_remote)
    *lpid_ptr = COMM_TO_INDEX(comm_ptr,idx);
  else
    *lpid_ptr = COMM_OFI(comm_ptr)->local_vcrt->vcr_table[idx].addr_idx;

  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Create_intercomm_from_lpids)
__ADI_INLINE__ int MPIDI_Create_intercomm_from_lpids(MPIR_Comm *newcomm_ptr,
                                                     int        size,
                                                     const int  lpids[])
{
  int i;
  BEGIN_FUNC(FCNAME);
  MPIDI_VCRT_Create(size, &COMM_OFI(newcomm_ptr)->vcrt);

  for(i=0; i<size; i++)
    COMM_OFI(newcomm_ptr)->vcrt->vcr_table[i].addr_idx = lpids[i];

  END_FUNC(FCNAME);
  return 0;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Aint_add)
__ADI_INLINE__ MPI_Aint MPIDI_Aint_add(MPI_Aint base, MPI_Aint disp)
{
  MPI_Aint result;
  BEGIN_FUNC(FCNAME);
  result =  MPIR_VOID_PTR_CAST_TO_MPI_AINT((char *)MPIR_AINT_CAST_TO_VOID_PTR(base) + disp);
  END_FUNC(FCNAME);
  return result;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Aint_diff)
__ADI_INLINE__ MPI_Aint MPIDI_Aint_diff(MPI_Aint addr1, MPI_Aint addr2)
{
  MPI_Aint result;
  BEGIN_FUNC(FCNAME);
  result =  MPIR_PTR_DISP_CAST_TO_MPI_AINT((char *)MPIR_AINT_CAST_TO_VOID_PTR(addr1) - (char *)MPIR_AINT_CAST_TO_VOID_PTR(addr2));
  END_FUNC(FCNAME);
  return result;
}

__SI__ int MPIDI_Choose_provider(info_t  *prov, info_t **prov_use)
{
  info_t *p = prov;
  int     i = 0;
  *prov_use = prov;
  if (MPIR_CVAR_OFI_DUMP_PROVIDERS) {
    fprintf(stdout, "Dumping Providers(first=%p):\n", prov);
    while(p) {
      fprintf(stdout, "%s", fi_tostr(p, FI_TYPE_INFO));
      p=p->next;
    }
  }
  return i;
}
#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Unused_gen_catalog)
__SI__ void MPIDI_Unused_gen_catalog()
{
#if 0
  char *a;
  int b,e;
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_pmi","**ofid_pmi %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_addrinfo","**ofid_addrinfo %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_opendomain","**ofid_opendomain %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_bind","**ofid_bind %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_fabric","**ofid_fabric %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_opencq","**ofid_opencq %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_openct","**ofid_openct %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_bind","**ofid_bind %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_ep_enable","**ofid_ep_enable %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_ep","**ofid_ep %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_avopen","**ofid_avopen %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_getname","**ofid_getname %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_avmap","**ofid_avmap %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_avlookup","**ofid_avlookup %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_avsync","**ofid_avsync %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_epclose","**ofid_epclose %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_cqclose","**ofid_cqclose %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_epsync","**ofid_epsync %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_alias","**ofid_alias %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_getopt","**ofid_getopt %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_setopt","**ofid_setopt %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_domainclose","**ofid_domainclose %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_avclose","**ofid_avclose %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_tsend","**ofid_tsend %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_tinject","**ofid_tinject %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_tsendsync","**ofid_tsendsync %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_trecv","**ofid_trecv %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_trecvsync","**ofid_trecvsync %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_poll","**ofid_poll %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_peek","**ofid_peek %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_send","**ofid_send %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_inject","**ofid_inject %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_rdma_write","**ofid_rdma_write %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_rdma_inject_write","**ofid_rdma_inject_write %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_rdma_atomicto","**ofid_rdma_atomicto %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_rdma_cswap","**ofid_rdma_cswap %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_rdma_readfrom","**ofid_rdma_readfrom %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_rdma_readfrom","**ofid_rdma_readfrom %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_mr_reg","**ofid_mr_reg %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_mr_unreg","**ofid_mr_unreg %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_prepost","**ofid_prepost %s %d %s %s",a,b,a,a);
  MPIR_ERR_SET2(e,MPI_ERR_OTHER,"**ofid_ctrlcancel","**ofid_ctrlcancel %s %d %s %s",a,b,a,a);
#endif
}
