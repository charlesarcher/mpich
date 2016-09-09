/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpiimpl.h"
#include "mpicomm.h"
#include "mpl_utlist.h"

/* -- Begin Profiling Symbol Block for routine MPI_Comm_create_endpoints */
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Comm_create_endpoints = PMPI_Comm_create_endpoints
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF PMPI_Comm_create_endpoints  MPI_Comm_create_endpoints
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPI_Comm_create_endpoints as PMPI_Comm_create_endpoints
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPI_Comm_create_endpoints(MPI_Comm comm, int my_num_ep,
                              MPI_Info info,MPI_Comm newcomm[]) __attribute__((weak,alias("PMPI_Comm_create_endpoints")));
#endif
/* -- End Profiling Symbol Block */

/* prototypes to make the compiler happy in the case that PMPI_LOCAL expands to
 * nothing instead of "static" */
PMPI_LOCAL int MPIR_Comm_create_endpoints_impl(MPIR_Comm *parent_comm_ptr, int my_num_ep,
                                               MPIR_Comm **newcomm_ptr);


#undef FUNCNAME
#define FUNCNAME MPIR_Comm_map_endpoint
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static int MPIR_Comm_map_endpoint(MPIR_Comm * newcomm, MPIR_Comm * src_comm,
                                  int *src_mapping, int *ep_mapping, int src_mapping_size,
                                  MPIR_Comm_map_t ** map)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Comm_map_t *mapper;
    MPIR_CHKPMEM_DECL(1);
    MPIR_FUNC_TERSE_STATE_DECL(MPIR_STATE_MPIR_COMM_MAP_ENDPOINT);
    MPIR_FUNC_TERSE_ENTER(MPIR_STATE_MPIR_COMM_MAP_ENDPOINT);

    MPIR_CHKPMEM_MALLOC(mapper, MPIR_Comm_map_t *, sizeof(MPIR_Comm_map_t), mpi_errno, "mapper");

    mapper->type             = MPIR_COMM_MAP_TYPE__ENDPOINT;
    mapper->src_comm         = src_comm;
    mapper->dir              = MPIR_COMM_MAP_DIR__L2L;
    mapper->src_mapping_size = src_mapping_size;

    mapper->src_mapping  = src_mapping;
    mapper->ep_mapping  = ep_mapping;
    mapper->free_mapping = 0;
    mapper->next         = NULL;

    MPL_LL_APPEND(newcomm->mapper_head, newcomm->mapper_tail, mapper);

    if (map)
        *map = mapper;

  fn_exit:
    MPIR_CHKPMEM_COMMIT();
    MPIR_FUNC_TERSE_EXIT(MPIR_STATE_MPIR_COMM_MAP_IRREGULAR);
    return mpi_errno;
  fn_fail:
    MPIR_CHKPMEM_REAP();
    goto fn_exit;
}


/* Define MPICH_MPI_FROM_PMPI if weak symbols are not supported to build
   the MPI routines */
#ifndef MPICH_MPI_FROM_PMPI
#undef MPI_Comm_create_endpoints
#define MPI_Comm_create_endpoints PMPI_Comm_create_endpoints

#undef FUNCNAME
#define FUNCNAME MPIR_Comm_create_endpoints_impl
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
PMPI_LOCAL int MPIR_Comm_create_endpoints_impl(MPIR_Comm *parent_comm_ptr, int my_num_ep, MPIR_Comm **newcomm_ptr)
{
    int total=0,prefix=0,idx,i,mpi_errno = MPI_SUCCESS, *allgather_buf,*src_mapping,*ep_mapping;
    MPIR_Comm *split_comm;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIR_Comm_map_t *map;

    MPIR_CHKLMEM_DECL(3);
    /* Create two communicators, one with process in the EP comm and one with
     * processes not in the EP comm, then free the latter
     * split gets a new context id to be used by the ep comms */
    mpi_errno = MPIR_Comm_split_impl(parent_comm_ptr, my_num_ep == 1, 0, &split_comm);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    if (my_num_ep == 0)
        MPIR_Comm_free_impl(split_comm);

    for(i=0; i<my_num_ep; i++) {
          if(i>0)MPIR_Comm_add_ref(split_comm);
          MPIR_Comm_create(&newcomm_ptr[i]);
          newcomm_ptr[i]->recvcontext_id = split_comm->recvcontext_id;
          newcomm_ptr[i]->context_id     = split_comm->context_id;
          newcomm_ptr[i]->comm_kind      = split_comm->comm_kind;
          newcomm_ptr[i]->endpoint_parent= split_comm;
      }
    MPIR_CHKLMEM_MALLOC(allgather_buf,int*,sizeof(int)*parent_comm_ptr->local_size,
                        mpi_errno,"allgather");
    allgather_buf[parent_comm_ptr->rank] = my_num_ep;
    mpi_errno = MPIR_Allgather_impl(MPI_IN_PLACE,1, MPI_INT, allgather_buf,
                                    1, MPI_INT,parent_comm_ptr,&errflag);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    if (my_num_ep == 0) goto fn_exit;
    for(i=0;i<parent_comm_ptr->local_size;i++){
        if(i==parent_comm_ptr->rank)
            prefix=total;
        total+=allgather_buf[i];
    }
    MPIR_CHKLMEM_MALLOC(src_mapping,int*,sizeof(int)*total,mpi_errno,"src_mapping");
    MPIR_CHKLMEM_MALLOC(ep_mapping,int*,sizeof(int)*total,mpi_errno,"src_mapping");

    MPIR_Comm_map_endpoint(newcomm_ptr[0], parent_comm_ptr, src_mapping,
                           ep_mapping, total, &map);
    for(i=0,idx=0; i<parent_comm_ptr->local_size;i++){
        int j;
        for(j=0;j<allgather_buf[i];j++){
            src_mapping[idx] = i;
            ep_mapping[idx]  = j;
            idx++;
        }
    }
    newcomm_ptr[0]->local_size  = total;
    newcomm_ptr[0]->remote_size = total;
    newcomm_ptr[0]->rank        = prefix;
    MPIR_Comm_commit(newcomm_ptr[0]);
    for(i=1; i<my_num_ep; i++) {
        newcomm_ptr[i]->local_size  = total;
        newcomm_ptr[i]->remote_size = total;
        newcomm_ptr[i]->rank        = prefix+i;
        MPIR_Comm_map_dup(newcomm_ptr[i], newcomm_ptr[0], MPIR_COMM_MAP_DIR__L2L);
        MPIR_Comm_commit(newcomm_ptr[i]);
    }

 fn_exit:
    MPIR_CHKLMEM_FREEALL();
    return mpi_errno;
 fn_fail:
    mpi_errno = MPIR_Err_create_code( MPI_SUCCESS, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__,
                                      MPI_ERR_ENDPOINTS, "**endpoints_arg", "**endpoints_arg %d",
                                      my_num_ep );
    goto fn_exit;
}
#endif /* MPICH_MPI_FROM_PMPI */

#undef FUNCNAME
#define FUNCNAME MPI_Comm_create_endpoints
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
/*@

MPI_Comm_create_endpoints - Creates a new communicator with additional endpoints

Input Parameters:
. comm - Parent communicator (handle)
. my_num_ep - Number of endpoints to create at the calling process
. info - Info hints

Output Parameters:
. newcomm - A new communicator with additional endpoints (handle)

Notes:
  This function creates a new communicator from an existing communicator, comm,
  where my_num_ep ranks in the output communicator are associated with a single
  calling rank in comm. This function is collective on comm. Distinct handles
  for each associated rank in the output communicator are returned in the
  newcomm array at the corresponding rank in comm. Ranks associated with a
  process in comm are numbered contiguously in the output communicator, and the
  starting rank is defined by the order of the associated rank in the parent
  communicator.

  If comm is an intracommunicator, this function returns a new
  intracommunicator new_comm with a communication group of size equal to the
  sum of the values of my_num_ep on all calling processes. No cached
  information propagates from comm to new_comm. Each process in comm must call
  'MPI_COMM_CREATE_ENDPOINTS' with a my_num_ep argument that ranges from 0 to
  the value of the 'MPI_COMM_MAX_ENDPOINTS' attribute on comm. Each process may
  specify a different value for the my_num_ep argument. When my_num_ep is 0, no
  output communicator is returned.

  If comm is an intercommunicator, then the output communicator is also an
  intercommunicator where the local group consists of endpoint ranks associated
  with ranks in the local group of comm and the remote group consists of
  endpoint ranks associated with ranks in the remote group of comm. If either
  the local or remote group is empty, 'MPI_COMM_NULL' is returned in all
  entries of newcomm.

  Ranks in new_comm behave as MPI processes. For example, a collective function
  on new_comm must be called concurrently on every rank in this communicator.
  An exception to this rule is made for 'MPI_COMM_FREE', which must be called
  for every rank in new_comm, but must permit a single thread to perform these
  calls serially.

  Rationale: The concurrency exception for 'MPI_COMM_FREE' is made to enable
  'MPI_COMM_CREATE_ENDPOINTS' to be used when the MPI library has not been
  initialized with 'MPI_THREAD_MULTIPLE', or when the threading package cannot
  satisfy the concurrency requirement for collective operations.

  Advice to Users: Although threads can acquire individual ranks through the
  'MPI_COMM_CREATE_ENDPOINTS' function, they still share an instance of the MPI
  library. Users must ensure that the threading level with which MPI was
  initialized is maintained. Some operations, such as collective operations,
  cannot be used by multiple threads sharing an instance of the MPI library,
  when MPI was initialized with 'MPI_THREAD_MULTIPLE'.

.N ThreadSafe

.N Fortran

.N Errors
.N MPI_SUCCESS
.N MPI_ERR_COMM

.seealso: MPI_Comm_free, MPI_Keyval_create, MPI_Attr_put, MPI_Attr_delete,
 MPI_Comm_create_keyval, MPI_Comm_set_attr, MPI_Comm_delete_attr
@*/
int MPI_Comm_create_endpoints(MPI_Comm comm, int my_num_ep, MPI_Info info,
                              MPI_Comm newcomm[])
{
    int i, mpi_errno = MPI_SUCCESS;
    MPIR_Comm *comm_ptr = NULL, **newcomm_ptr = NULL;
    MPIR_FUNC_TERSE_STATE_DECL(MPID_STATE_MPI_COMM_CREATE_ENDPOINTS);
    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPI_COMM_CREATE_ENDPOINTS);
    
    MPIR_ERRTEST_INITIALIZED_ORDIE();

    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);


    /* Validate parameters, especially handles needing to be converted */
#   ifdef HAVE_ERROR_CHECKING
    {
        MPID_BEGIN_ERROR_CHECKS;
        {
            MPIR_ERRTEST_COMM(comm, mpi_errno);
        }
        MPID_END_ERROR_CHECKS;
    }
#   endif /* HAVE_ERROR_CHECKING */

    /* Convert MPI object handles to object pointers */
    MPIR_Comm_get_ptr( comm, comm_ptr );

    /* Validate parameters and objects (post conversion) */
#   ifdef HAVE_ERROR_CHECKING
    {
        MPID_BEGIN_ERROR_CHECKS;
        {
            /* Validate comm_ptr: If comm_ptr is invalid, it is set to null */
            MPIR_Comm_valid_ptr( comm_ptr, mpi_errno, FALSE );
            if (mpi_errno) goto fn_fail;

            if (my_num_ep > 0) {
                MPIR_ERRTEST_ARGNULL(newcomm, "newcomm", mpi_errno);
            }

            MPIR_ERRTEST_ARGNEG(my_num_ep, "my_num_ep", mpi_errno);
        }
        MPID_END_ERROR_CHECKS;
    }
#   endif /* HAVE_ERROR_CHECKING */

    /* ... body of routine ...  */

    if (my_num_ep > 0)
        newcomm_ptr = MPL_malloc(sizeof(MPIR_Comm*) * my_num_ep);

    mpi_errno = MPIR_Comm_create_endpoints_impl(comm_ptr, my_num_ep, newcomm_ptr);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    for (i = 0; i < my_num_ep; i++)
        MPIR_OBJ_PUBLISH_HANDLE(newcomm[i], newcomm_ptr[i]->handle);
    /* ... end of body of routine ... */

fn_exit:
    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPI_COMM_CREATE_ENDPOINTS);
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);
    return mpi_errno;

fn_fail:
    /* --BEGIN ERROR HANDLING-- */
#   ifdef HAVE_ERROR_CHECKING
    {
        mpi_errno = MPIR_Err_create_code(
            mpi_errno, MPIR_ERR_RECOVERABLE, FCNAME, __LINE__, MPI_ERR_OTHER, "**mpi_comm_create_endpoints",
            "**mpi_comm_create_endpoints %C %d %I %p", comm, my_num_ep, info, newcomm);
    }
#   endif
    if (NULL != newcomm_ptr)
        MPL_free(newcomm_ptr);
    for (i = 0; i < my_num_ep; i++)
        newcomm[i] = MPI_COMM_NULL;
    mpi_errno = MPIR_Err_return_comm( comm_ptr, FCNAME, mpi_errno );
    goto fn_exit;
    /* --END ERROR HANDLING-- */
}

