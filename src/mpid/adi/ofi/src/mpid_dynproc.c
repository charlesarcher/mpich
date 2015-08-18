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
#include <pmi.h>
#include <mpidimpl.h>
#include "mpid_types.h"

extern int  MPIR_Info_get_impl(MPIR_Info *info_ptr, const char *key, int valuelen, char *value, int *flag);
extern void MPIR_Info_get_nkeys_impl(MPIR_Info *info_ptr, int *nkeys);
extern void MPIR_Info_get_valuelen_impl(MPIR_Info *info_ptr, const char *key, int *valuelen, int *flag);
extern void MPIR_Info_get_nkeys_impl(MPIR_Info *info_ptr, int *nkeys);
extern int  MPIR_Info_get_nthkey_impl(MPIR_Info *info, int n, char *key);
#define MPIDI_PORT_NAME_TAG_KEY "tag"
#define MPIDI_CONNENTRY_TAG_KEY "connentry"

#ifdef MPIDI_USE_AV_TABLE
#define TABLE_INDEX_INCR() MPIDI_Addr_table->size++
#else
#define TABLE_INDEX_INCR()
#endif

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_mpi_to_pmi_keyvals)
__SI__ int MPIDI_mpi_to_pmi_keyvals(MPIR_Info     *info_ptr,
                                    PMI_keyval_t **kv_ptr,
                                    int           *nkeys_ptr)
{
  char key[MPI_MAX_INFO_KEY];
  PMI_keyval_t *kv = 0;
  int           i, nkeys = 0, vallen, flag, mpi_errno=MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  if(!info_ptr || info_ptr->handle == MPI_INFO_NULL)
    goto fn_exit;

  MPIR_Info_get_nkeys_impl(info_ptr, &nkeys);

  if(nkeys == 0) goto fn_exit;

  kv = (PMI_keyval_t *)MPL_malloc(nkeys * sizeof(PMI_keyval_t));

  for(i=0; i<nkeys; i++) {
    MPI_RC_POP(MPIR_Info_get_nthkey_impl(info_ptr,i,key));
    MPIR_Info_get_valuelen_impl(info_ptr,key,&vallen,&flag);
    kv[i].key = (const char *)MPL_strdup(key);
    kv[i].val = (char *)MPL_malloc(vallen + 1);
    MPIR_Info_get_impl(info_ptr, key, vallen+1, kv[i].val, &flag);
  }

fn_fail:
fn_exit:
  *kv_ptr    = kv;
  *nkeys_ptr = nkeys;
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_free_pmi_keyvals)
__SI__ void MPIDI_free_pmi_keyvals(PMI_keyval_t **kv,
                                   int            size,
                                   int           *counts)
{
  int i,j;

  BEGIN_FUNC(FCNAME);

  for(i=0; i<size; i++) {
    for(j=0; j<counts[i]; j++) {
      if(kv[i][j].key != NULL)
        MPL_free((char *)kv[i][j].key);

      if(kv[i][j].val != NULL)
        MPL_free(kv[i][j].val);
    }

    if(kv[i] != NULL)
      MPL_free(kv[i]);
  }

  END_FUNC(FCNAME);
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_free_port_name_tag)
__SI__ void MPIDI_free_port_name_tag(int tag)
{
  int index, rem_tag;
  BEGIN_FUNC(FCNAME);

  index = tag / (sizeof(int) * 8);
  rem_tag = tag - (index * sizeof(int) * 8);

  MPIDI_Global.port_name_tag_mask[index] &= ~(1 << ((8 * sizeof(int)) - 1 - rem_tag));
  END_FUNC(FCNAME);
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_get_port_name_tag)
__SI__ int MPIDI_get_port_name_tag(int *port_name_tag)
{
  int i, j;
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  for(i = 0; i < MPIR_MAX_CONTEXT_MASK; i++)
    if(MPIDI_Global.port_name_tag_mask[i] != ~0)
      break;

  if(i < MPIR_MAX_CONTEXT_MASK)
    for(j = 0; j < (8 * sizeof(int)); j++) {
      if((MPIDI_Global.port_name_tag_mask[i] | (1 << ((8 * sizeof(int)) - j - 1))) !=
         MPIDI_Global.port_name_tag_mask[i]) {
        MPIDI_Global.port_name_tag_mask[i] |= (1 << ((8 * sizeof(int)) - j - 1));
        *port_name_tag = ((i * 8 * sizeof(int)) + j);
        goto fn_exit;
      }
    }
  else
    goto fn_fail;

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;

fn_fail:
  *port_name_tag = -1;
  mpi_errno = MPI_ERR_OTHER;
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_GetTagFromPort)
__SI__ int MPIDI_GetTagFromPort(const char *port_name,
                                int        *port_name_tag)
{
  BEGIN_FUNC(FCNAME);
  int mpi_errno = MPI_SUCCESS;
  int str_errno = MPL_STR_SUCCESS;

  if(strlen(port_name) == 0)
    goto fn_exit;

  str_errno = MPL_str_get_int_arg(port_name,
                                  MPIDI_PORT_NAME_TAG_KEY,
                                  port_name_tag);
  MPIR_ERR_CHKANDJUMP(str_errno, mpi_errno, MPI_ERR_OTHER,
                      "**argstr_no_port_name_tag");
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_GetConnNameFromPort)
__SI__ int MPIDI_GetConnNameFromPort(const char *port_name,
                                     char       *connname)
{
  int mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);
  int maxlen = MPIDI_KVSAPPSTRLEN;
  MPL_str_get_binary_arg(port_name,
                         MPIDI_CONNENTRY_TAG_KEY,
                         connname,
                         MPIDI_Global.addrnamelen,
                         &maxlen);
  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(accept_probe_callback)
__SI__ int accept_probe_callback(cq_tagged_entry_t *wc,
                                 MPIR_Request      *rreq)
{
  BEGIN_FUNC(FCNAME);
  MPIDI_Dynproc_req *ctrl = (MPIDI_Dynproc_req *)rreq;
  ctrl->source = get_source(wc->tag);
  ctrl->tag    = get_tag(wc->tag);
  ctrl->msglen = wc->len;
  ctrl->done   = 1;
  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(accept_recv_callback)
__SI__ int dynproc_done_callback(cq_tagged_entry_t *wc,
                                 MPIR_Request      *rreq)
{
  BEGIN_FUNC(FCNAME);
  MPIDI_Dynproc_req *ctrl = (MPIDI_Dynproc_req *)rreq;
  ctrl->done++;
  END_FUNC(FCNAME);
  return MPI_SUCCESS;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_dynproc_create_ic)
__SI__ int MPIDI_dynproc_create_ic(const char      *port_name,
                                   char            *addr_table,
                                   MPID_Node_id_t  *node_table,
                                   int              entries,
                                   MPIR_Comm       *comm_ptr,
                                   MPIR_Comm      **newcomm,
                                   int              is_low_group,
                                   char            *api)
{
  int        start,i,context_id_offset,mpi_errno = MPI_SUCCESS;
  MPIR_Comm *tmp_comm_ptr = NULL;
  fi_addr_t *addr = NULL;
  BEGIN_FUNC(FCNAME);

  MPI_RC_POP(MPIDI_GetTagFromPort(port_name,&context_id_offset));
  MPI_RC_POP(MPIR_Comm_create(&tmp_comm_ptr));

  tmp_comm_ptr->context_id     = MPIR_CONTEXT_SET_FIELD(DYNAMIC_PROC,
                                                        context_id_offset,
                                                        1);
  tmp_comm_ptr->recvcontext_id       = tmp_comm_ptr->context_id;
  tmp_comm_ptr->remote_size          = entries;
  tmp_comm_ptr->local_size           = comm_ptr->local_size;
  tmp_comm_ptr->rank                 = comm_ptr->rank;
  tmp_comm_ptr->comm_kind            = MPIR_COMM_KIND__INTERCOMM;
  tmp_comm_ptr->local_comm           = comm_ptr;
  tmp_comm_ptr->is_low_group         = is_low_group;
  COMM_OFI(tmp_comm_ptr)->local_vcrt = COMM_OFI(comm_ptr)->vcrt;

  mpi_errno = MPIDI_VCRT_Create(tmp_comm_ptr->remote_size,
                                &COMM_OFI(tmp_comm_ptr)->vcrt);
  start = MPIDI_Addr_table->size;
  MPIDI_Global.node_map = (MPID_Node_id_t *)MPL_realloc(MPIDI_Global.node_map,
                                                        (entries+start)*sizeof(MPID_Node_id_t));

  for(i=0; i<entries; i++)
    MPIDI_Global.node_map[start+i] = node_table[i];

#ifndef MPIDI_USE_AV_TABLE
  MPIDI_Addr_table = (MPIDI_Addr_table_t *)MPL_realloc(MPIDI_Addr_table,
                                                       (entries+start)*sizeof(fi_addr_t)+
                                                       sizeof(MPIDI_Addr_table_t));
  addr=&TO_PHYS(start);
#endif

  MPIDI_Addr_table->size += entries;
  for(i=0; i<entries; i++) {
    COMM_OFI(tmp_comm_ptr)->vcrt->vcr_table[i].addr_idx += start;
  }
  FI_RC(fi_av_insert(MPIDI_Global.av,addr_table,entries,
                     addr,0ULL,NULL),avmap);
  *newcomm = tmp_comm_ptr;

  MPI_RC_POP(MPIR_Comm_dup_impl(tmp_comm_ptr, newcomm));

  MPIDI_VCRT_Release(COMM_OFI(tmp_comm_ptr)->vcrt);
  MPIR_Handle_obj_free(&MPIR_Comm_mem, tmp_comm_ptr);

  MPL_free(addr_table);
  MPL_free(node_table);

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_dynproc_bcast)
__SI__ int MPIDI_dynproc_bcast(int              root,
                               MPIR_Comm       *comm_ptr,
                               int             *out_root,
                               ssize_t         *out_table_size,
                               char           **out_addr_table,
                               MPID_Node_id_t **out_node_table)
{
  int            entries, mpi_errno =  MPI_SUCCESS;
  MPIR_Errflag_t errflag = MPIR_ERR_NONE;
  BEGIN_FUNC(FCNAME);

  MPI_RC_POP(MPIR_Bcast_intra(out_root,1,MPI_INT,
                              root,comm_ptr,&errflag));
  MPI_RC_POP(MPIR_Bcast_intra(out_table_size, 1, MPI_LONG_LONG_INT,
                              root, comm_ptr, &errflag));

  if(*out_addr_table == NULL)
    *out_addr_table  = (char *)MPL_malloc(*out_table_size);

  MPI_RC_POP(MPIR_Bcast_intra(*out_addr_table, *out_table_size,
                              MPI_CHAR,root, comm_ptr, &errflag));

  entries     = *out_table_size/MPIDI_Global.addrnamelen;

  if(*out_node_table == NULL)
    *out_node_table  = (MPID_Node_id_t *)MPL_malloc(MPIDI_Global.addrnamelen*entries);

  MPI_RC_POP(MPIR_Bcast_intra(*out_node_table, entries*sizeof(MPID_Node_id_t),
                              MPI_CHAR,root, comm_ptr, &errflag));

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef  FCNAME
#define FCNAME DECL_FUNC(MPIDI_dynproc_exch_map)
__SI__ int MPIDI_dynproc_exch_map(int              root,
                                  int              phase,
                                  int              port_id,
                                  fi_addr_t       *conn,
                                  char            *conname,
                                  MPIR_Comm       *comm_ptr,
                                  ssize_t         *out_table_size,
                                  int             *out_root,
                                  char           **out_addr_table,
                                  MPID_Node_id_t **out_node_table)
{
  int i,mpi_errno = MPI_SUCCESS;

  BEGIN_FUNC(FCNAME);

  MPIDI_Dynproc_req req[2];
  uint64_t          match_bits    = 0;
  uint64_t          mask_bits     = 0;
  ssize_t           ret           = 0;
  msg_tagged_t      msg;
  req[0].done            = 0;
  req[0].callback        = accept_probe_callback;
  req[1].done            = 0;
  req[1].callback        = accept_probe_callback;
  match_bits             = init_recvtag(&mask_bits,port_id,
                                        MPI_ANY_SOURCE,
                                        MPI_ANY_TAG);
  match_bits            |= MPID_DYNPROC_SEND;

  if(phase == 0) {
    /* Receive the addresses                           */
    /* We don't know the size, so probe for table size */
    /* Receive phase updates the connection            */
    /* With the probed address                         */
    msg.msg_iov   = NULL;
    msg.desc      = NULL;
    msg.iov_count = 0;
    msg.addr      = FI_ADDR_UNSPEC;
    msg.tag       = match_bits;
    msg.ignore    = mask_bits;
    msg.context   = (void *) &req[0].context;
    msg.data      = 0;
    while(req[0].done != 1) {
      MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
      ret = fi_trecvmsg(G_RXC_TAG(0),&msg,FI_PEEK|FI_COMPLETION);
      MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_FI_MUTEX);
      if(ret == 0)
        PROGRESS_WHILE(req[0].done == 0);
      else if(ret == -FI_ENOMSG)
        continue;
      else
        MPIR_ERR_SETFATALANDJUMP4(mpi_errno,MPI_ERR_OTHER,"**ofid_peek",
                                  "**ofid_peek %s %d %s %s",__SHORT_FILE__,
                                  __LINE__,FCNAME,fi_strerror(errno));
    }

    *out_table_size    = req[0].msglen;
    *out_root          = req[0].tag;
    *out_addr_table    = (char *)MPL_malloc(*out_table_size);

    int entries        = req[0].msglen/MPIDI_Global.addrnamelen;
    *out_node_table    = (MPID_Node_id_t *)MPL_malloc(entries*sizeof(MPID_Node_id_t));

    req[0].done     = 0;
    req[0].callback = dynproc_done_callback;
    req[1].done     = 0;
    req[1].callback = dynproc_done_callback;

    FI_RC_RETRY(fi_trecv(G_RXC_TAG(0),
                         *out_addr_table,
                         *out_table_size,
                         MPIDI_Global.mr,
                         FI_ADDR_UNSPEC,
                         match_bits,
                         mask_bits,
                         &req[0].context),trecv);
    FI_RC_RETRY(fi_trecv(G_RXC_TAG(0),
                         *out_node_table,
                         entries*sizeof(MPID_Node_id_t),
                         MPIDI_Global.mr,
                         FI_ADDR_UNSPEC,
                         match_bits,
                         mask_bits,
                         &req[1].context),trecv);

    PROGRESS_WHILE(!req[0].done || !req[1].done);
    memcpy(conname, *out_addr_table+req[0].source*MPIDI_Global.addrnamelen, MPIDI_Global.addrnamelen);
  }

  if(phase == 1) {
    /* Send our table to the child */
    /* Send phase maps the entry   */
    char   *my_addr_table;
    int     tag   = root;
    int     tblsz = MPIDI_Global.addrnamelen*comm_ptr->local_size;
    my_addr_table       = (char *)MPL_malloc(tblsz);

    MPID_Node_id_t *my_node_table;
    MPID_Node_id_t  nodetblsz = sizeof(*my_node_table)*comm_ptr->local_size;
    my_node_table             = (MPID_Node_id_t *)MPL_malloc(nodetblsz);

    match_bits                = init_sendtag(port_id,
                                             comm_ptr->rank,
                                             tag,MPID_DYNPROC_SEND);

    for(i=0; i<comm_ptr->local_size; i++) {
      size_t sz = MPIDI_Global.addrnamelen;
      FI_RC(fi_av_lookup(MPIDI_Global.av,
                         COMM_TO_PHYS(comm_ptr, i),
                         my_addr_table+i*MPIDI_Global.addrnamelen,
                         &sz),avlookup);
      MPIR_Assert(sz == MPIDI_Global.addrnamelen);
    }

    for(i=0; i<comm_ptr->local_size; i++)
      my_node_table[i] = MPIDI_Global.node_map[COMM_TO_INDEX(comm_ptr,i)];

    /* fi_av_map here is not quite right for some providers */
    /* we need to get this connection from the sockname     */
    req[0].done     = 0;
    req[0].callback = dynproc_done_callback;
    req[1].done     = 0;
    req[1].callback = dynproc_done_callback;
    FI_RC_RETRY(fi_tsend(G_TXC_TAG(0),
                         my_addr_table,
                         tblsz,
                         MPIDI_Global.mr,
                         *conn,
                         match_bits,
                         (void *) &req[0].context),tsend);
    FI_RC_RETRY(fi_tsend(G_TXC_TAG(0),
                         my_node_table,
                         nodetblsz,
                         MPIDI_Global.mr,
                         *conn,
                         match_bits,
                         (void *) &req[1].context),tsend);

    PROGRESS_WHILE(!req[0].done || !req[1].done);

    MPL_free(my_addr_table);
    MPL_free(my_node_table);
  }

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_spawn_multiple)
__ADI_INLINE__ int MPIDI_Comm_spawn_multiple(int         count,
                                             char       *commands[],
                                             char      **argvs[],
                                             const int   maxprocs[],
                                             MPIR_Info  *info_ptrs[],
                                             int         root,
                                             MPIR_Comm  *comm_ptr,
                                             MPIR_Comm **intercomm,
                                             int         errcodes[])
{
  char           port_name[MPI_MAX_PORT_NAME];
  int           *info_keyval_sizes=0, i, mpi_errno=MPI_SUCCESS;
  PMI_keyval_t **info_keyval_vectors=0, preput_keyval_vector;
  int           *pmi_errcodes = 0, pmi_errno=0;
  int            total_num_processes, should_accept = 1;
  BEGIN_FUNC(FCNAME);

  memset(port_name, 0, sizeof(port_name));

  if(comm_ptr->rank == root) {
    total_num_processes = 0;

    for(i=0; i<count; i++)
      total_num_processes += maxprocs[i];

    pmi_errcodes = (int *)MPL_malloc(sizeof(int) * total_num_processes);
    MPIR_ERR_CHKANDJUMP(!pmi_errcodes, mpi_errno, MPI_ERR_OTHER, "**nomem");

    for(i=0; i<total_num_processes; i++)
      pmi_errcodes[i] = 0;

    MPI_RC_POP(MPIDI_Open_port(NULL, port_name));

    info_keyval_sizes   = (int *)          MPL_malloc(count*sizeof(int));
    MPIR_ERR_CHKANDJUMP(!info_keyval_sizes, mpi_errno, MPI_ERR_OTHER, "**nomem");
    info_keyval_vectors = (PMI_keyval_t **) MPL_malloc(count*sizeof(PMI_keyval_t *));
    MPIR_ERR_CHKANDJUMP(!info_keyval_vectors, mpi_errno, MPI_ERR_OTHER, "**nomem");

    if(!info_ptrs)
      for(i=0; i<count; i++) {
        info_keyval_vectors[i] = 0;
        info_keyval_sizes[i]   = 0;
      }
    else
      for(i=0; i<count; i++)
        MPI_RC_POP(MPIDI_mpi_to_pmi_keyvals(info_ptrs[i],
                                            &info_keyval_vectors[i],
                                            &info_keyval_sizes[i]));

    preput_keyval_vector.key = MPIDI_PARENT_PORT_KVSKEY;
    preput_keyval_vector.val = port_name;
    pmi_errno                = PMI_Spawn_multiple(count, (const char **)
                                                  commands,
                                                  (const char ** *) argvs,
                                                  maxprocs,
                                                  info_keyval_sizes,
                                                  (const PMI_keyval_t **)
                                                  info_keyval_vectors,
                                                  1,
                                                  &preput_keyval_vector,
                                                  pmi_errcodes);

    if(pmi_errno != PMI_SUCCESS)
      MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                           "**pmi_spawn_multiple",
                           "**pmi_spawn_multiple %d",
                           pmi_errno);

    if(errcodes != MPI_ERRCODES_IGNORE) {
      for(i=0; i<total_num_processes; i++) {
        errcodes[i] = pmi_errcodes[0];
        should_accept = should_accept && errcodes[i];
      }

      should_accept = !should_accept;
    }
  }

  if(errcodes != MPI_ERRCODES_IGNORE) {
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPI_RC_POP(MPIR_Bcast_impl(&should_accept,1,MPI_INT,
                               root,comm_ptr,&errflag));
    MPI_RC_POP(MPIR_Bcast_impl(&pmi_errno,1,MPI_INT,
                               root,comm_ptr,&errflag));
    MPI_RC_POP(MPIR_Bcast_impl(&total_num_processes,1,MPI_INT,
                               root, comm_ptr, &errflag));
    MPI_RC_POP(MPIR_Bcast_impl(errcodes, total_num_processes, MPI_INT,
                               root, comm_ptr,&errflag));
  }

  if(should_accept)
    MPI_RC_POP(MPIDI_Comm_accept(port_name, NULL, root,
                                 comm_ptr, intercomm));
  else {
    if((pmi_errno == PMI_SUCCESS) && (errcodes[0] != 0))
      MPIR_Comm_create(intercomm);
  }

  if(comm_ptr->rank == root)
    MPI_RC_POP(MPIDI_Close_port(port_name));

fn_exit:

  if(info_keyval_vectors) {
    MPIDI_free_pmi_keyvals(info_keyval_vectors, count, info_keyval_sizes);
    MPL_free(info_keyval_vectors);
  }

  if(info_keyval_sizes)
    MPL_free(info_keyval_sizes);

  if(pmi_errcodes)
    MPL_free(pmi_errcodes);

  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_disconnect)
__ADI_INLINE__ int MPIDI_Comm_disconnect(MPIR_Comm *comm_ptr)
{
  int            mpi_errno = MPI_SUCCESS;
  MPIR_Errflag_t errflag   = MPIR_ERR_NONE;

  BEGIN_FUNC(FCNAME);

  MPI_RC_POP(MPIR_Barrier_impl(comm_ptr, &errflag));
  MPI_RC_POP(MPIR_Comm_free_impl(comm_ptr));

fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Open_port)
__ADI_INLINE__ int MPIDI_Open_port(MPIR_Info *info_ptr,
                                   char      *port_name)
{
  BEGIN_FUNC(FCNAME);

  int mpi_errno     = MPI_SUCCESS;
  int str_errno     = MPL_STR_SUCCESS;
  int port_name_tag = 0;
  int len           = MPI_MAX_PORT_NAME;

  MPI_RC_POP(MPIDI_get_port_name_tag(&port_name_tag));
  MPI_STR_RC(MPL_str_add_int_arg(&port_name,&len,MPIDI_PORT_NAME_TAG_KEY,
                                 port_name_tag),port_str);
  MPI_STR_RC(MPL_str_add_binary_arg(&port_name,&len,MPIDI_CONNENTRY_TAG_KEY,
                                    MPIDI_Global.addrname,
                                    MPIDI_Global.addrnamelen),port_str);
fn_exit:
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Close_port)
__ADI_INLINE__ int MPIDI_Close_port(const char *port_name)
{
  int mpi_errno = MPI_SUCCESS;
  int port_name_tag;

  BEGIN_FUNC(FCNAME);

  mpi_errno = MPIDI_GetTagFromPort(port_name, &port_name_tag);
  MPIDI_free_port_name_tag(port_name_tag);

  END_FUNC(FCNAME);
  return mpi_errno;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_accept)
__ADI_INLINE__ int MPIDI_Comm_accept(const char  *port_name,
                                     MPIR_Info   *info,
                                     int          root,
                                     MPIR_Comm   *comm_ptr,
                                     MPIR_Comm  **newcomm)
{
  int             entries,mpi_errno = MPI_SUCCESS;
  char           *child_addr_table    = NULL;
  MPID_Node_id_t *child_node_table    = NULL;
  ssize_t         child_table_sz      = -1LL;
  int             child_root          = -1;
  BEGIN_FUNC(FCNAME);
  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_SPAWN_MUTEX);
  int rank = comm_ptr->rank;

  if(rank == root) {
    fi_addr_t conn;
    char      conname[FI_NAME_MAX];
    int       port_id;
    MPI_RC_POP(MPIDI_GetTagFromPort(port_name,&port_id));
    MPI_RC_POP(MPIDI_dynproc_exch_map(root,0,port_id,&conn,conname,comm_ptr,
                                      &child_table_sz,&child_root,
                                      &child_addr_table,
                                      &child_node_table));
    FI_RC(fi_av_insert(MPIDI_Global.av,conname,1,&conn,0ULL,NULL),avmap);
    TABLE_INDEX_INCR();
    MPI_RC_POP(MPIDI_dynproc_exch_map(root,1,port_id,&conn,conname,comm_ptr,
                                      &child_table_sz,&child_root,
                                      &child_addr_table,
                                      &child_node_table));
    FI_RC(fi_av_remove(MPIDI_Global.av, &conn,1,0ULL),avmap);
  }

  /* Map the new address table */
  MPI_RC_POP(MPIDI_dynproc_bcast(root,comm_ptr,&child_root,
                                 &child_table_sz,
                                 &child_addr_table,
                                 &child_node_table));
  /* Now Create the New Intercomm */
  entries   = child_table_sz/MPIDI_Global.addrnamelen;
  MPI_RC_POP(MPIDI_dynproc_create_ic(port_name,
                                     child_addr_table,
                                     child_node_table,
                                     entries,
                                     comm_ptr,
                                     newcomm,
                                     1,
                                     (char *)"Accept"));
fn_exit:
  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_SPAWN_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;

fn_fail:
  goto fn_exit;
}


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_Comm_connect)
__ADI_INLINE__ int MPIDI_Comm_connect(const char *port_name,
                                      MPIR_Info  *info,
                                      int         root,
                                      MPIR_Comm  *comm_ptr,
                                      MPIR_Comm **newcomm)
{
  int                entries,mpi_errno = MPI_SUCCESS;
  BEGIN_FUNC(FCNAME);

  char           *parent_addr_table = NULL;
  MPID_Node_id_t *parent_node_table = NULL;
  ssize_t         parent_table_sz   = -1LL;
  int             parent_root       = -1;
  int             rank              = comm_ptr->rank;
  int             port_id;

  MPID_THREAD_CS_ENTER(POBJ,MPIR_THREAD_POBJ_SPAWN_MUTEX);
  MPI_RC_POP(MPIDI_GetTagFromPort(port_name,&port_id));

  if(rank == root) {
    fi_addr_t conn;
    char      conname[FI_NAME_MAX];
    MPI_RC_POP(MPIDI_GetConnNameFromPort(port_name,conname));
    FI_RC(fi_av_insert(MPIDI_Global.av,conname,1,&conn,0ULL,NULL),avmap);
    TABLE_INDEX_INCR();
    MPI_RC_POP(MPIDI_dynproc_exch_map(root,1,port_id,&conn,conname,comm_ptr,
                                      &parent_table_sz,&parent_root,
                                      &parent_addr_table,
                                      &parent_node_table));
    MPI_RC_POP(MPIDI_dynproc_exch_map(root,0,port_id,&conn,conname,comm_ptr,
                                      &parent_table_sz,&parent_root,
                                      &parent_addr_table,
                                      &parent_node_table));
    FI_RC(fi_av_remove(MPIDI_Global.av, &conn,1,0ULL),avmap);
  }

  /* Map the new address table */
  MPI_RC_POP(MPIDI_dynproc_bcast(root,comm_ptr,&parent_root,
                                 &parent_table_sz,
                                 &parent_addr_table,
                                 &parent_node_table));

  /* Now Create the New Intercomm */
  entries   = parent_table_sz/MPIDI_Global.addrnamelen;
  MPI_RC_POP(MPIDI_dynproc_create_ic(port_name,
                                     parent_addr_table,
                                     parent_node_table,
                                     entries,
                                     comm_ptr,
                                     newcomm,
                                     0,
                                     (char *)"Connect"));
fn_exit:
  MPID_THREAD_CS_EXIT(POBJ,MPIR_THREAD_POBJ_SPAWN_MUTEX);
  END_FUNC(FCNAME);
  return mpi_errno;
fn_fail:
  goto fn_exit;
}
