/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

#ifndef MPIR_REQUEST_POST_H_INCLUDED
#define MPIR_REQUEST_POST_H_INCLUDED

static inline MPIR_Request *MPIR_Request_create(MPIR_Request_kind_t kind)
{
    MPIR_Request *req;

    req = (MPIR_Request*)MPIR_Handle_obj_alloc(&MPIR_Request_mem);
    if (req != NULL) {
	MPL_DBG_MSG_P(MPIR_DBG_REQUEST,VERBOSE,
                      "allocated request, handle=0x%08x", req->handle);
#ifdef MPICH_DBG_OUTPUT
	/*MPIR_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPIR_REQUEST);*/
	if (HANDLE_GET_MPI_KIND(req->handle) != MPIR_REQUEST)
	{
	    int mpi_errno;
	    mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL,
                                             FCNAME, __LINE__, MPI_ERR_OTHER,
                                             "**invalid_handle", "**invalid_handle %d", req->handle);
	    MPID_Abort(MPIR_Process.comm_world, mpi_errno, -1, NULL);
	}
#endif
	/* FIXME: This makes request creation expensive.  We need to
         * trim this to the basics, with additional setup for
         * special-purpose requests (think base class and
         * inheritance).  For example, do we really* want to set the
         * kind to UNDEFINED? And should the RMA values be set only
         * for RMA requests? */
	MPIR_Object_set_ref(req, 1);
	req->kind = kind;
        MPIR_cc_set(&req->cc, 1);
	req->cc_ptr		   = &req->cc;

        req->completion_notification = NULL;

	req->status.MPI_ERROR	   = MPI_SUCCESS;
        MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);

	req->comm		   = NULL;

        switch(kind) {
        case MPIR_REQUEST_KIND__SEND:
            MPII_REQUEST_CLEAR_DBG(req);
            break;
        case MPIR_REQUEST_KIND__COLL:
            req->u.nbc.errflag = MPIR_ERR_NONE;
            break;
        default:
            break;
        }

        MPID_Request_create_hook(req);
    }
    else
    {
	/* FIXME: This fails to fail if debugging is turned off */
	MPL_DBG_MSG(MPIR_DBG_REQUEST,TYPICAL,"unable to allocate a request");
    }

    return req;
}

static inline void MPIR_Request_free(MPIR_Request *req)
{
    int inuse;

    MPIR_Request_release_ref(req, &inuse);

    /* inform the device that we are decrementing the ref-count on
     * this request */
    MPID_Request_free_hook(req);

    if (inuse == 0) {
        MPL_DBG_MSG_P(MPIR_DBG_REQUEST,VERBOSE,
                       "freeing request, handle=0x%08x", req->handle);

#ifdef MPICH_DBG_OUTPUT
        if (HANDLE_GET_MPI_KIND(req->handle) != MPIR_REQUEST)
        {
            int mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL,
                                                 FCNAME, __LINE__, MPI_ERR_OTHER,
                                                 "**invalid_handle", "**invalid_handle %d", req->handle);
            MPID_Abort(MPIR_Process.comm_world, mpi_errno, -1, NULL);
        }

        if (req->ref_count != 0)
        {
            int mpi_errno = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL,
                                                 FCNAME, __LINE__, MPI_ERR_OTHER,
                                                 "**invalid_refcount", "**invalid_refcount %d", req->ref_count);
            MPID_Abort(MPIR_Process.comm_world, mpi_errno, -1, NULL);
        }
#endif

        /* FIXME: We need a better way to handle these so that we do
         * not always need to initialize these fields and check them
         * when we destroy a request */
        /* FIXME: We need a way to call these routines ONLY when the
         * related ref count has become zero. */
        if (req->comm != NULL) {
            MPIR_Comm_release(req->comm);
        }

        if (req->kind == MPIR_REQUEST_KIND__GREQUEST && req->u.ureq.greq_fns != NULL) {
            MPL_free(req->u.ureq.greq_fns);
        }

        MPID_Request_destroy_hook(req);

        MPIR_Handle_obj_free(&MPIR_Request_mem, req);
    }
}

#endif /* MPIR_REQUEST_POST_H_INCLUDED */

